#pragma once
#include "afxwin.h"
#include "mytimer.h"

// CNetworkTips 对话框

class CNetworkTips : public CDialogEx
{
	DECLARE_DYNAMIC(CNetworkTips)

public:
	CNetworkTips(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNetworkTips();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NETWORK_TIPS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_ctlNetworkTips;
	virtual BOOL OnInitDialog();

private:
	void tips_online();
	void tips_offline();

	void access_http();
	easyTimer m_timers_access_http;

	// url
	std::string m_strHost;
	// tips的显示位置，默认右上
	int m_nPosition = 2;

public:
	// 设置需要检测的url
	void set_checking_httpurl(IN std::string host) {
		m_strHost = host;
	};

	// 设置显示位置
	//  1 左上
	//  2 右上
	//  3 左下
	//  4 右下
	void set_position(int position) {
		m_nPosition = position;
	};
};
