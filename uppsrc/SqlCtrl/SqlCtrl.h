#ifndef SQLCTRL_H
#define SQLCTRL_H

#include <Sql/Sql.h>
#include <CtrlLib/CtrlLib.h>

int  SqlError(const char *text, const char *error, const char *statement, bool retry = false);
int  SqlError(const char *text, const SqlSession& session, bool retry = false);
int  SqlError(const char *text, const Sql& sql APPSQLCURSOR, bool retry = false);

#ifdef PLATFORM_WIN32
int  SqlError(HWND parent, const char *text, const char *error, const char *statement, bool retry = false);
int  SqlError(HWND parent, const char *text, const SqlSession& session, bool retry = false);
int  SqlError(HWND parent, const char *text, const Sql& sql APPSQLCURSOR, bool retry = false);
#endif

bool   DisplayError(const SqlSession& session, const char *msg = NULL);
bool   ShowError(SqlSession& session, const char *msg = NULL);
bool   ErrorRollback(SqlSession& session, const char *emsg = NULL);
bool   OkCommit(SqlSession& session, const char *emsg = NULL);

bool   DisplayError(const Sql& sql, const char *msg = NULL);
bool   ShowError(Sql& sql, const char *msg = NULL);
bool   ErrorRollback(Sql& sql, const char *emsg = NULL);
bool   OkCommit(Sql& sql, const char *emsg = NULL);

#ifndef NOAPPSQL
bool   DisplayError(const char *msg = NULL);
bool   ShowError(const char *msg = NULL);
bool   ErrorRollback(const char *emsg = NULL);
bool   OkCommit(const char *emsg = NULL);
#endif

void SqlLoad(MapConvert& cv, Sql& sql);
void SqlLoad(MapConvert& cv, const SqlSet& set, SqlSession& ss APPSQLSESSION);
#ifndef NOAPPSQL
void operator*=(MapConvert& cv, const SqlSet& set);
#endif

void SqlLoad(DropList& dl, Sql& sql);
void SqlLoad(DropList& dl, const SqlSet& set, SqlSession& ss APPSQLSESSION);
#ifndef NOAPPSQL
void operator*=(DropList& cv, const SqlSet& set);
#endif

class SqlOption : public Option {
public:
	virtual void   SetData(const Value& data);
	virtual Value  GetData() const;
};

class SqlArray : public ArrayCtrl {
public:
	virtual void  SetData(const Value& v);
	virtual Value GetData() const;
	virtual bool  Accept();

protected:
	virtual bool  UpdateRow();
	virtual void  RejectRow();

private:
	SqlSession *ssn;
	SqlId       table;
	Value       fkv;
	SqlId       fk;
	SqlBool     where;
	SqlSet      orderby;
	int         querytime;
	int         count;
	int64       offset;
	bool        lateinsert;
	bool        goendpostquery;
	bool        autoinsertid;

	SqlBool     GetWhere();
#ifndef NOAPPSQL
	SqlSession& Session()                                  { return ssn ? *ssn : SQL.GetSession(); }
#else
	SqlSession& Session()                                  { ASSERT(ssn); return *ssn; }
#endif

	bool        PerformInsert();
	bool        PerformDelete();

	void      Inserting();

public:
	Callback  WhenPostQuery;

	void      StdBar(Bar& menu);
	bool      CanInsert() const;
	void      StartInsert();
	void      StartDuplicate();
	void      DoRemove();

	void      SetSession(SqlSession& _session)             { ssn = &_session; }

	void      Join(SqlId fk, ArrayCtrl& master);
	void      Join(ArrayCtrl& master);

	void      Query();
	void      AppendQuery(SqlBool where);
	void      Query(SqlBool where)                         { SetWhere(where); Query(); }

	void      Limit(int _offset, int _count)               { offset = _offset; count = _count; }
	void      Limit(int count)                             { Limit(0, count); }

