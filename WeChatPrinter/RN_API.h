/*****************************************************************************
  *  @COPYRIGHT NOTICE
  *  @Copyright (c) 2015, REIN
  *  @All rights reserved
  *  @file	 : RN_API.h
  *  @version  : ver 1.0
  *  @author   : Sunming
  *  @date     : 2016/2/2 15:02
  *  @brief    : 
  *  @detail   : 系统打印机头文件
*****************************************************************************/

#ifndef _RN_API_H_
#define _RN_API_H_
#define RN_API_EXPORTS 

#include <windows.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN  
#endif

#if defined(_UNICODE) || defined(UNICODE)
    #error must use EJCRile switch mbcs instead of unicode.
#endif

#ifdef RN_API_EXPORTS
    #define RN_API __declspec(dllexport) WINAPI 
#else
    #define RN_API __declspec(dllimport) WINAPI 
#endif

#ifdef __cplusplus
extern "C"{
#endif 

typedef DWORD RN_RESULT;

typedef DWORD RN_STATUS;

enum LOGLEVEL
{
	LOGLEVEL_NULL   = 0,
	LOGLEVEL_ERR    = 1,
	LOGLEVEL_ENTER  = 2,
	LOGLEVEL_DETAIL = 3
};

enum PRINTERSTATUS
{
	PRINTER_ONREADY		= 0,
	PRINTER_NOTREADY    = 1,
	PRINTER_PRINTING	= 2,
	PRINTER_PAPEROUT	= 3,
	PRINTER_ERROR		= 4,
	PRINTER_UNKNOW		= 5
};

#define RN_OK   0

/*****************************************************************************/
/*                     系统打印机返回值                                         */
/*****************************************************************************/

#define RN_SYPT_PORT_OPEN_ERROR         6001
#define RN_SYPT_DEVICE_INIT_ERROR       6002
#define RN_SYPT_NOT_SUPPORTED           6003
#define RN_SYPT_NOT_INITIALIZED         6004
#define RN_SYPT_COMMUNICATION_ERROR     6005
#define RN_SYPT_BUSY                    6006
#define RN_SYPT_PAPER_JAMMED            6021
#define RN_SYPT_PAPER_OUT               6022
#define RN_SYPT_HEAD_ERROR              6023
#define RN_SYPT_STACK_FULL              6024
#define RN_SYPT_CUT_ERROR               6025
#define RN_SYPT_INTERNAL_ERROR          6049
#define RN_SYPT_INVALID_CONFIG_INI      6051
#define RN_SYPT_DATA_FORMAT_ERROR       6052
#define RN_SYPT_LOGO_ERROR              6053

#ifdef __cplusplus
}
#endif

#endif


