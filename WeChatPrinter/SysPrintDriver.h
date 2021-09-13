#pragma once
#include "CommonFun.h"
#include "RN_API.h"


class CSysPrintDriver
{
public:
	CSysPrintDriver(void);
	~CSysPrintDriver(void);

	BOOL LoadDll();
	CString GetLastErr(){ return m_strLastErr;}

private:
	HMODULE m_hSysPrint;
	CString m_strLastErr;

private:
	typedef DWORD (WINAPI *pInitSysPrinter)();
	typedef DWORD (WINAPI *pPrintTextAndPicByXmlFile)();
	typedef DWORD (WINAPI *pCancelPrinterData)();
	typedef DWORD (WINAPI *pPrintByPicture)(int nPox, int nPoy, double nPercent, LPCSTR strPic);
	typedef DWORD (WINAPI *pPrintByPicture2)(int nPox, int nPoy, int nWidth, int nHeight, LPCSTR strPic);
	typedef DWORD (WINAPI *pGetPrinterStatus)(DWORD* dwStatus, LPVOID lpStatus);

public:
	pInitSysPrinter InitSysPrinter;
	pPrintByPicture PrintByPicture;
	pPrintByPicture2 PrintByPicture2;
	pGetPrinterStatus GetPrinterStatus;
	pPrintTextAndPicByXmlFile PrintTextAndPicByXmlFile;
	pCancelPrinterData CancelPrinterData;
};

