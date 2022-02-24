// NetworkTips.cpp : 实现文件
//

#include "stdafx.h"
#include "Heads.h"
#include "WeChatPrinter.h"
#include "NetworkTips.h"
#include "afxdialogex.h"
#include "myos.h"
#include "myhttp.h"

// CNetworkTips 对话框

IMPLEMENT_DYNAMIC(CNetworkTips, CDialogEx)

CNetworkTips::CNetworkTips(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_NETWORK_TIPS, pParent)
{

}

CNetworkTips::~CNetworkTips()
{
}

void CNetworkTips::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_NETWORK_TIPS, m_ctlNetworkTips);
}


BEGIN_MESSAGE_MAP(CNetworkTips, CDialogEx)
END_MESSAGE_MAP()


// CNetworkTips 消息处理程序


BOOL CNetworkTips::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// 不在任务栏显示
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	// 设置透明度
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_LAYERED);
	SetLayered(GetSafeHwnd(), RGB(0, 0, 0), 0xFF, LWA_COLORKEY | LWA_ALPHA);
	// 设置坐标
	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);
	int imagex = 60;
	int imagey = 60;
	switch (m_nPosition)
	{
	case 1: // 左上
		SetWindowPos(NULL, 10, 10, imagex, imagey, SWP_SHOWWINDOW);
		break;
	case 2:	// 右上
		SetWindowPos(NULL, x - imagex - 10, 10, imagex, imagey, SWP_SHOWWINDOW);
		break;
	case 3:	// 左下
		SetWindowPos(NULL, 10, y - imagey - 10, imagex, imagey, SWP_SHOWWINDOW);
		break;
	case 4:	// 右下
		SetWindowPos(NULL, x - imagex - 10, y - imagey - 10, imagex, imagey, SWP_SHOWWINDOW);
		break;
	default:	// 右上
		SetWindowPos(NULL, x - imagex - 10, 10, imagex, imagey, SWP_SHOWWINDOW);
		break;
	}
	m_ctlNetworkTips.SetWindowPos(NULL, 0, 0, 60, 60, SWP_NOZORDER);

	// 默认显示离线
	tips_offline();

	// 定时检查
	m_timers_access_http.setTimeout([&]() {
		access_http();
	}, 1000);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CNetworkTips::access_http()
{
	httplib::Client cli(m_strHost.c_str());
	auto res = cli.Get("/");

	switch (res.error())
	{
	case Success:
		tips_online();
		break;
	default:
		tips_offline();
		break;
	}

	m_timers_access_http.setTimeout([&]() {
		access_http();
	}, 5000);
}

void CNetworkTips::tips_online()
{
	CImage image_online;
	image_online.Load(get_fullpath("data/GUI/image/online.png").c_str());
	if (image_online.GetBPP() == 32) //确认该图像包含Alpha通道
	{
		int i;
		int j;
		for (i = 0; i < image_online.GetWidth(); i++)
		{
			for (j = 0; j < image_online.GetHeight(); j++)
			{
				byte *pByte = (byte *)image_online.GetPixelAddress(i, j);
				pByte[0] = pByte[0] * pByte[3] / 255;
				pByte[1] = pByte[1] * pByte[3] / 255;
				pByte[2] = pByte[2] * pByte[3] / 255;
			}
		}
	}
	HBITMAP hBmp = image_online.Detach();
	m_ctlNetworkTips.SetBitmap(hBmp);

	image_online.Destroy();
	DeleteObject(hBmp);

	::SetWindowPos(GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

void CNetworkTips::tips_offline()
{
	CImage image_offline;
	image_offline.Load(get_fullpath("data/GUI/image/offline.png").c_str());
	if (image_offline.GetBPP() == 32) //确认该图像包含Alpha通道
	{
		int i;
		int j;
		for (i = 0; i < image_offline.GetWidth(); i++)
		{
			for (j = 0; j < image_offline.GetHeight(); j++)
			{
				byte *pByte = (byte *)image_offline.GetPixelAddress(i, j);
				pByte[0] = pByte[0] * pByte[3] / 255;
				pByte[1] = pByte[1] * pByte[3] / 255;
				pByte[2] = pByte[2] * pByte[3] / 255;
			}
		}
	}
	HBITMAP hBmp = image_offline.Detach();
	m_ctlNetworkTips.SetBitmap(hBmp);

	image_offline.Destroy();
	DeleteObject(hBmp);

	::SetWindowPos(GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

