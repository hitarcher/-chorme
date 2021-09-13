// DBSqliteFun.cpp : implementation file
//

#include "stdafx.h"
#include "DBSqliteFun.h"
#include "FileStruct.h"
#include "../配置/Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

int GBKToUTF8(unsigned char * lpGBKStr,unsigned char * lpUTF8Str,int nUTF8StrLen)
{
    wchar_t * lpUnicodeStr = NULL;
    int nRetLen = 0;
	
	//如果GBK字符串为NULL则出错退出
    if(!lpGBKStr) return 0;
	
	//获取转换到Unicode编码后所需要的字符空间长度
    nRetLen = ::MultiByteToWideChar(CP_ACP,0,(char *)lpGBKStr,-1,NULL,NULL);  
	//为Unicode字符串空间
    lpUnicodeStr = new WCHAR[nRetLen + 1];  
	//转换到Unicode编码
    nRetLen = ::MultiByteToWideChar(CP_ACP,0,(char *)lpGBKStr,-1,lpUnicodeStr,nRetLen);
	//转换失败则出错退出
    if(!nRetLen) return 0;
	
	//获取转换到UTF8编码后所需要的字符空间长度
    nRetLen = ::WideCharToMultiByte(CP_UTF8,0,lpUnicodeStr,-1,NULL,0,NULL,NULL);  
    
	if(!lpUTF8Str)  //输出缓冲区为空则返回转换后需要的空间大小
    {
        if(lpUnicodeStr) delete []lpUnicodeStr;
        return nRetLen;
    }
    
    if(nUTF8StrLen < nRetLen)  //如果输出缓冲区长度不够则退出
    {
        if(lpUnicodeStr) delete []lpUnicodeStr;
        return 0;
    }
	
	//转换到UTF8编码
    nRetLen = ::WideCharToMultiByte(CP_UTF8,0,lpUnicodeStr,-1,(char *)lpUTF8Str,nUTF8StrLen,NULL,NULL);  
    
    if(lpUnicodeStr) delete []lpUnicodeStr;
    
    return nRetLen;
}

CDBSqliteFun::CDBSqliteFun()
{
	m_pSqliteDB = NULL;
}

CDBSqliteFun::~CDBSqliteFun()
{
	CloseDatabase();
}

int CDBSqliteFun::OpenDatabase(CString strDbPath)
{
	char szpath[1024] = {0};
	GBKToUTF8((unsigned char*)strDbPath.GetBuffer(0),(unsigned char*)szpath,sizeof(szpath));
	return sqlite3_open(szpath,&m_pSqliteDB);
}

int CDBSqliteFun::CloseDatabase()
{
	return sqlite3_close(m_pSqliteDB);;
}

int CDBSqliteFun::ExeSqlTransaction(const char *pSql)
{
	int ret = 0;
	ret  = sqlite3_exec(m_pSqliteDB, "begin transaction", 0, 0, &m_pError); //开始一个事务
	if(ret != SQLITE_OK)
	{
		return ret;
	}
	ret = sqlite3_exec(m_pSqliteDB, pSql, 0, 0, &m_pSqlError);
	if(ret != SQLITE_OK)
	{
		sqlite3_exec(m_pSqliteDB, "rollback transaction", 0, 0, &m_pError); //回滚事务
		return ret;
	}
	return sqlite3_exec( m_pSqliteDB, "commit transaction", 0, 0, &m_pError); //提交事务
}

int CDBSqliteFun::Sqlite3_get_table(const char *zSql, char ***pazResult, int *pnRow, int *pnColumn)
{
	return sqlite3_get_table(m_pSqliteDB,zSql,pazResult,pnRow,pnColumn,&m_pSqlError);
}

int CDBSqliteFun::Sqlite3_prepare(const char *zSql,  int nByte,  sqlite3_stmt **ppStmt,  const char **pzTail)
{
	return sqlite3_prepare(m_pSqliteDB,zSql,nByte,ppStmt,pzTail);
}
