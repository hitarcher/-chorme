// CWebBrowser2.cpp : �� Microsoft Visual C++ ������ ActiveX �ؼ���װ��Ķ���


#include "stdafx.h"
#include "CWebBrowser2.h"

/////////////////////////////////////////////////////////////////////////////
// CWebBrowser2

IMPLEMENT_DYNCREATE(CWebBrowser2, CWnd)

// CWebBrowser2 ����

// CWebBrowser2 ����


BOOL CWebBrowser2::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	switch (pMsg->message) {
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		return TRUE;
	default:
		break;
	}
	return CWnd::PreTranslateMessage(pMsg);
}
