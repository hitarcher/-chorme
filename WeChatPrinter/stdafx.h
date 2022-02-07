
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的
 
// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS
#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持
#include <afxdisp.h>



#define LOG_NAME_SHELL "SHELL.LOG"

#define LOG_NAME_TRADE "TRADE.LOG"						//平台交易日志
#define LOG_NAME_DEBUG "DEBUG.LOG"						//运行日志
#define LOG_NAME_CEF "CEF.LOG"						//运行日志
#define LOG_NAME_RMQ "RMQ.LOG"							//RMQ消息日志
#define LOG_NAME_OFFLINE "OFFLINE.LOG"					//心跳日志
#define LOG_NAME_MEMORY "MEMORY.LOG"					//内存日志
#define LOG_NAME_ZIP "ZIP.LOG"							//素材压缩日志

#define INFO_PUBLISH_SCREEN_NAME "INFO_PUBLISH_SCREEN"


//#define TESTMODE		//开启测试模式



#ifdef TESTMODE
// #define CHECKUPDATE	//启动自动更新
#define	ZIPIMGANYWAY	//无视系统内存，直接压缩图片素材
#define	DELAY_TIME 1	//延迟启动时间

#else 
#define CHECKUPDATE		//启动自动更新
#define	DELAY_TIME 15	//延迟启动时间
//#define	AVOIDREPEATRUN	//避免重复运行

#endif

//#define  STARTHOOK			//启动钩子
//#define  DETAIlEDLOG			//启用详细日志  
//#define  CLOSESCRECT			//关闭加密
//#define  DOWNLOAD_NO			//关闭下载

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


