#pragma once

#ifndef __MY_OS__H__
#define __MY_OS__H__

#include <fstream>
#include <sstream>
#include <tchar.h>

#include "mystring.h"

/************************************************************************/
/*                             文件操作                                 */
/************************************************************************/
#define exist(pszFile) ((-1 == _taccess(pszFile, 0)) ? false:true)

#include <filesystem>
inline
bool mkdir(const char* folder)
{
	try
	{
		std::experimental::filesystem::create_directories(folder);
	}
	catch (const std::exception&)
	{
		return false;
	}
	return true;
}

inline 
void rmdir(const char* folder)
{
	std::experimental::filesystem::remove_all(folder);
}

inline
std::string get_filename(std::string& path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}

inline
std::string get_exefolder()
{
	char fullpath[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, fullpath, MAX_PATH);
	std::string folder = fullpath;
	return folder.substr(0, folder.find_last_of("/\\") + 1);
}

inline 
std::string get_fullpath(std::string path)
{
	return get_exefolder() + path;
}

inline
std::string easyReadData(std::string filefullpath)
{
	std::ostringstream tmp;
	std::ifstream in(filefullpath);
	tmp << in.rdbuf();
	std::string read = tmp.str();
	return read;
}

inline
void easyWriteData(std::string filefullpath, const char* data, size_t length)
{
	fstream fi;
	fi.open(filefullpath, fstream::binary | fstream::out);
	fi.write(data, length);
	fi.close();
}


/************************************************************************/
/*                                系统操作                              */
/************************************************************************/
#include <time.h>
// 获取当前是周几
inline
int weekday()
{
	std::time_t t = std::time(NULL);   // get time now
	std::tm* t_now = std::localtime(&t);
	return t_now->tm_wday;
}

// 当前时间戳
typedef enum {
	  t_date = 0		// %Y-%m-%d %H:%M:%S     | 2016-08-10 11:12:30    
	, t_date1			// %Y-%m-%d %H:%M:%S.000 | 2016-08-10 11:12:30.000
	, t_date2			// %Y%m%d%H%M%S          | 20160810111230         

	, t_day				// %Y-%m-%d              | 2016-08-10             
	, t_day1			// %Y%m%d                | 20160810               
	, t_day2			// %m%d                  | 0810                   

	, t_time			// %H:%M:%S.%d           | 11:12:30.000
	, t_time1			// %H:%M:%S              | 11:12:30               
	, t_time2 			// %H%M%S                | 111230                 
	, t_time3			// %H:%M                 | 11:12                  
	, t_time4			// %H %M                 | 11 12                  
}time_format;

