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
/*                             ͨ�ú�����                               */
/************************************************************************/
#define LOG_DD Automatic_log
#define LOG_LEVEL Automatic_level


/************************************************************************/
/*                              ��    ��                                */
/************************************************************************/
#define LOG_MSG "��Ϣ"
#define LOG_ERR "����"

#define RETENTIONLOGDAYS	(30)



/************************************************************************/
/*                            ȫ�ֺ�������                              */
/************************************************************************/
BOOL Automatic_log(BYTE bType, LPCTSTR strFile, LPCTSTR lpszFormat, ...);
BOOL Automatic_level(UINT uLevel);

//CString GetCurTime(TIME_TYPE nType);

//���ļ��ж�ȡ����
CString ReadFileContent(const char* pFile);

//���ַ���д���ļ�
BOOL WriteFileContent(const char* pFile, CString strContent);

//��ȡHTTP���������
BOOL GetHttpMsg(CString strHttpAddr, CString& strHtml);

CString SendURLPost(CString strServerName, CString strFormActionUrl, CString strPostStr);

CString GetFileName(CString strFilePath);

/************************************************************************/
/*                            ȫ�ֱ�������                              */
/************************************************************************/
extern CConfig	g_Config;

#endif // __JSRCB_DEV_DRIVE_COMM_FUNCTION_H__
