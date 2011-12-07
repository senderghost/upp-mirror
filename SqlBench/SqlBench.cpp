#include <plugin/sqlite3/Sqlite3.h>

using namespace Upp;

#define MODEL <SqlBench/test.sch>

#define SCHEMADIALECT <plugin/sqlite3/Sqlite3Schema.h>
#include "Sql/sch_header.h"

#ifdef _DEBUG
#include <Sql/sch_schema.h>
#endif

#include <Sql/sch_source.h>

void Test(String a, String b)
{
	String c = a + b;
	Cout() << c;
}

void Test1(String& a, String& b)
{
	String c = a + b;
	Cout() << c;
}

void Test2(SqlId a, SqlId b)
{
	String c = ~a + ~b;
	Cout() << c;
}

String alfa = "0", beta = "1";

CONSOLE_APP_MAIN
{

/*
	DDUMP(~FindSchJoin("TABLE1,TABLE2"));
	DDUMP(~FindSchJoin("TABLE2,TABLE1"));
*/	
	SqlId myid("FOO");
	
	RDUMP(~myid);
	RDUMP(~myid);
	
	
	Test(alfa, beta);
	Test(alfa, beta);
	Test1(alfa, beta);
	Test2(NAME, LASTNAME);

	LOG("-- Starting new run of Sqlite3 simple test");
#ifdef _DEBUG
	LOG("Debug mode!");
#endif

	Sqlite3Session sqlite3;
	sqlite3.LogErrors(true);
	if(!sqlite3.Open(ConfigFile("simple.db"))) {
		LOG("Can't create or open database file\n");
		return;
	}

	SQL = sqlite3;

	// Update the schema to match the schema described in "simple.sch"
#ifdef _DEBUG
	SqlSchema sch(SQLITE3);
	StdStatementExecutor se(sqlite3);
	All_Tables(sch);
	if(sch.ScriptChanged(SqlSchema::UPGRADE))
		Sqlite3PerformScript(sch.Upgrade(),se);
	if(sch.ScriptChanged(SqlSchema::ATTRIBUTES)) {
		Sqlite3PerformScript(sch.Attributes(),se);
	}
	if(sch.ScriptChanged(SqlSchema::CONFIG)) {
		Sqlite3PerformScript(sch.ConfigDrop(),se);
		Sqlite3PerformScript(sch.Config(),se);
	}
	sch.SaveNormal();
#endif

	Sql sql;
	sql*Insert(TABLE1)(ID,0)(NAME,"Joe")(LASTNAME,"Smith")(BDATE,20000101);
	LOG(sql.ToString());
	sql*Insert(TABLE1)(ID,1)(NAME,"Mike")(LASTNAME,"Smith")(BDATE,20000102);
	LOG(sql.ToString());
	sql*Insert(TABLE1)(ID,2)(NAME,"Jon")(LASTNAME,"Goober")(BDATE,20000103);
	LOG(sql.ToString());

	RLOG(sql.Compile(
		Select(ID(ID, NAME, LASTNAME))
		.From(TABLE1)
	    .InnerJoinRef(TABLE2)
	    .Where(BDATE == GetSysDate())));

	RLOG(sql.Compile(
		Select(ID(ID, NAME, LASTNAME))
		.From(TABLE1)
	    .RightJoinRef(TABLE2).On(IsNull(BDATE))
	    .Where(BDATE == GetSysDate())));

	RDUMP(sql.Compile(Select(ID(ID, NAME, LASTNAME)).From(TABLE1).Where(BDATE == GetSysDate())));
	RDUMP(sql.Compile(Select(ID(ID, NAME, LASTNAME)).From(TABLE1).Where(BDATE == GetSysDate())));
	RDUMP(sql.Compile(Select(ID(NAME)).From(TABLE1).Where(BDATE == GetSysDate())));


	RDUMP(sql.Compile(Select(NAME(ID, NAME, LASTNAME), LASTNAME)
	                  .From(TABLE1).Where(BDATE == GetSysDate())));

	return;

	String x;
	Date d = GetSysDate();
	for(int i = 0; i < 1000000; i++) {
		SqlSelect s;
		{
			RTIMING("Create sql select");
			s = Select(ID, NAME, LASTNAME).From(TABLE1).Where(BDATE == d);
		}
		{
			RTIMING("Compile sql select");
			x = sql.Compile(s);
		}
	}

	for(int i = 0; i < 10000; i++) {
		RTIMING("Create sql execute");
		
		SQL * Select(ID, NAME, LASTNAME).From(TABLE1).Where(BDATE == i);
	}
}