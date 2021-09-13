#pragma once

class CTrade
{
public:
	CTrade(void);
	~CTrade(void);

private:
	CString m_strLastErr;
private:
	CString AddMsgContent(CString strType,CString strBody="",CString strReserve="");	// 查询类型
public:
	//获得错误报告
	CString GetLastErr() {return m_strLastErr;}
	// 签到
	BOOL CTrade::Login(const char* pUrl, int &iNextBitSpace, CString &strLastSignTime, CString &OnlineTime, CString &strOrgCode, CString &strDeviceType);
	// 下发节目/节目单
	BOOL GetTemplate(const char* pUrl, CString &strJson,int itemtype);
	// 上传下发后的节目状态
	BOOL UpLoadProGrameStatus(const char* pUrl, int itemtype , int istatus);
	// 上传设备状态
	BOOL UpLoadDeviceStatus(const char* pUrl);
	//上传图片
	BOOL UpLoadPic(const char* pUrl, CString strBase64Pic ,CString strPicName);
	//心跳接口
	BOOL HeartBeat(const char* pUrl, int &iNextBitSpace, CString &strLastSignTime, CString &OnlineTime);

};

