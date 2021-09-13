#pragma once

class CTrade
{
public:
	CTrade(void);
	~CTrade(void);

private:
	CString m_strLastErr;
private:
	CString AddMsgContent(CString strType,CString strBody="",CString strReserve="");	// ��ѯ����
public:
	//��ô��󱨸�
	CString GetLastErr() {return m_strLastErr;}
	// ǩ��
	BOOL CTrade::Login(const char* pUrl, int &iNextBitSpace, CString &strLastSignTime, CString &OnlineTime, CString &strOrgCode, CString &strDeviceType);
	// �·���Ŀ/��Ŀ��
	BOOL GetTemplate(const char* pUrl, CString &strJson,int itemtype);
	// �ϴ��·���Ľ�Ŀ״̬
	BOOL UpLoadProGrameStatus(const char* pUrl, int itemtype , int istatus);
	// �ϴ��豸״̬
	BOOL UpLoadDeviceStatus(const char* pUrl);
	//�ϴ�ͼƬ
	BOOL UpLoadPic(const char* pUrl, CString strBase64Pic ,CString strPicName);
	//�����ӿ�
	BOOL HeartBeat(const char* pUrl, int &iNextBitSpace, CString &strLastSignTime, CString &OnlineTime);

};

