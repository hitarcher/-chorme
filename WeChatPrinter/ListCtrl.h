#pragma once


// ListCtrl 对话框

class ListCtrl : public CDialogEx
{
	DECLARE_DYNAMIC(ListCtrl)

public:
	ListCtrl(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~ListCtrl();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADMIN_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
