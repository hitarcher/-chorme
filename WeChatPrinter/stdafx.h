
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��
 
// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS
#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��
#include <afxdisp.h>



#define LOG_NAME_SHELL "SHELL.LOG"

#define LOG_NAME_TRADE "TRADE.LOG"						//ƽ̨������־
#define LOG_NAME_DEBUG "DEBUG.LOG"						//������־
#define LOG_NAME_CEF "CEF.LOG"						//������־
#define LOG_NAME_RMQ "RMQ.LOG"							//RMQ��Ϣ��־
#define LOG_NAME_OFFLINE "OFFLINE.LOG"					//������־
#define LOG_NAME_MEMORY "MEMORY.LOG"					//�ڴ���־
#define LOG_NAME_ZIP "ZIP.LOG"							//�ز�ѹ����־

#define INFO_PUBLISH_SCREEN_NAME "INFO_PUBLISH_SCREEN"


//#define TESTMODE		//��������ģʽ



#ifdef TESTMODE
// #define CHECKUPDATE	//�����Զ�����
#define	ZIPIMGANYWAY	//����ϵͳ�ڴ棬ֱ��ѹ��ͼƬ�ز�
#define	DELAY_TIME 1	//�ӳ�����ʱ��

#else 
#define CHECKUPDATE		//�����Զ�����
#define	DELAY_TIME 15	//�ӳ�����ʱ��
//#define	AVOIDREPEATRUN	//�����ظ�����

#endif

//#define  STARTHOOK			//��������
//#define  DETAIlEDLOG			//������ϸ��־  
//#define  CLOSESCRECT			//�رռ���
//#define  DOWNLOAD_NO			//�ر�����

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


