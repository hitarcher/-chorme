#if !defined(AFX_DBSQLITEFUN_H__DC7062D9_6FF6_4E17_BBD9_87ECF348542B__INCLUDED_)
#define AFX_DBSQLITEFUN_H__DC7062D9_6FF6_4E17_BBD9_87ECF348542B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DBSqliteFun.h : header file
//
#include <sqlite3.h>

/////////////////////////////////////////////////////////////////////////////
typedef int (*Sqlite3_Callback)(void*,int,char**, char**);

class CDBSqliteFun
{
// Construction
public:
	CDBSqliteFun();
	virtual ~CDBSqliteFun();
private:
	sqlite3* m_pSqliteDB;
	char*	 m_pError;		//对数据库操作出错
	char*	 m_pSqlError;	//专指执行sql语句出错
	
public:
	int OpenDatabase(CString strDbPath);	///返回为0 表示打开成功
	int CloseDatabase();					///返回0 表示关闭成功
	int ExeSqlTransaction(const char *pSql);		///执行sql语句
	int GetTable(int iTableType);				///获取单个排队机相关表格
public:
	char* GetLastError() {return m_pError;}
	char* GetLastSqlError() {return m_pSqlError;}
	const char* Sqlite3_errmsg() {return sqlite3_errmsg(m_pSqliteDB);}
	int  Sqlite3_get_table(const char *zSql, char ***pazResult, int *pnRow, int *pnColumn);
	int Sqlite3_prepare(const char *zSql,  int nByte,  sqlite3_stmt **ppStmt,  const char **pzTail);

protected:

};

#endif // !defined(AFX_DBSQLITEFUN_H__DC7062D9_6FF6_4E17_BBD9_87ECF348542B__INCLUDED_)
