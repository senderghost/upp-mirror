#include <Core/Core.h>
#pragma BLITZ_APPROVE

#ifdef PLATFORM_POSIX
#include <sys/mman.h>
#endif

NAMESPACE_UPP

#ifdef UPP_HEAP

#include "HeapImp.h"

//#define _DEBUG

#define LTIMING(x) // RTIMING(x)

MemoryProfile *sPeak;
int sKB;

int MemoryUsedKb()
{
	return sKB;
}

MemoryProfile *PeakMemoryProfile()
{
	if(sPeak)
		return sPeak;
	sPeak = (MemoryProfile *)MemoryAllocPermanent(sizeof(MemoryProfile));
	memset(sPeak, 0, sizeof(MemoryProfile));
	return NULL;
}


void *SysAllocRaw(size_t size)
{
	LTIMING("SysAllocRaw");
	if(sPeak) *sPeak = MemoryProfile();
	sKB += int(((size + 4095) & ~4095) >> 10);
#ifdef PLATFORM_WIN32
	void *ptr = VirtualAlloc(NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#else
#ifdef PLATFORM_LINUX
	void *ptr =  mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#else
	void *ptr =  mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
#endif
#endif
	if(!ptr)
		Panic("Out of memory!");
	return ptr;
}

void  SysFreeRaw(void *ptr, size_t size)
{
	LTIMING("SysFreeRaw");
//	LOGF("SysFreeRaw %X - %d\n", ptr, size);
	sKB -= int(((size + 4095) & ~4095) >> 10);
#ifdef PLATFORM_WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	munmap(ptr, size);
#endif
}

//----------------------------------------------------------------

static StaticMutex sHeapLock;

void *MemoryAllocPermanentRaw(size_t size)
{
	if(size >= 256)
		return malloc(size);
	static byte *ptr = NULL;
	static byte *limit = NULL;
	if(ptr + size >= limit) {
		ptr = (byte *)SysAllocRaw(4096);
		limit = ptr + 4096;
	}
	void *p = ptr;
	ptr += size;
	return p;
}


void *MemoryAllocPermanent(size_t size)
{
#ifdef _MULTITHREADED
	Mutex::Lock __(sHeapLock);
#endif
	return MemoryAllocPermanentRaw(size);
}

void HeapPanic(const char *text, void *pos, int size)
{
	RLOG("\n\n" << text << "\n");
	char h[200];
	sprintf(h, "MemoryWatchFree(%p);\n", pos);
	RLOG(h);
	HexDump(VppLog(), pos, size, 64);
	Panic(text);
}

#ifdef _DEBUG

void FreeFill(dword *ptr, int count)
{
	while(count--)
		*ptr++ = 0x65657246;
}

void FreeCheck(dword *ptr, int count)
{
	while(count--)
		if(*ptr++ != 0x65657246)
			HeapPanic("Writes to freed blocks detected", ptr, count);
}

#endif

inline int Ksz(int k)   { return (k + 1) << 4; }

#define MPAGE(a, x)     { 0, 0, 0, 0, 0, NULL, a[x], a[x] },

inline
void MPage::Format(int k)
{
	klass = k;
	int sz = Ksz(k);
	count = 4064 / sz;
	freecount = 0;
	free = 32 + count * sz - sz;
	freelist = NULL;
#ifdef _DEBUG
	FreeFill((dword *)((byte *)this + 32), (4096 - 32) / 4);
#endif
}

#ifdef flagMT
#ifdef COMPILER_MSC
__declspec(thread)
#else
__thread
#endif
#endif
MCache mcache[16];

static MPage sFull[NKLASS][1] = {
#define SFULL(x)   MPAGE(sFull, x)
	SFULL(0)  SFULL(1)  SFULL(2)  SFULL(3)  SFULL(4)  SFULL(5)  SFULL(6)  SFULL(7)
	SFULL(8)  SFULL(9)  SFULL(10) SFULL(11) SFULL(12) SFULL(13) SFULL(14) SFULL(15)
};

static MPage sWork[NKLASS][1] = {
#define SWORK(x)   MPAGE(sWork, x)
	SWORK(0)  SWORK(1)  SWORK(2)  SWORK(3)  SWORK(4)  SWORK(5)  SWORK(6)  SWORK(7)
	SWORK(8)  SWORK(9)  SWORK(10) SWORK(11) SWORK(12) SWORK(13) SWORK(14) SWORK(15)
};

static MPage sFree[1] = { MPAGE(&sFree, 0) };

static inline FreeLink *sMAllocKN(int k)
{
	MPage *p = sWork[k]->next;
	int n = CACHERES;
	int sz = Ksz(k);
	FreeLink *l = NULL;
	for(;;) {
		while(p->free >= 32) {
			if(n == 0) return l;
			FreeLink *b = (FreeLink *)((byte *)p + p->free);
			p->free -= sz;
			b->next = l;
			l = b;
			n--;
		}
		for(;;) {
			if(n == 0) return l;
			FreeLink *b = p->freelist;
			if(!b) break;
			p->freelist = b->next;
			--p->freecount;
			b->next = l;
			l = b;
			n--;
		}
		if(p->next == p) {
			p = sFree->next;
			if(p->next == p)
				p = (MPage *)SysAllocRaw(4096);
			else
				p->Unlink();
			p->Format(k);
			p->Link(sWork[k]);
		}
		else {
			p->Unlink();
			p->Link(sFull[k]);
			p = sWork[k]->next;
		}
	}
}

static inline void sMFreeK(void *ptr, MPage *p, int k)
{
	((FreeLink *)ptr)->next = p->freelist;
	p->freelist = (FreeLink *)ptr;
	if(p->freecount == 0) {
		p->freecount = 1;
		p->Unlink();
		p->Link(sWork[k]);
	}
	else
		p->freecount++;
	if(p->freecount == p->count) {
		p->Unlink();
		p->Link(sFree);
	}
}

void *MAlloc_Get(MCache& m, int k)
{
	FreeLink *l;
	sHeapLock.Enter();
	l = sMAllocKN(k);
	sHeapLock.Leave();
	m.list = l->next;
	m.count = CACHERES - 1;
	return l;
}

#ifdef _DEBUG

void *CheckFree(void *p, int k)
{
	ASSERT(((MPage *)((uintptr_t)p & ~(uintptr_t)4095))->klass == k);
#ifdef CPU_64
	FreeCheck((dword *)p + 2, (Ksz(k) >> 2) - 2);
#else
	FreeCheck((dword *)p + 1, (Ksz(k) >> 2) - 1);
#endif
	return p;
}

#else
inline void *CheckFree(void *p, int) { return p; }
#endif

void *MemoryAllocSz(size_t& size)
{
	size_t sz = size;
	if(sz == 0) sz = 1;
	if(sz <= 256) {
		LTIMING("Small alloc");
		int k = ((int)sz - 1) >> 4;
		MCache& m = mcache[k];
		FreeLink *l = m.list;
		if(l == NULL) {
			ASSERT(m.count == 0);
			sHeapLock.Enter();
			l = sMAllocKN(k);
			sHeapLock.Leave();
			m.count = CACHERES;
		}
		m.count--;
		m.list = l->next;
		size = Ksz(k);
		return CheckFree(l, k);
	}
	return LAlloc(size);
}

void *MemoryAlloc(size_t sz)
{
	if(sz == 0) sz = 1;
	if(sz <= 256) {
		LTIMING("Small alloc");
		int k = ((int)sz - 1) >> 4;
		MCache& m = mcache[k];
		FreeLink *l = m.list;
		if(l == NULL) {
			ASSERT(m.count == 0);
			sHeapLock.Enter();
			l = sMAllocKN(k);
			sHeapLock.Leave();
			m.count = CACHERES;
		}
		m.count--;
		m.list = l->next;
		return CheckFree(l, k);
	}
	return LAlloc(sz);
}

void MFree_Reduce(MCache& m, int k)
{
	FreeLink *l = m.list;
	sHeapLock.Enter();
	for(int i = 0; i < CACHERES; i++) {
		FreeLink *f = l;
		l = l->next;
		sMFreeK(f, (MPage *)((uintptr_t)f & ~(uintptr_t)4095), k);
	}
	sHeapLock.Leave();
	m.list = l;
	m.count -= CACHERES;
}

void MemoryFree(void *ptr)
{
	if(!ptr) return;
	if(((dword)(uintptr_t)ptr) & 8)
		LFree(ptr);
	else {
		LTIMING("Small free");
		MPage *p = (MPage *)((uintptr_t)ptr & ~(uintptr_t)4095);
		int k = p->klass;
		ASSERT((((uintptr_t)ptr & (uintptr_t)4095) - 32) % Ksz(k) == 0);
#ifdef _DEBUG
#ifdef CPU_64
		FreeFill((dword *)ptr + 2, (Ksz(k) >> 2) - 2);
#else
		FreeFill((dword *)ptr + 1, (Ksz(k) >> 2) - 1);
#endif
#endif
		if(k < 9) {
			MCache& m = mcache[k];
			((FreeLink *)ptr)->next = m.list;
			m.list = (FreeLink *)ptr;
			ASSERT(m.count >= 0);
			if(++m.count > CACHEMAX)
				MFree_Reduce(m, k);
		}
		else {
			sHeapLock.Enter();
			sMFreeK(ptr, p, k);
			sHeapLock.Leave();
		}
	}
}

void MemoryFreeThread()
{
	for(int k = 0; k < 9; k++) {
		MCache& m = mcache[k];
		FreeLink *l = m.list;
		sHeapLock.Enter();
		while(l) {
			FreeLink *f = l;
			l = l->next;
			sMFreeK(f, (MPage *)((uintptr_t)f & ~(uintptr_t)4095), k);
		}
		m.list = NULL;
		m.count = 0;
		sHeapLock.Leave();
	}
}

void MemoryShrink()
{
	LTIMING("MemoryShrink");
	MemoryFreeThread();
	sHeapLock.Enter();
	for(;;) {
		MPage *p = sFree->next;
		if(p->next == p) break;
		p->Unlink();
		SysFreeRaw(p, 4096);
	}
	sHeapLock.Leave();
	LMemoryShrink();
}

void MemoryCheck()
{
	sHeapLock.Enter();
	for(int i = 0; i < NKLASS; i++) {
		MPage *p = sWork[i]->next;
		while(p != sWork[i]) {
			FreeLink *l = p->freelist;
			while(l) {
				CheckFree(l, p->klass);
				l = l->next;
			}
			ASSERT(p->free <= 4096);
			ASSERT(((MPage *)((uintptr_t)p & ~(uintptr_t)4095))->klass == i);
			ASSERT(p->count == (4096 - 32) / Ksz(i));
			p = p->next;
		}
		p = sFull[i]->next;
		while(p != sFull[i]) {
			ASSERT(p->free <= 4096);
			ASSERT(((MPage *)((uintptr_t)p & ~(uintptr_t)4095))->klass == i);
			ASSERT(p->count == (4096 - 32) / Ksz(i));
			p = p->next;
		}
	}
	sHeapLock.Leave();
}

void MemoryProfile::Make()
{
	memset(this, 0, sizeof(MemoryProfile));
	for(int i = 0; i < NKLASS; i++) {
		MPage *p = sFull[i]->next;
		int qq = Ksz(i) / 4;
		while(p != sFull[i]) {
			allocated[qq] += p->count;
			p = p->next;
		}
		p = sWork[i]->next;
		while(p != sWork[i]) {
			allocated[qq] += p->count - p->freecount;
			fragmented[qq] += p->freecount;
			p = p->next;
		}
	}
	MPage *p = sFree->next;
	while(p != sFree) {
		freepages++;
		p = p->next;
	}

	LMake(*this);
}

MemoryProfile::MemoryProfile()
{
	sHeapLock.Enter();
	Make();
	sHeapLock.Leave();
}

#endif

END_UPP_NAMESPACE
