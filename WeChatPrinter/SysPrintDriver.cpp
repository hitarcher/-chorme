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
		AfxMessageBox("����SYSTEM_PRINTER.DLLʧ��");
		return FALSE;
	}
	InitSysPrinter = (pInitSysPrinter)GetProcAddress(m_hSysPrint, "_InitSysPrinter");
	if (NULL == InitSysPrinter)
	{
		AfxMessageBox("�Ҳ���_InitSysPrinter����");
		return FALSE;
	}
	PrintByPicture = (pPrintByPicture)GetProcAddress(m_hSysPrint, "_PrintByPicture");
	if (NULL == PrintByPicture)
	{
		AfxMessageBox("�Ҳ���_PrintByPicture����");
		return FALSE;
	}
	PrintByPicture2 = (pPrintByPicture2)GetProcAddress(m_hSysPrint, "_PrintByPicture2");
	if (NULL == PrintByPicture2)
	{
		AfxMessageBox("�Ҳ���_PrintByPicture2����");
		return FALSE;
	}
	GetPrinterStatus = (pGetPrinterStatus)GetProcAddress(m_hSysPrint, "_GetPrinterStatus");
	if (NULL == GetPrinterStatus)
	{
		AfxMessageBox("�Ҳ���_GetPrinterStatus����");
		return FALSE;
	}
	PrintTextAndPicByXmlFile = (pPrintTextAndPicByXmlFile)GetProcAddress(m_hSysPrint, "_PrintTextAndPicByXmlFile");
	if (NULL == PrintTextAndPicByXmlFile)
	{
		AfxMessageBox("�Ҳ���_PrintTextAndPicByXmlFile����");
		return FALSE;
	}
	CancelPrinterData = (pCancelPrinterData)GetProcAddress(m_hSysPrint, "_CancelPrinterData");
	if (NULL == CancelPrinterData)
	{
		AfxMessageBox("�Ҳ���_CancelPrinterData����");
		return FALSE;
	}
	return TRUE;
}

