#pragma once

#include <vector>
#include <iostream>
using namespace std;
class CConfig
{
public:
	CConfig(void);
	~CConfig(void);

private:
	CString m_strLastErr;
	CString m_strCfgPath;

public:

	CString m_strHttpUrl;
	CString m_strHttpDwonloadUrl;

	CString m_strIPAddress;
	CString m_strAccount;
	CString m_strPassword;
	CString m_strDeviceCode;
	CString m_strOrgCode;
	CString m_strExchangeName;
	CString m_strRouteKeyIndex;
	vector<CString> m_vecRouteKey;
	CString m_strDeviceType;
	int m_nPort;
	int	m_nModeName;
	int	m_nChannel;
	BOOL m_bAutoConfig;

	CString m_strRelatePath;
	CString m_strTemporaryJson;
	CString m_strTempalteJson;
	CString m_strOldTemplate;

	//CString m_str3DesPrivateKey;
	BOOL m_bTopMost;

	int m_nPositionX;					//ҳ�������λ��
	int m_nPositionY;					//ҳ��������λ��
	int m_nPageWide;					//ҳ����
	int m_nPageHigh;					//ҳ��߶�
public:
	CString GetLastErr() {return m_strLastErr;}
	BOOL LoadBaseCfg();
	BOOL LoadRMQCfg();
	CString GetStringFromCfgDev(LPCTSTR lpAppName, LPCTSTR lpKeyName);
	BOOL WriteStringToCfgDev(LPCTSTR lpAppName, LPCTSTR lpKeyName, CString strValue);
	UINT GetIntgFromCfgDev(CString strAppName, CString strKeyName);
};

