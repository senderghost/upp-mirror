#include "www.h"

#define LLOG(x)  LOG(x)

StaticCriticalSection     reflink_lock;
VectorMap<String, String> reflink;

struct ScanTopicIterator : RichText::Iterator {
	String         link;

	virtual bool operator()(int pos, const RichPara& para)
	{
		if(!IsNull(para.format.label)) {
			LLOG("label: " << para.format.label);
			INTERLOCKED_(reflink_lock)
				reflink.Add(para.format.label, link);
		}
		return false;
	}
};


static void sDoFile(const char *path, const char *link)
{
	ScanTopicIterator sti;
	sti.link = link;
	ParseQTF(ReadTopic(LoadFile(path))).Iterate(sti);
}

void GatherRefLinks(const char *upp)
{
#ifdef MTC
	CoWork work;
#endif
	for(FindFile pff(AppendFileName(upp, "*.*")); pff; pff.Next()) {
		if(pff.IsFolder()) {
			String package = pff.GetName();
			String pdir = AppendFileName(upp, package);
			TopicLink tl;
			tl.package = package;
			for(FindFile ff(AppendFileName(pdir, "*.tpp")); ff; ff.Next()) {
				if(ff.IsFolder()) {
					String group = GetFileTitle(ff.GetName());
					tl.group = group;
					String dir = AppendFileName(pdir, ff.GetName());
					for(FindFile ft(AppendFileName(dir, "*.tpp")); ft; ft.Next()) {
						if(ft.IsFile()) {
							String path = AppendFileName(dir, ft.GetName());
							tl.topic = GetFileTitle(ft.GetName());
							String link = TopicLinkString(tl);
#ifdef MTC
							work & callback2(sDoFile, path, link);
#else
							ScanTopicIterator sti;
							sti.link = link;
							LLOG("Indexing topic " << path << " link: " << link);
							ParseQTF(ReadTopic(LoadFile(path))).Iterate(sti);
#endif
						}
					}
				}
			}
		}
	}
}

struct GatherLinkIterator : RichText::Iterator {
	Index<String> link;

	virtual bool operator()(int pos, const RichPara& para)
	{
		for(int i = 0; i < para.GetCount(); i++) {
			String l = para[i].format.link;
			if(!IsNull(l)) {
				LLOG("GatherLink " << l);
				if(l[0] == ':') {
					int q = reflink.Find(l);
					int w = q;
					if(q < 0)
						q = reflink.Find(l + "::class");
					if(q < 0)
						q = reflink.Find(l + "::struct");
					if(q < 0)
						q = reflink.Find(l + "::union");
					if(q >= 0)
						l = reflink[q];
				}
				link.FindAdd(Nvl(reflink.Get(l, Null), l));
			}
		}
		return false;
	}
};

String TopicFileName(const char *dir, const char *topic)
{
	TopicLink tl = ParseTopicLink(topic);
	return AppendFileName(dir, AppendFileName(tl.package, AppendFileName(tl.group + ".tpp", tl.topic + ".tpp")));
}

String TopicFileName(const char *topic)
{
	String p = TopicFileName(uppbox, topic);
	if(FileExists(p))
		return p;
	p = TopicFileName(bazaar, topic);
	if(FileExists(p))
		return p;
	return TopicFileName(uppsrc, topic);
}

String TopicFileNameHtml(const char *topic)
{
	TopicLink tl = ParseTopicLink(topic);
	return tl.group + "$" + tl.package+ "$" + tl.topic + ".html";
}

static void sGatherTopics(VectorMap<String, Topic> *map, const char *topic)
{
	GatherTopics(*map, topic);
}

String GatherTopics(VectorMap<String, Topic>& map, const char *topic, String& title)
{
	static StaticCriticalSection mapl;
	LLOG("Gather topics: " << topic);
	int q;
	INTERLOCKED_(mapl)
		q = map.Find(topic);
	if(q < 0) {
		LLOG("GatherTopics " << topic);
		Topic p = ReadTopic(LoadFile(TopicFileName(topic)));
		title = p.title;
		String t = p;
		if(IsNull(t))
			return "index.html";
		INTERLOCKED_(mapl)
			map.Add(topic) = p;
		GatherLinkIterator ti;
		ParseQTF(t).Iterate(ti);
#ifdef MTC
		CoWork work;
		for(int i = 0; i < ti.link.GetCount(); i++)
			work & callback2(sGatherTopics, &map, ti.link[i]);
#else
		for(int i = 0; i < ti.link.GetCount(); i++)
			sGatherTopics(&map, ti.link[i]);
#endif
	}
	else
		INTERLOCKED_(mapl)
			title = map[q].title;
	return TopicFileNameHtml(topic);
}

String GatherTopics(VectorMap<String, Topic>& map, const char *topic)
{
	String dummy;
	return GatherTopics(map, topic, dummy);
}
