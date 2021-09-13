#include "stdafx.h"
#include "Config.h"
#include "CommonFun.h"
#define MAX 1024

CConfig::CConfig(void)
{
	m_strLastErr = "";
	m_strOrgCode = "";
	m_strDeviceType = "";
}


CConfig::~CConfig(void)
{

}

BOOL CConfig::LoadBaseCfg()
{
	m_strCfgPath = GetFullPath("Config\\Config.ini");
	if(CheckFileExist(m_strCfgPath) == FALSE)
	{
		m_strLastErr.Format("缺失系统文件[%s]",GetFullPath("Config\\Config.ini"));
		return FALSE;
	}
	GetPrivateProfileString("BASE", "Template", "FFFFFFFFFFFF", m_strRelatePath.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strRelatePath.ReleaseBuffer();
	GetPrivateProfileString("BASE", "TempalteJson", "FFFFFFFFFFFF", m_strTempalteJson.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strTempalteJson.ReleaseBuffer();
	GetPrivateProfileString("BASE", "TemporaryJson", "FFFFFFFFFFFF", m_strTemporaryJson.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strTemporaryJson.ReleaseBuffer();
	GetPrivateProfileString("BASE", "OldTemplateJson", "FFFFFFFFFFFF", m_strOldTemplate.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strOldTemplate.ReleaseBuffer();

// 	GetPrivateProfileString("BASE", "PrivateKey", "FFFFFFFFFFFF", m_str3DesPrivateKey.GetBuffer(MAX), MAX, m_strCfgPath);
// 	m_str3DesPrivateKey.ReleaseBuffer();
	m_bTopMost = GetPrivateProfileInt("BASE", "TopMost", 1, m_strCfgPath);

	// 获取网页的位置
	m_nPositionX = GetPrivateProfileInt("POSITION", "POSX", 0, m_strCfgPath);
	m_nPositionY = GetPrivateProfileInt("POSITION", "POSY", 0, m_strCfgPath);
	// 获取网页的大小
	m_nPageWide = GetPrivateProfileInt("POSITION", "PAGEWIDE", 1920, m_strCfgPath);
	m_nPageHigh = GetPrivateProfileInt("POSITION", "PAGEHIGH", 1080, m_strCfgPath);


	GetPrivateProfileString("BASE","HttpUrl","FFFFFFFFFFFF",m_strHttpUrl.GetBuffer(MAX),MAX,m_strCfgPath);
	m_strHttpUrl.ReleaseBuffer();
	GetPrivateProfileString("BASE","HttpDwonloadUrl","FFFFFFFFFFFF",m_strHttpDwonloadUrl.GetBuffer(MAX),MAX,m_strCfgPath);
	m_strHttpDwonloadUrl.ReleaseBuffer();

	//设备号，签到需要用的到
	GetPrivateProfileString("RMQ", "QueueName", "", m_strDeviceCode.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strDeviceCode.ReleaseBuffer();
	return TRUE;
}

BOOL CConfig::LoadRMQCfg()
{
	GetPrivateProfileString("RMQ", "IPAddress", "", m_strIPAddress.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strIPAddress.ReleaseBuffer();
	GetPrivateProfileString("RMQ", "Account", "", m_strAccount.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strAccount.ReleaseBuffer();
	GetPrivateProfileString("RMQ", "Password", "", m_strPassword.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strPassword.ReleaseBuffer();
	GetPrivateProfileString("RMQ", "OrgCode", "", m_strOrgCode.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strOrgCode.ReleaseBuffer();
	GetPrivateProfileString("RMQ", "ExchangeName", "", m_strExchangeName.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strExchangeName.ReleaseBuffer();
	GetPrivateProfileString("RMQ", "RouteKey", "", m_strRouteKeyIndex.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strRouteKeyIndex.ReleaseBuffer();
	GetPrivateProfileString("RMQ", "DeviceType", "", m_strDeviceType.GetBuffer(MAX), MAX, m_strCfgPath);
	m_strDeviceType.ReleaseBuffer();

	m_nPort = GetPrivateProfileInt("RMQ", "Port", 0, m_strCfgPath);
	m_nChannel = GetPrivateProfileInt("RMQ", "Channel", 0, m_strCfgPath);
	m_nModeName = GetPrivateProfileInt("RMQ", "ModeName", 0, m_strCfgPath);
	m_bAutoConfig = GetPrivateProfileInt("RMQ", "AutoConfig", 0, m_strCfgPath);

	vector<CString> vecIndex = SplitString(m_strRouteKeyIndex, "|");
	if (m_bAutoConfig)	//自动修改本地配置开关，通过本地配置获取。限定格式和个数
	{
		if (vecIndex.size() != 5)
		{
			m_strLastErr.Format("RouteKey不为5个");
			return FALSE;
		}
		CString strTemp("");
		strTemp = "ry.mq." + m_strOrgCode + "." + m_strDeviceType + "." + m_strDeviceCode;
		m_vecRouteKey.push_back(strTemp);
		WriteStringToCfgDev("RMQ", vecIndex[0], strTemp);

		strTemp = "ry.mq." + m_strOrgCode + "." + m_strDeviceType;
		m_vecRouteKey.push_back(strTemp);
		WriteStringToCfgDev("RMQ", vecIndex[1], strTemp);

		strTemp = "ry.mq." + m_strDeviceType;
		m_vecRouteKey.push_back(strTemp);
		WriteStringToCfgDev("RMQ", vecIndex[2], strTemp);

		strTemp = "ry." + m_strOrgCode;
		m_vecRouteKey.push_back(strTemp);
		WriteStringToCfgDev("RMQ", vecIndex[3], strTemp);

		strTemp = "*";
		m_vecRouteKey.push_back(strTemp);
		WriteStringToCfgDev("RMQ", vecIndex[4], strTemp);

	}
	else//直接读取
	{
		for (unsigned int i = 0; i < vecIndex.size(); i++)
		{
			CString strTemp("");
			GetPrivateProfileString("RMQ", vecIndex[i], "", strTemp.GetBuffer(MAX), MAX, m_strCfgPath);
			strTemp.ReleaseBuffer();
			m_vecRouteKey.push_back(strTemp);
		}
	}
	return TRUE;
}

CString CConfig::GetStringFromCfgDev(LPCTSTR lpAppName, LPCTSTR lpKeyName)
{
	CString strtmp;
	GetPrivateProfileString(lpAppName,lpKeyName,"FFFFFFFFFFFF",strtmp.GetBuffer(MAX),MAX,m_strCfgPath);
	strtmp.ReleaseBuffer();
	return strtmp;
}
UINT CConfig::GetIntgFromCfgDev(CString strAppName, CString strKeyName)
{
	int iTemp = GetPrivateProfileInt(strAppName, strKeyName, 0, m_strCfgPath);
	return iTemp;
}

BOOL CConfig::WriteStringToCfgDev(LPCTSTR lpAppName, LPCTSTR lpKeyName, CString strValue)
{
	return WritePrivateProfileString(lpAppName, lpKeyName, strValue, m_strCfgPath);
}