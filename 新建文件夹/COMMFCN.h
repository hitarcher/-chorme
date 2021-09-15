#ifndef __JSRCB_DEV_DRIVE_COMM_FUNCTION_H__
#define __JSRCB_DEV_DRIVE_COMM_FUNCTION_H__

#ifdef DEBUG
#pragma comment(lib, "HsBased.lib")
#else
#pragma comment(lib, "CommUtils.lib")
#endif

#include "LOG2.H"
#include "CommonFun.h"
#include "RS232C.H"
#include "MD5.H"
#include "Config.h"

/************************************************************************/
/*                             通用函数宏                               */
/************************************************************************/
#define LOG_DD Automatic_log
#define LOG_LEVEL Automatic_level


/************************************************************************/
/*                              常    量                                */
/************************************************************************/
#define LOG_MSG "信息"
#define LOG_ERR "错误"

#define RETENTIONLOGDAYS	(30)



/************************************************************************/
/*                            全局函数申明                              */
/************************************************************************/
BOOL Automatic_log(BYTE bType, LPCTSTR strFile, LPCTSTR lpszFormat, ...);
BOOL Automatic_level(UINT uLevel);

//CString GetCurTime(TIME_TYPE nType);

//从文件中读取内容
CString ReadFileContent(const char* pFile);

//把字符串写到文件
BOOL WriteFileContent(const char* pFile, CString strContent);

//获取HTTP请求的内容
BOOL GetHttpMsg(CString strHttpAddr, CString& strHtml);

CString SendURLPost(CString strServerName, CString strFormActionUrl, CString strPostStr);

CString GetFileName(CString strFilePath);

/************************************************************************/
/*                            全局变量申明                              */
/************************************************************************/
extern CConfig	g_Config;

#endif // __JSRCB_DEV_DRIVE_COMM_FUNCTION_H__
