#pragma once

#include "ImageDlg.h"
// CAdmins �Ի���

class CAdmins : public CImageDlg
{
	DECLARE_DYNAMIC(CAdmins)

public:
	CAdmins(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAdmins();

	// �Ի�������
	enum { IDD = IDD_DIALOG_ADMINS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnCancel();
	virtual BOOL DestroyWindow();
};