	SqlArray& SetTable(SqlId _table)                       { table = _table; return *this; }
	SqlArray& SetWhere(SqlBool _where)                     { where = _where; return *this;  }
	SqlArray& SetOrderBy(SqlSet _orderby)                  { orderby = _orderby; return *this; }
	SqlArray& SetOrderBy(const SqlVal& a)                  { return SetOrderBy(SqlSet(a)); }
	SqlArray& SetOrderBy(const SqlVal& a, const SqlVal& b) { return SetOrderBy(SqlSet(a, b)); }
	SqlArray& SetOrderBy(const SqlVal& a, const SqlVal& b, const SqlVal& c)
	                                                       { return SetOrderBy(SqlSet(a, b, c)); }
	SqlArray& GoEndPostQuery(bool b = true)                { goendpostquery = b; return *this; }
	SqlArray& AutoInsertId(bool b = true)                  { autoinsertid = b; return *this; }
	SqlArray& AppendingAuto()                              { Appending(); return AutoInsertId(); }

	void      Clear();
	void      Reset();

	typedef   SqlArray CLASSNAME;

	SqlArray();
};

class SqlCtrls {
	struct Item {
		SqlId id;
		Ctrl *ctrl;
	};
	Array<Item> item;

public:
	void      Add(SqlId id, Ctrl& ctrl);
	SqlCtrls& operator()(SqlId id, Ctrl& ctrl)       { Add(id, ctrl); return *this; }
	SqlSet    Set() const;
	operator  SqlSet() const                         { return Set(); }
	void      Read(Sql& sql);
	bool      Fetch(Sql& sql);
#ifndef NOAPPSQL
	bool      Fetch()                                { return Fetch(SQL); }
#endif
	bool      Load(Sql& sql, SqlSet set)             { sql * set; return Fetch(sql); }
#ifndef NOAPPSQL
	bool      Load(SqlSet set)                       { return Load(SQL, set); }
#endif
	bool      Load(Sql& sql, SqlId table, SqlBool where);
#ifndef NOAPPSQL
	bool      Load(SqlId table, SqlBool where);
#endif
	void      Insert(SqlInsert& insert) const;
	void      Update(SqlUpdate& update) const;
	void      UpdateModified(SqlUpdate& update) const;
	SqlInsert Insert(SqlId table) const;
	SqlUpdate Update(SqlId table) const;
	SqlUpdate UpdateModified(SqlId table) const;
	bool      Accept();
	void      ClearModify();
	bool      IsModified();
	void      Enable(bool b = true);
	void      Disable()                              { Enable(false); }
	void      SetNull();
	Callback  operator<<=(Callback cb);

	int         GetCount() const                     { return item.GetCount(); }
	Ctrl&       operator[](int i)                    { return *item[i].ctrl; }
	const Ctrl& operator[](int i) const              { return *item[i].ctrl; }
	SqlId       operator()(int i) const              { return item[i].id; }

	void      Reset()                                { item.Clear(); }
};

class SqlDetail : public StaticRect {
public:
	virtual void  SetData(const Value& v);
	virtual Value GetData() const;
	virtual bool  Accept();

private:
	SqlSession *ssn;
	SqlCtrls    ctrls;
	SqlId       table;
	Value       fkval;
	SqlId       fk;
	bool        present;
	bool        autocreate;

#ifndef NOAPPSQL
	SqlSession& Session()                          { return ssn ? *ssn : SQL.GetSession(); }
#else
	SqlSession& Session()                          { ASSERT(ssn); return *ssn; }
#endif

	void        Query();

public:
	Callback    WhenPostQuery;

	bool        IsPresent() const                  { return present; }
	bool        Create();
	bool        Delete();

	void        SetSession(SqlSession& _session)   { ssn = &_session; }

	SqlDetail&  Add(SqlId id, Ctrl& ctrl);
	SqlDetail&  operator()(SqlId id, Ctrl& ctrl)   { return Add(id, ctrl); }

	SqlDetail&  Join(SqlId fk, ArrayCtrl& master);
	SqlDetail&  Join(ArrayCtrl& master);
	SqlDetail&  SetTable(SqlId _table)             { table = _table; return *this; }
	SqlDetail&  AutoCreate(bool b = true)          { autocreate = b; return *this; }

	void        Reset();

	SqlDetail();
};

void        SQLCommander(SqlSession& session);
#ifndef NOAPPSQL
inline void SQLCommander() { SQLCommander(SQL.GetSession()); }
#endif
void        SQLObjectTree(SqlSession& session APPSQLSESSION);

#endif
