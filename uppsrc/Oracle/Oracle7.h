#ifndef ORACLE7_H
#define ORACLE7_H

#include <Sql/Sql.h>
#include "OraCommon.h"

class Oracle7 : public SqlSession {
public:
	virtual void                  Begin();
	virtual void                  Commit();
	virtual void                  Rollback();

	virtual String                Savepoint();
	virtual void                  RollbackTo(const String& savepoint);

	virtual bool                  IsOpen() const;

	virtual RunScript             GetRunScript() const          { return &OraclePerformScript; }

	virtual Vector<String>        EnumUsers();
	virtual Vector<String>        EnumDatabases();
	virtual Vector<String>        EnumTables(String database);
	virtual Vector<String>        EnumViews(String database);
	virtual Vector<String>        EnumSequences(String database);
	virtual Vector<String>        EnumPrimaryKey(String database, String table);
	virtual String                EnumRowID(String database, String table);
	virtual Vector<String>        EnumReservedWords();

protected:
	virtual SqlConnection        *CreateConnection();

protected:
	friend class OCI7Connection;

	Link<OCI7Connection> clink;
	bool                 connected;
	byte                 lda[256];
	byte                 hda[512];
	String               user;
	int                  level;
	bool                 autocommit;
	int                  tmode;

	String         Spn();
	void           SetSp();
	String         GetErrorMsg(int code) const;
	void           PreExec();
	void           PostExec();

public:
	bool Open(const String& s);
	void Close();

	enum TransactionMode {
		NORMAL,              // autocommit at level 0, no Commit or Rollback allowed at level 0
		ORACLE               // Oracle-style Commit and Rollback at level 0
	};

	void    SetTransactionMode(int mode)            { tmode = mode; }

	Oracle7();
	~Oracle7();
};

typedef Oracle7 OracleSession;

#endif
