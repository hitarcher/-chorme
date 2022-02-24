#pragma once
#include "afxwin.h"
#include "mytimer.h"

// CNetworkTips �Ի���

class CNetworkTips : public CDialogEx
{
	DECLARE_DYNAMIC(CNetworkTips)

public:
	CNetworkTips(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CNetworkTips();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NETWORK_TIPS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
	// tips����ʾλ�ã�Ĭ������
	int m_nPosition = 2;

public:
	// ������Ҫ����url
	void set_checking_httpurl(IN std::string host) {
		m_strHost = host;
	};

	// ������ʾλ��
	//  1 ����
	//  2 ����
	//  3 ����
	//  4 ����
	void set_position(int position) {
		m_nPosition = position;
	};
};
