#pragma once

#ifndef __MY_OS__H__
#define __MY_OS__H__

#include "stdafx.h"
#include <fstream>
#include <sstream>

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
// 创建线程
inline
DWORD InitThread(HANDLE &p_h, HANDLE &p_hEvent, _In_ LPTHREAD_START_ROUTINE lpStartAddress, _In_opt_ __drv_aliasesMem LPVOID lpParameter) {
	DWORD dwThreadId = 0;
	p_h = CreateThread(NULL, 0, lpStartAddress, lpParameter, 0, &dwThreadId);
	p_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	return dwThreadId;
}

// 释放线程
inline
void UnintThread(HANDLE &p_h, HANDLE &p_hEvent, BOOL &bExit) {
	bExit = true;
	SetEvent(p_hEvent);
	WaitForSingleObject(p_h, 10000);
}

// 截屏
inline
BOOL GetScreenShot(std::string &encodedImage)
{
	//////////////////////////////////////////////////////////////////////////
	// 参考：https://stackoverflow.com/questions/36544155/converting-a-screenshot-bitmap-to-jpeg-in-memory
	// 参考：https://stackoverflow.com/questions/3291167/how-can-i-take-a-screenshot-in-a-windows-application
	// 参考：https://stackoverflow.com/questions/33305379/c-bitmap-to-base64
	//////////////////////////////////////////////////////////////////////////

	// get the device context of the screen
	HDC hScreenDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	// and a device context to put it in
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

	int width = GetDeviceCaps(hScreenDC, HORZRES);
	int height = GetDeviceCaps(hScreenDC, VERTRES);

	// maybe worth checking these are positive values
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);

	// get a new bitmap
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);

	// screenshot to jpg and save to stream
	fstream fi;
	vector<BYTE> buf;
	IStream *stream = NULL;
	HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &stream);
	CImage image;
	ULARGE_INTEGER liSize;

	image.Attach(hBitmap);
	image.Save(stream, Gdiplus::ImageFormatJPEG);
	IStream_Size(stream, &liSize);
	DWORD len = liSize.LowPart;
	IStream_Reset(stream);
	buf.resize(len);
	IStream_Read(stream, &buf[0], len);
	stream->Release();

	// just testing if the buf contains the correct data
	fi.open("data/screenshot.jpg", fstream::binary | fstream::out);
	fi.write(reinterpret_cast<const char*>(&buf[0]), buf.size() * sizeof(BYTE));
	fi.close();

	// to base64
	encodedImage = base64_encode(reinterpret_cast<const unsigned char*>(&buf[0]), buf.size() * sizeof(BYTE));

	// clean up
	DeleteDC(hMemoryDC);
	DeleteDC(hScreenDC);
	return TRUE;
}

// 获取mac地址
// http://study.marearts.com/2017/01/get-mac-address-in-mfc.html
inline
CString GetMACAddress()
{
	CString strGateWay = _T("");
	CString strMACAddress = _T("");
	IP_ADAPTER_INFO ipAdapterInfo[10];
	DWORD dwBuflen = sizeof(ipAdapterInfo);

	DWORD dwStatus = GetAdaptersInfo(ipAdapterInfo, &dwBuflen);
	if (dwStatus != ERROR_SUCCESS)
	{
		strMACAddress.Format(_T("Error for GetAdaptersInfo : %d"), dwStatus);
		return strMACAddress;
	}
	PIP_ADAPTER_INFO pIpAdapterInfo = ipAdapterInfo;
	do {
		strGateWay = (CString)pIpAdapterInfo->GatewayList.IpAddress.String;
		if (strGateWay[0] == '0')
		{
			pIpAdapterInfo = pIpAdapterInfo->Next;
		}
		else
		{
			strMACAddress.Format(_T("%02X-%02X-%02X-%02X-%02X-%02X"),
				pIpAdapterInfo->Address[0],
				pIpAdapterInfo->Address[1],
				pIpAdapterInfo->Address[2],
				pIpAdapterInfo->Address[3],
				pIpAdapterInfo->Address[4],
				pIpAdapterInfo->Address[5]
			);

			break;
		}
	} while (pIpAdapterInfo);

	return strMACAddress;
}

// 通过ip获取mac地址
// http://study.marearts.com/2017/01/get-mac-address-in-mfc.html
#include <nb30.h>
inline
#pragma  comment(lib, "Netapi32.lib")
CString GetMacAddressbyIP(CString strIP)
{
	NCB Ncb;
	UCHAR uRetCode;
	LANA_ENUM lenum;
	int i;
	CString strOutput = _T("");
	CString string;
	ADAPTER_STATUS Adapter;
	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (UCHAR *)&lenum;
	Ncb.ncb_length = sizeof(lenum);
	uRetCode = Netbios(&Ncb);
	for (i = 0; i < lenum.length; i++)
	{
		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBRESET;
		Ncb.ncb_lana_num = lenum.lana[i];
		uRetCode = Netbios(&Ncb);

		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBASTAT;
		Ncb.ncb_lana_num = lenum.lana[i];
		strcpy_s((char*)Ncb.ncb_callname, sizeof(Ncb.ncb_callname), (LPSTR)(LPCTSTR)strIP.GetBuffer(0));
		Ncb.ncb_buffer = (unsigned char *)&Adapter;
		Ncb.ncb_length = sizeof(Adapter);
		uRetCode = Netbios(&Ncb);
		if (uRetCode == 0)
		{
			string.Format(_T("%02X:%02X:%02X:%02X:%02X:%02X"),
				Adapter.adapter_address[0],
				Adapter.adapter_address[1],
				Adapter.adapter_address[2],
				Adapter.adapter_address[3],
				Adapter.adapter_address[4],
				Adapter.adapter_address[5]);
			strOutput += string;
		}
	}

	return strOutput;
}

// 判断是否是64位系统
// https://www.yisu.com/zixun/352330.html
inline
bool is_64bitsystem()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		return true;
	else
		return false;
}

/* voice control start */
#pragma comment( lib, "winmm" )
#include <mmsyscom.h>
#include <mmeapi.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

// 声音调节
inline
int voice_volume(float volume)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) return 1;

	IMMDeviceEnumerator *deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
	IMMDevice *defaultDevice = NULL;
	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
	if (FAILED(hr)) return 2;

	deviceEnumerator->Release();
	deviceEnumerator = NULL;

	IAudioEndpointVolume *endpointVolume;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume)
		, CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&endpointVolume));
	if (FAILED(hr)) return 3;

	BOOL currentMute;
	hr = endpointVolume->GetMute(&currentMute);
	if (FAILED(hr)) return 4;

	if (currentMute == TRUE)
	{
		hr = endpointVolume->SetMute(FALSE, NULL);
	}
	float currentVolume;
	hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume); //把主音量的水平标量
	if (FAILED(hr)) return 5;

	CString str;
	str.Format("Current Volume is: %f", currentVolume);

	hr = endpointVolume->SetMasterVolumeLevelScalar(volume / 100, NULL);
	if (FAILED(hr)) return 6;

	return 0;
}
/* voice control end */

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