inline 
std::string time2string(std::tm* tm, uint16_t milliseconds, time_format format)
{
	switch (format)
	{
	case t_date: return vstring("%04d-%02d-%02d %02d:%02d:%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	case t_date1: return vstring("%04d-%02d-%02d %02d:%02d:%02d.%03d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec, milliseconds);

	case t_date2: return vstring("%04d%02d%02d%02d%02d%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	case t_day: return vstring("%04d-%02d-%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

	case t_day1: return vstring("%04d%02d%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

	case t_day2: return vstring("%02d%02d",
		tm->tm_mon + 1, tm->tm_mday);

	case t_time: return vstring("%02d:%02d:%02d.%03d",
		tm->tm_hour, tm->tm_min, tm->tm_sec, milliseconds);

	case t_time1: return vstring("%02d:%02d:%02d",
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	case t_time2: return vstring("%02d%02d%02d",
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	case t_time3: return vstring("%02d:%02d",
		tm->tm_hour, tm->tm_min);

	case t_time4: return vstring("%02d %02d",
		tm->tm_hour, tm->tm_min);

	default:
		break;
	}
	return time2string(tm, milliseconds, t_date);
}

inline
std::tm string2time(std::string timestring, time_format format)
{
	std::tm tm;
	memset(&tm, 0, sizeof(tm));
	switch (format)
	{
	case t_date: 
		sscanf(timestring.c_str(), "%d-%d-%d %d:%d:%d"
			, &tm.tm_year
			, &tm.tm_mon
			, &tm.tm_mday
			, &tm.tm_hour
			, &tm.tm_min
			, &tm.tm_sec
		);
		break;
	case t_date1:
		sscanf(timestring.c_str(), "%d-%d-%d %d:%d:%d"
			, &tm.tm_year
			, &tm.tm_mon
			, &tm.tm_mday
			, &tm.tm_hour
			, &tm.tm_min
			, &tm.tm_sec
		);
		break;
	case t_date2:
		sscanf(timestring.c_str(), "%04d%02d%02d%02d%02d%02d"
			, &tm.tm_year
			, &tm.tm_mon
			, &tm.tm_mday
			, &tm.tm_hour
			, &tm.tm_min
			, &tm.tm_sec
		);
		break;
	case t_day:
		sscanf(timestring.c_str(), "%d-%d-%d"
			, &tm.tm_year
			, &tm.tm_mon
			, &tm.tm_mday
		);
		break;
	case t_day1:
		sscanf(timestring.c_str(), "%04d%02d%02d"
			, &tm.tm_year
			, &tm.tm_mon
			, &tm.tm_mday
		);
		break;
	case t_day2:
		sscanf(timestring.c_str(), "%02d%02d"
			, &tm.tm_mon
			, &tm.tm_mday
		);
		break;
	case t_time:
		sscanf(timestring.c_str(), "%d:%d:%d"
			, &tm.tm_hour
			, &tm.tm_min
			, &tm.tm_sec
		);
		break;
	case t_time1:
		sscanf(timestring.c_str(), "%d:%d:%d"
			, &tm.tm_hour
			, &tm.tm_min
			, &tm.tm_sec
		);
		break;
	case t_time2:
		sscanf(timestring.c_str(), "%02d%02d%02d"
			, &tm.tm_hour
			, &tm.tm_min
			, &tm.tm_sec
		);
		break;
	case t_time3:
		sscanf(timestring.c_str(), "%02d:%02d"
			, &tm.tm_hour
			, &tm.tm_min
		); 
		break;
	case t_time4:
		sscanf(timestring.c_str(), "%02d %02d"
			, &tm.tm_hour
			, &tm.tm_min
		);
		break;
	default:
		break;
	}
	
	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	return tm;
}

inline
std::string now(time_format format)
{
	std::time_t t = std::time(nullptr);
	std::tm* t_now = std::localtime(&t);
	return time2string(t_now, t % 1000, format);
}

inline
std::string now_past(time_format format, uint16_t day)
{
	std::time_t t = std::time(nullptr);
	std::tm* tm = std::localtime(&t);
	tm->tm_mday -= day;
	std::mktime(tm);
	return time2string(tm, t % 1000, format);
}

inline
std::string now_future(time_format format, uint16_t day)
{
	std::time_t t = std::time(nullptr);
	std::tm* tm = std::localtime(&t);
	tm->tm_mday += day;
	std::mktime(tm);
	return time2string(tm, t % 1000, format);
}

/************************************************************************/
/*                            user32.dll                                */
/* MessageBoxTimeout                                                    */
/*参考：https://www.codeproject.com/Articles/7914/MessageBoxTimeout-API */
/************************************************************************/
#include <windows.h>
#include <tchar.h>

//Functions & other definitions required-->
typedef int(__stdcall *MSGBOXAAPI)(IN HWND hWnd,
	IN LPCSTR lpText, IN LPCSTR lpCaption,
	IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);
typedef int(__stdcall *MSGBOXWAPI)(IN HWND hWnd,
	IN LPCWSTR lpText, IN LPCWSTR lpCaption,
	IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

#ifdef UNICODE
#define MessageBoxTimeout MessageBoxTimeoutW
#else
#define MessageBoxTimeout MessageBoxTimeoutA
#endif 

#define MB_TIMEDOUT 32000

inline
int MessageBoxTimeoutA(HWND hWnd, LPCSTR lpText,
	LPCSTR lpCaption, UINT uType, WORD wLanguageId,
	DWORD dwMilliseconds)
{
	static MSGBOXAAPI MsgBoxTOA = NULL;

	if (!MsgBoxTOA)
	{
		HMODULE hUser32 = GetModuleHandle(_T("user32.dll"));
		if (hUser32)
		{
			MsgBoxTOA = (MSGBOXAAPI)GetProcAddress(hUser32,
				"MessageBoxTimeoutA");
			//fall through to 'if (MsgBoxTOA)...'
		}
		else
		{
			//stuff happened, add code to handle it here 
			//(possibly just call MessageBox())
			return 0;
		}
	}

	if (MsgBoxTOA)
	{
		return MsgBoxTOA(hWnd, lpText, lpCaption,
			uType, wLanguageId, dwMilliseconds);
	}

	return 0;
}

inline
int MessageBoxTimeoutW(HWND hWnd, LPCWSTR lpText,
	LPCWSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds)
{
	static MSGBOXWAPI MsgBoxTOW = NULL;

	if (!MsgBoxTOW)
	{
		HMODULE hUser32 = GetModuleHandle(_T("user32.dll"));
		if (hUser32)
		{
			MsgBoxTOW = (MSGBOXWAPI)GetProcAddress(hUser32,
				"MessageBoxTimeoutW");
			//fall through to 'if (MsgBoxTOW)...'
		}
		else
		{
			//stuff happened, add code to handle it here 
			//(possibly just call MessageBox())
			return 0;
		}
	}

	if (MsgBoxTOW)
	{
		return MsgBoxTOW(hWnd, lpText, lpCaption,
			uType, wLanguageId, dwMilliseconds);
	}

	return 0;
}
//End required definitions <--

/************************************************************************/
/*                            user32.dll                                */
/* SetLayeredWindowAttributes                                           */
/************************************************************************/

typedef BOOL(__stdcall *SETLAYEREDAPI)(
	HWND     hwnd,
	COLORREF crKey,
	BYTE     bAlpha,
	DWORD    dwFlags);

inline
BOOL SetLayered(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	static SETLAYEREDAPI SetLayeredApi = NULL;

	if (!SetLayeredApi)
	{
		HMODULE hUser32 = GetModuleHandle(_T("user32.dll"));
		if (hUser32)
		{
			SetLayeredApi = (SETLAYEREDAPI)GetProcAddress(hUser32,
				"SetLayeredWindowAttributes");
			//fall through to 'if (SetLayeredApi)...'
		}
		else
		{
			//stuff happened, add code to handle it here 
			//(possibly just call SetLayeredApi())
			return FALSE;
		}
	}

	if (SetLayeredApi)
	{
		return SetLayeredApi(hwnd, crKey, bAlpha, dwFlags);
	}

	return FALSE;
}
#endif