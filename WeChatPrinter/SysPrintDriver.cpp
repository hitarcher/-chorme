#include "stdafx.h"
#include "SysPrintDriver.h"


CSysPrintDriver::CSysPrintDriver(void)
{
	InitSysPrinter = NULL;
	PrintByPicture = NULL;
	PrintByPicture2 = NULL;
	GetPrinterStatus = NULL;
	PrintTextAndPicByXmlFile = NULL;
	CancelPrinterData = NULL;
}


CSysPrintDriver::~CSysPrintDriver(void)
{
	if(m_hSysPrint)	
	{
		FreeLibrary(m_hSysPrint);
		m_hSysPrint = NULL;
	}
}

BOOL CSysPrintDriver::LoadDll()
{
	if (m_hSysPrint) {FreeLibrary(m_hSysPrint); m_hSysPrint = NULL;}
#ifdef _DEBUG
	m_hSysPrint = LoadLibraryEx(GetFullPath("SYSTEM_PRINTERd.DLL"),0,LOAD_WITH_ALTERED_SEARCH_PATH);
#else
	m_hSysPrint = LoadLibraryEx(GetFullPath("SYSTEM_PRINTER.DLL"), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
#endif
	
	if (NULL == m_hSysPrint)
	{
		AfxMessageBox("加载SYSTEM_PRINTER.DLL失败");
		return FALSE;
	}
	InitSysPrinter = (pInitSysPrinter)GetProcAddress(m_hSysPrint, "_InitSysPrinter");
	if (NULL == InitSysPrinter)
	{
		AfxMessageBox("找不到_InitSysPrinter函数");
		return FALSE;
	}
	PrintByPicture = (pPrintByPicture)GetProcAddress(m_hSysPrint, "_PrintByPicture");
	if (NULL == PrintByPicture)
	{
		AfxMessageBox("找不到_PrintByPicture函数");
		return FALSE;
	}
	PrintByPicture2 = (pPrintByPicture2)GetProcAddress(m_hSysPrint, "_PrintByPicture2");
	if (NULL == PrintByPicture2)
	{
		AfxMessageBox("找不到_PrintByPicture2函数");
		return FALSE;
	}
	GetPrinterStatus = (pGetPrinterStatus)GetProcAddress(m_hSysPrint, "_GetPrinterStatus");
	if (NULL == GetPrinterStatus)
	{
		AfxMessageBox("找不到_GetPrinterStatus函数");
		return FALSE;
	}
	PrintTextAndPicByXmlFile = (pPrintTextAndPicByXmlFile)GetProcAddress(m_hSysPrint, "_PrintTextAndPicByXmlFile");
	if (NULL == PrintTextAndPicByXmlFile)
	{
		AfxMessageBox("找不到_PrintTextAndPicByXmlFile函数");
		return FALSE;
	}
	CancelPrinterData = (pCancelPrinterData)GetProcAddress(m_hSysPrint, "_CancelPrinterData");
	if (NULL == CancelPrinterData)
	{
		AfxMessageBox("找不到_CancelPrinterData函数");
		return FALSE;
	}
	return TRUE;
}

