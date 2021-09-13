// Admins.cpp : 实现文件
//

#include "stdafx.h"
#include "WeChatPrinter.h"
#include "Admins.h"
#include "afxdialogex.h"
#include "commfun.h"


#define COMMAND_ADMIN_CLOSE 1000
#define COMMAND_MAINWINDOW_OUT 1001
#define COMMAND_SHUTDOWN_TERMINAL 1002
#define COMMAND_RESTART_TERMINAL 1003
#define COMMAND_OPENLOG 1004

// CAdmins 对话框

//IMPLEMENT_DYNAMIC(CAdmins, CImageDlg)

CAdmins::CAdmins(CWnd* pParent /*=NULL*/)
	: CImageDlg(CAdmins::IDD, pParent)
{

}

CAdmins::~CAdmins()
{
}

void CAdmins::DoDataExchange(CDataExchange* pDX)
{
	CImageDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAdmins, CImageDlg)
END_MESSAGE_MAP()


// CAdmins 消息处理程序
BOOL CAdmins::OnInitDialog()
{
	ShowCursor(TRUE);
	CImageDlg::OnInitDialog();
	SetWindowPos(NULL, 0, 0, 613, 337, SWP_NOMOVE);
	AddImage(0, 0, "DATA\\RES\\PUBLIC\\gif\\background.gif");
	AddImage(170, 250, "DATA\\RES\\PUBLIC\\gif\\loginout.gif", COMMAND_MAINWINDOW_OUT, "DATA\\RES\\PUBLIC\\gif\\loginout_d.gif");
	AddImage(573, 10, "DATA\\RES\\PUBLIC\\gif\\close.gif", COMMAND_ADMIN_CLOSE, "DATA\\RES\\PUBLIC\\gif\\close_d.gif");
	AddImage(360, 85, "DATA\\RES\\PUBLIC\\gif\\shutdown.gif", COMMAND_SHUTDOWN_TERMINAL, "DATA\\RES\\PUBLIC\\gif\\shutdown_d.gif");
	AddImage(110, 85, "DATA\\RES\\PUBLIC\\gif\\restart.gif", COMMAND_RESTART_TERMINAL, "DATA\\RES\\PUBLIC\\gif\\restart_d.gif");
	AddImage(15, 213, "DATA\\RES\\PUBLIC\\gif\\openlog.gif", COMMAND_OPENLOG, "DATA\\RES\\PUBLIC\\gif\\openlog_d.gif");

	return TRUE;
}

CString getFullPath(CString strTempPath)
{
	CString strEngineeringPath;
	TCHAR szPath[MAX_PATH] = { 0 };
	if (GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		TCHAR *pChr = _tcsrchr(szPath, _T('\\'));
		pChr++;
		*pChr = _T('\0');
		//(_tcsrchr(szPath, _T('\\')))[1] = 0;//三句可以由这一句替代
	}
	strEngineeringPath.Format(_T("%s"), szPath);

	return  strEngineeringPath + strTempPath ;
}

BOOL CAdmins::OnCommand(WPARAM wParam, LPARAM lParam)
{
	CString strPath = getFullPath("");
	switch (wParam)
	{
	case COMMAND_ADMIN_CLOSE:
		EndDialog(0);
		break;
	case COMMAND_MAINWINDOW_OUT:
		EndDialog(1);
		break;
	case COMMAND_SHUTDOWN_TERMINAL:
		ShutDown();
		break;
	case COMMAND_RESTART_TERMINAL:
		Reboot();
		break;
	case COMMAND_OPENLOG:
		//system("start \"\" \"D:\\1 pic\""); //路径为D:\test\hh\xxx
		system("start \"\" \"" + strPath +"\"");
		break;
	default:
		break;
	}
	return CImageDlg::OnCommand(wParam, lParam);
}


void CAdmins::OnCancel()
{
	CImageDlg::OnCancel();
}


BOOL CAdmins::DestroyWindow()
{
	return CImageDlg::DestroyWindow();
}
