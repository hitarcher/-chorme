
// WeChatPrinter.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CWeChatPrinterApp:
// �йش����ʵ�֣������ WeChatPrinter.cpp
//

class CWeChatPrinterApp : public CWinApp
{
public:
	CWeChatPrinterApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CWeChatPrinterApp theApp;