#pragma once


// ListCtrl �Ի���

class ListCtrl : public CDialogEx
{
	DECLARE_DYNAMIC(ListCtrl)

public:
	ListCtrl(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~ListCtrl();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADMIN_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
