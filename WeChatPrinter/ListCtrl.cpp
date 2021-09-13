// ListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "WeChatPrinter.h"
#include "ListCtrl.h"
#include "afxdialogex.h"


// ListCtrl 对话框

IMPLEMENT_DYNAMIC(ListCtrl, CDialogEx)

ListCtrl::ListCtrl(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ADMIN_DLG, pParent)
{

}

ListCtrl::~ListCtrl()
{
}

void ListCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ListCtrl, CDialogEx)
END_MESSAGE_MAP()


// ListCtrl 消息处理程序
