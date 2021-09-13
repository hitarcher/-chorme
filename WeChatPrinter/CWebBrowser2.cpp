// CWebBrowser2.cpp : 由 Microsoft Visual C++ 创建的 ActiveX 控件包装类的定义


#include "stdafx.h"
#include "CWebBrowser2.h"

/////////////////////////////////////////////////////////////////////////////
// CWebBrowser2

IMPLEMENT_DYNCREATE(CWebBrowser2, CWnd)

// CWebBrowser2 属性

// CWebBrowser2 操作


BOOL CWebBrowser2::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	switch (pMsg->message) {
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		return TRUE;
	default:
		break;
	}
	return CWnd::PreTranslateMessage(pMsg);
}
