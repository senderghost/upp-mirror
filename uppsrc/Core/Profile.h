class String;

class TimingInspector {
protected:
	static bool active;

	const char *name;
	int         call_count;
	dword       total_time;
	dword       min_time;
	dword       max_time;
	int         nesting_depth;
	int         max_nesting;
	int         all_count;
	dword       start_time;

public:
	TimingInspector(const char *name = NULL); // Not String !!!
	~TimingInspector();

	void   Start();
	void   End();

	String Dump() const;

	class Routine {
	public:
		Routine(TimingInspector& stat) : stat(stat) { stat.Start(); }
		~Routine()                                  { stat.End(); }

	protected:
		TimingInspector& stat;
	};

	static void Activate(bool b)                    { active = b; }
};

class HitCountInspector
{
public:
	HitCountInspector(const char *name, int hitcount = 0) : name(name), hitcount(hitcount) {}
	~HitCountInspector();

	void Step()              { hitcount++; }
	void Add(int i)          { hitcount += i; }
	void operator ++ ()      { Step(); }
	void operator += (int i) { Add(i); }

private:
	const char *name;
	int         hitcount;
};

#define RTIMING(x) \
	static TimingInspector COMBINE(sTmStat, __LINE__)(x); \
	TimingInspector::Routine COMBINE(sTmStatR, __LINE__)(COMBINE(sTmStat, __LINE__));
#define RHITCOUNT(n) \
	{ static HitCountInspector hitcount(n); hitcount.Step(); }
#define RACTIVATE_TIMING()    TimingInspector::Activate(true);
#define RDEACTIVATE_TIMING()  TimingInspector::Activate(false);
