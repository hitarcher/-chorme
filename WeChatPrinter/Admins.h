#pragma once

#include "ImageDlg.h"
// CAdmins 对话框

class CAdmins : public CImageDlg
{
	DECLARE_DYNAMIC(CAdmins)

public:
	CAdmins(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAdmins();

	// 对话框数据
	enum { IDD = IDD_DIALOG_ADMINS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnCancel();
	virtual BOOL DestroyWindow();
};
