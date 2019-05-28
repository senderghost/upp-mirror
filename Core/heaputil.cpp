#include "Core.h"

#ifdef UPP_HEAP

#ifdef PLATFORM_POSIX
#include <sys/mman.h>
#endif

namespace Upp {

#include "HeapImp.h"

void OutOfMemoryPanic(size_t size)
{
	char h[200];
	sprintf(h, "Out of memory!\nRequested size: %lld B\nU++ allocated memory: %d KB",
	        (long long)size, MemoryUsedKb());
	Panic(h);
}

size_t Heap::huge_4KB_count;
size_t Heap::free_4KB;
size_t Heap::big_size;
size_t Heap::big_count;
size_t Heap::sys_size;
size_t Heap::sys_count;
size_t Heap::huge_chunks;

int MemoryUsedKb() { return int(4 * (Heap::huge_4KB_count - Heap::free_4KB)); }

int sKBLimit = INT_MAX;

void  MemoryLimitKb(int kb)
{
	sKBLimit = kb;
}

void DoPeakProfile();

void *SysAllocRaw(size_t size, size_t reqsize)
{
	size_t rsz = int(((size + 4095) & ~4095) >> 10);
	void *ptr = NULL;
	if(MemoryUsedKb() < sKBLimit) {
	#ifdef PLATFORM_WIN32
		ptr = VirtualAlloc(NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	#elif PLATFORM_LINUX
		ptr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		if(ptr == MAP_FAILED)
			ptr = NULL;
	#else
		ptr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
		if(ptr == MAP_FAILED)
			ptr = NULL;
	#endif
	}
	if(!ptr)
		OutOfMemoryPanic(size/*reqsize*/);
	DoPeakProfile();
	return ptr;
}

void  SysFreeRaw(void *ptr, size_t size)
{
#ifdef PLATFORM_WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	munmap(ptr, size);
#endif
}

void *MemoryAllocPermanent(size_t size)
{
	Mutex::Lock __(Heap::mutex);
	if(size > 10000)
		return SysAllocRaw(size, size);
	static byte *ptr = NULL;
	static byte *limit = NULL;
	ASSERT(size < INT_MAX);
	if(ptr + size >= limit) {
		ptr = (byte *)SysAllocRaw(16384, 16384);
		limit = ptr + 16384;
	}
	void *p = ptr;
	ptr += size;
	return p;
}

void HeapPanic(const char *text, void *pos, int size)
{
	RLOG("\n\n" << text << "\n");
	HexDump(VppLog(), pos, size, 1024);
	Panic(text);
}

#ifdef HEAPDBG

void *Heap::DbgFreeCheckK(void *p, int k)
{
	Page *page = GetPage(p);
	ASSERT((byte *)page + sizeof(Page) <= (byte *)p && (byte *)p < (byte *)page + 4096);
	ASSERT((4096 - ((uintptr_t)p & (uintptr_t)4095)) % Ksz(k) == 0);
	ASSERT(page->klass == k);
	DbgFreeCheck((FreeLink *)p + 1, Ksz(k) - sizeof(FreeLink));
	return p;
}

void Heap::DbgFreeFillK(void *p, int k)
{
	DbgFreeFill((FreeLink *)p + 1, Ksz(k) - sizeof(FreeLink));
}

#endif


void Heap::Make(MemoryProfile& f)
{
	Mutex::Lock __(mutex);
	memset((void *)&f, 0, sizeof(MemoryProfile));
	for(int i = 0; i < NKLASS; i++) {
		int qq = Ksz(i) / 4;
		Page *p = work[i]->next;
		while(p != work[i]) {
			f.allocated[qq] += p->active;
			f.fragments[qq] += p->Count() - p->active;
			p = p->next;
		}
		p = full[i]->next;
		while(p != full[i]) {
			f.allocated[qq] += p->active;
			p = p->next;
		}
		if(empty[i])
			f.freepages++;
		p = aux.empty[i];
		while(p) {
			f.freepages++;
			p = p->next;
		}
	}
	int ii = 0;
	int fi = 0;
	DLink *m = large->next;
	while(m != large) {
		LargeHeap::BlkHeader *h = m->GetFirst();
		for(;;) {
			if(h->IsFree()) {
				f.large_fragments_count++;
				int sz = LUNIT * h->GetSize();
				f.large_fragments_total += sz;
				if(h->size < 2048)
					f.large_fragments[sz >> 8]++;
			}
			else {
				f.large_count++;
				f.large_total += LUNIT * h->size;
			}
			if(h->IsLast())
				break;
			h = h->GetNextHeader();
		}
		m = m->next;
	}

	f.sys_count = (int)sys_count;
	f.sys_total = sys_size;
	
	f.huge_count = int(big_count - sys_count);
	f.huge_total = 4096 * max((int)big_size - (int)sys_size, 0); // this is not 100% correct, but approximate
	
	f.chunks32MB = (int)huge_chunks;
}

#ifdef flagHEAPSTAT
int stat[65536];
int bigstat;

void Heap::Stat(size_t sz)
{
	if(sz < 65536)
		stat[sz]++;
	else
		bigstat++;
}

EXITBLOCK {
	int sum = 0;
	for(int i = 0; i < 65536; i++)
		sum += stat[i];
	sum += bigstat;
	int total = 0;
	VppLog() << Sprintf("Allocation statistics: (total allocations: %d)\n", sum);
	for(int i = 0; i < 65536; i++)
		if(stat[i]) {
			total += stat[i];
			VppLog() << Sprintf("%5d %8dx %2d%%, total %8dx %2d%%\n",
			                    i, stat[i], 100 * stat[i] / sum, total, 100 * total / sum);
		}
	if(bigstat) {
		total += bigstat;
		VppLog() << Sprintf(">64KB %8dx %2d%%, total %8dx %2d%%\n",
		                    bigstat, 100 * bigstat / sum, total, 100 * total / sum);
	}
}
#endif

}

#endif
