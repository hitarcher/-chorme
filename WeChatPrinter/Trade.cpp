#include "stdafx.h"
#include "Trade.h"
#include "HttpClient.h"
#include <string.h>
#include <json.hpp>
//工具函数头文件
#include "ToolsFunction.h"
#include "Config.h"

bool IsJsonData(std::string strData);//判断是否是json文件，是否符合规范
using namespace std;
using json = nlohmann::json;

extern CConfig g_Config;


//存储调用日志的函数名
CString g_strFunName = "";	
//平台报文私钥
CString g_str3DesPrivateKey = "6C4E60E55552386C759569836DC0F83869836DC0F838C0F7";

CTrade::CTrade(void)
{
	m_strLastErr = "";
}

CTrade::~CTrade(void)
{
}

CString CTrade::AddMsgContent(CString strType,CString strBody,CString strReserve)
{
	//新版平台，使用标准模板生成json数据
	CString strContent = "";
	json Head;
	json Body;
	json Root;
	Head["version"] = "1.0";
	Head["tradecode"] = strType.GetBuffer();
	Head["organcode"] = g_Config.m_strOrgCode.GetBuffer();
	Head["devicecode"] = g_Config.m_strDeviceCode.GetBuffer();
	Head["workdate"] = GetCurTime(DAY_LONG).c_str();
	Head["senddate"] = GetCurTime(DAY_LONG).c_str();
	Head["sendtime"] = GetCurTime(TIME_LONG).c_str();
	CString serialnumber = GetCurTime(DATE_NORMAL).c_str() + g_Config.m_strOrgCode + g_Config.m_strDeviceCode;
// 	if (serialnumber.GetLength() > 36)
// 	{
// 		LOG2(LOGTYPE_DEBUG, LOG_NAME_TRADE, "AddMsgContent", "serialnumber 流水号过长[%d]", serialnumber.GetLength());
// 		return "";
// 	}
// 	int iPownum = 36 - serialnumber.GetLength();
// 	if (iPownum)//将流水号补全到32位
// 	{
// 		srand((unsigned)time(NULL));
// 		CString strTemp;
// 		while (iPownum--)
// 		{
// 			int inum = rand() % 10;
// 			strTemp.Format("%d", inum);
// 			serialnumber += strTemp;
// 		}
// 	}
	Head["serialnumber"] = serialnumber.GetBuffer();
	Head["reserve"] = strReserve.GetBuffer();
	Head["token"] = "0";
	//组装head头
	Root["head"] = Head;

	g_Config.m_strOrgCode.ReleaseBuffer();
	g_Config.m_strDeviceCode.ReleaseBuffer();
	serialnumber.ReleaseBuffer();
	strReserve.ReleaseBuffer();
	//组装body
	CString body = "";
	body.Format("{%s}", strBody);
	Root["body"] = json::parse(body.GetBuffer(0));
	body.ReleaseBuffer();
	std::string out = Root.dump(4);
	CString str_json = out.c_str();
	str_json.Replace("\\", "");
	str_json.Replace(" ", "");
	str_json.Replace("\n", "");//去掉所有的空格，回车
	strContent.Format("%08d%s", str_json.GetLength(), str_json);
	if (strType.CompareNoCase("2011000009") != 0)//上传图片转base64,内容巨多，屏蔽了
	{
		if (strType.CompareNoCase("2011000008") != 0)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_TRADE, g_strFunName, "请求报文为\n%s", strContent);
		}
		else
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_OFFLINE, g_strFunName, "请求报文为\n%s", strContent);
		}
	}

#ifdef CLOSESCRECT
	return strContent;
#else
	CString strOut = Encode_3Des(g_str3DesPrivateKey, strContent);
	return strOut;
#endif
}
//1 签到
BOOL CTrade::Login(const char* pUrl, int &iNextBitSpace, CString &strLastSignTime, CString &OnlineTime,CString &strOrgCode ,CString &strDeviceType)
{
	CHttpClient httpClient;
	int iRet = 0;//单节目要data，多节目要data和datalist，干脆传整个json
	m_strLastErr = "";
	g_strFunName = "Login";
	httpClient.addParam("jsonparam", AddMsgContent("2011000001"));//addParam 里去掉jsonparam= 和&
	CString strGBKRsp = httpClient.doPost(pUrl);
	
	if (strGBKRsp.CompareNoCase("RequestTimedOut") == 0)
	{
		m_strLastErr = "请求超时";
		return FALSE;
	}
	if (strGBKRsp.CompareNoCase("") == 0)
	{
		m_strLastErr = "返回数据为空";
		return FALSE;
	}

#ifndef CLOSESCRECT
	CString strOut = Decode_3Des(g_str3DesPrivateKey, strGBKRsp);
	strGBKRsp = strOut;
#endif
	ConvertUtf8ToGBK(strGBKRsp);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_TRADE, "Login", "返回报文为\n%s", strGBKRsp);
	string strGBK = strGBKRsp;
	if (FALSE == IsJsonData(strGBK))
	{
		m_strLastErr = "非json字符串";
		return FALSE;
	}
	json jMsg = json::parse(strGBK);
	if (jMsg.find("body") == jMsg.end())
	{
		m_strLastErr = "获取字段失败[body]";
		return FALSE;
	}
	json jBody = jMsg["body"];//进入同一级才能find
	if (jBody.find("result") == jBody.end() || jBody.find("msg") == jBody.end() || jBody.find("data") == jBody.end())
	{
		m_strLastErr = "获取字段失败[result][msg][body]";
		return FALSE;
	}
	if (jBody["result"].get<std::string>() != "0")
	{
		m_strLastErr.Format("error.result =%s,msg = %s", jBody["result"].get<std::string>().c_str(), jBody["msg"].get<std::string>().c_str());
		return FALSE;
	}
	////测试功能，心跳，暂未上线
// 	if (jBody.find("nextbitspace") == jBody.end() || jBody.find("lastsigntime") == jBody.end() || jBody.find("deviceOnlinetime") == jBody.end())
// 	{
// 		m_strLastErr = "获取字段失败[nextbitspace][OnlineTime][onlinetime]";		
// 		return FALSE;
// 	}
// 	iNextBitSpace = atoi(jBody["nextbitspace"].get<std::string>().c_str());
// 	strLastSignTime = jBody["lastsigntime"].get<std::string>().c_str();
// 	OnlineTime = jBody["deviceOnlinetime"].get<std::string>().c_str();

	json jData = jBody["data"];
	if (jData.find("organnbr") == jData.end() || jData.find("typenbr") == jData.end())
	{
		m_strLastErr = "获取字段失败[organnbr][typenbr]";
		return FALSE;
	}
	
	strOrgCode = jData["organnbr"].get<std::string>().c_str();
	strDeviceType = jData["typenbr"].get<std::string>().c_str();
	return TRUE;
}

//2 下发节目/节目单
BOOL CTrade::GetTemplate(const char* pUrl, CString &strJson ,int itemtype)
{
	CHttpClient httpClient;
	int iRet = 0;//单节目要data，多节目要data和datalist，干脆传整个json
	m_strLastErr = "";
	g_strFunName = "GetTemplate";

	CString strBody = "";
	strBody.Format("\"itemtype\":\"%d\"", itemtype);
	httpClient.addParam("jsonparam", AddMsgContent("2011000002", strBody));//addParam 里去掉jsonparam= 和&
	CString strGBKRsp = httpClient.doPost(pUrl); 
	if (strGBKRsp.CompareNoCase("RequestTimedOut") == 0)
	{
		m_strLastErr = "请求超时";
		return FALSE;
	}
	if (strGBKRsp.CompareNoCase("") == 0)
	{
		m_strLastErr = "返回数据为空";
		return FALSE;
	}
#ifndef CLOSESCRECT
	CString strOut = Decode_3Des(g_str3DesPrivateKey, strGBKRsp);
	strGBKRsp = strOut;
#endif
	ConvertUtf8ToGBK(strGBKRsp);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_TRADE, "GetTemplate", "返回报文为\n%s",strGBKRsp);
	string strGBK = strGBKRsp;

	if (FALSE == IsJsonData(strGBK))
	{
		m_strLastErr = "非json字符串,"+ strGBKRsp;
		return FALSE;
	}
	json jMsg = json::parse(strGBK);
	if (jMsg.find("body") == jMsg.end())
	{
		m_strLastErr = "获取字段失败[body]";
		return FALSE;
	}
	json jBody = jMsg["body"];//进入同一级才能find
	if (jBody.find("result") == jBody.end() || jBody.find("msg") == jBody.end() || jBody.find("data") == jBody.end())
	{
		m_strLastErr = "获取字段失败[result][msg][body]";
		return FALSE;
	}
	if (jBody["result"].get<std::string>() != "0")
	{
		m_strLastErr.Format("error.result =%s,msg = %s", jBody["result"].get<std::string>().c_str(), jBody["msg"].get<std::string>().c_str());
		return FALSE;
	}

	//2 找到单节目，但是找不到节目表单
	json jData = jBody["data"];
	if (jData.find("itemtemplatejson") != jData.end() && jBody.find("datalist") == jBody.end())
	{
		json jTemp = jData["itemtemplatejson"];
		if (FALSE == jTemp.is_object())
		{
			m_strLastErr = "解析json字符串失败[itemtemplatejson信息]";
			return FALSE;
		}
	}
	//3 找到节目表单
	else if(jBody.find("datalist") != jBody.end())
	{
		json jTemp = jBody["datalist"];
		if (FALSE == jTemp.is_array())
		{
			m_strLastErr = "解析json字符串失败[datalist信息]";
			return FALSE;
		}
	}
	std::string strTemp = jMsg.dump(4);//全部保存
	strJson = strTemp.c_str();
	return TRUE;
}

//3 上传下发后的节目状态
BOOL CTrade::UpLoadProGrameStatus(const char* pUrl, int itemtype, int istatus)
{
	CHttpClient httpClient;
	int iRet = 0;//单节目要data，多节目要data和datalist，干脆传整个json
	m_strLastErr = "";
	g_strFunName = "UpLoadProGrameStatus";

	CString strBody = "";
	strBody.Format("\"itemtype\":\"%d\",\"status\":\"%d\"", itemtype,istatus);
	httpClient.addParam("jsonparam", AddMsgContent("2011000003", strBody));//addParam 里去掉jsonparam= 和&
	CString strGBKRsp = httpClient.doPost(pUrl);

	if (strGBKRsp.CompareNoCase("RequestTimedOut") == 0)
	{
		m_strLastErr = "请求超时";
		return FALSE;
	}
	if (strGBKRsp.CompareNoCase("") == 0)
	{
		m_strLastErr = "返回数据为空";
		return FALSE;
	}
#ifndef CLOSESCRECT
	CString strOut = Decode_3Des(g_str3DesPrivateKey, strGBKRsp);
	strGBKRsp = strOut;
#endif
	ConvertUtf8ToGBK(strGBKRsp);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_TRADE, "UpLoadProGrameStatus", "返回报文为\n%s", strGBKRsp);
	string strGBK = strGBKRsp;

	if (FALSE == IsJsonData(strGBK))
	{
		m_strLastErr = "非json字符串";
		return FALSE;
	}
	json jMsg = json::parse(strGBK);
	if (jMsg.find("body") == jMsg.end())
	{
		m_strLastErr = "获取字段失败[body]";
		return FALSE;
	}
	json jBody = jMsg["body"];//进入同一级才能find
	if (jBody.find("result") == jBody.end() || jBody.find("msg") == jBody.end() || jBody.find("data") == jBody.end())
	{
		m_strLastErr = "获取字段失败[result],[msg],[body]";
		return FALSE;
	}
	if ("0" != jBody["result"])
	{
		m_strLastErr.Format("error.result =%s,msg = %s", jBody["result"].get<std::string>().c_str(), jBody["msg"].get<std::string>().c_str());
		return FALSE;
	}

	return TRUE;
}

//8 上传设备状态
BOOL CTrade::UpLoadDeviceStatus(const char* pUrl)
{
	CHttpClient httpClient;
	CString strBody = "";
	m_strLastErr = "";
	g_strFunName = "UpLoadDeviceStatus";

	httpClient.addParam("jsonparam", AddMsgContent("2011000008", strBody));
	CString strGBKRsp = httpClient.doPost(pUrl);

	if (strGBKRsp.CompareNoCase("RequestTimedOut") == 0)
	{
		m_strLastErr = "请求超时";
		return FALSE;
	}
	if (strGBKRsp.CompareNoCase("") == 0)
	{
		m_strLastErr = "返回数据为空";
		return FALSE;
	}
#ifndef CLOSESCRECT
	CString strOut = Decode_3Des(g_str3DesPrivateKey, strGBKRsp);
	strGBKRsp = strOut;
#endif
	string strGBK = strGBKRsp;

	if (FALSE == IsJsonData(strGBK))
	{
		m_strLastErr = "非json字符串";
		return FALSE;
	}
	json jMsg = json::parse(strGBK);
	if (jMsg.find("body") == jMsg.end())
	{
		m_strLastErr = "获取字段失败[body]";
		return FALSE;
	}
	json jBody = jMsg["body"];//进入同一级才能find
	if (jBody.find("result") == jBody.end() || jBody.find("msg") == jBody.end() || jBody.find("data") == jBody.end())
	{
		m_strLastErr = "获取字段失败[result],[msg],[body]";
		return FALSE;
	}
	if ("0" != jBody["result"])
	{
		m_strLastErr.Format("error.result =%s,msg = %s", jBody["result"].get<std::string>().c_str(), jBody["msg"].get<std::string>().c_str());
		return FALSE;
	}
	return TRUE;
}

//9 上传图片
BOOL CTrade::UpLoadPic(const char* pUrl, CString strBase64Pic, CString strPicName)
{
	CHttpClient httpClient;
	CString strBody = "";
	m_strLastErr = "";
	g_strFunName = "UpLoadPic";

	strBody.Format("\"imageBase\":\"%s\",\"filename\":\"%s\"", strBase64Pic, strPicName);
	httpClient.addParam("jsonparam", AddMsgContent("2011000009", strBody));
	CString strGBKRsp = httpClient.doPost(pUrl);

	if (strGBKRsp.CompareNoCase("RequestTimedOut") == 0)
	{
		m_strLastErr = "请求超时";
		return FALSE;
	}
	if (strGBKRsp.CompareNoCase("") == 0)
	{
		m_strLastErr = "返回数据为空";
		return FALSE;
	}
#ifndef CLOSESCRECT
	CString strOut = Decode_3Des(g_str3DesPrivateKey, strGBKRsp);
	strGBKRsp = strOut;
#endif	
	ConvertUtf8ToGBK(strGBKRsp);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_TRADE, "UpLoadPic", "返回报文为\n%s", strGBKRsp);
	string strGBK = strGBKRsp;

	if (FALSE == IsJsonData(strGBK))
	{
		m_strLastErr = "非json字符串";
		return FALSE;
	}
	json jMsg = json::parse(strGBK);
	if (jMsg.find("body") == jMsg.end())
	{
		m_strLastErr = "获取字段失败[body]";
		return FALSE;
	}
	json jBody = jMsg["body"];//进入同一级才能find
	if (jBody.find("result") == jBody.end() || jBody.find("msg") == jBody.end() || jBody.find("data") == jBody.end())
	{
		m_strLastErr = "获取字段失败[result],[msg],[body]";
		return FALSE;
	}
	if ("0" != jBody["result"])
	{
		m_strLastErr.Format("error.result =%s,msg = %s", jBody["result"].get<std::string>().c_str(), jBody["msg"].get<std::string>().c_str());
		return FALSE;
	}
	return TRUE;
}

BOOL CTrade::HeartBeat(const char* pUrl, int &iNextBitSpace, CString &strLastSignTime, CString &OnlineTime)

{
	CHttpClient httpClient;
	CString strBody = "";
	int iRet = 0;
	m_strLastErr = "";
	g_strFunName = "HeartBeat";
	strBody.Format("\"onlinetime\":\"%s\",\"lastsigntime\":\"%s\"", OnlineTime, strLastSignTime);

	httpClient.addParam("jsonparam", AddMsgContent("2011000010", strBody));
	CString strGBKRsp = httpClient.doPost(pUrl);

	if (strGBKRsp.CompareNoCase("RequestTimedOut") == 0)
	{
		m_strLastErr = "请求超时";
		return FALSE;
	}
	if (strGBKRsp.CompareNoCase("") == 0)
	{
		m_strLastErr = "返回数据为空";
		return FALSE;
	}

#ifndef CLOSESCRECT
	CString strOut = Decode_3Des(g_str3DesPrivateKey, strGBKRsp);
	strGBKRsp = strOut;
#endif	ConvertUtf8ToGBK(strGBKRsp);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_TRADE, "HeartBeat", "返回报文为\n%s", strGBKRsp);
	string strGBK = strGBKRsp;

	if (FALSE == IsJsonData(strGBK))
	{
		m_strLastErr = "非json字符串";
		return FALSE;
	}
	json jMsg = json::parse(strGBK);
	if (jMsg.find("body") == jMsg.end())
	{
		m_strLastErr = "获取字段失败[body]";
		return FALSE;
	}
	json jBody = jMsg["body"];//进入同一级才能find
	if (jBody.find("result") == jBody.end() || jBody.find("msg") == jBody.end() || jBody.find("data") == jBody.end())
	{
		m_strLastErr = "获取字段失败[result][msg][body]";
		return FALSE;
	}
	if (jBody["result"].get<std::string>() != "0")
	{
		m_strLastErr.Format("error.result =%s,msg = %s", jBody["result"].get<std::string>().c_str(), jBody["msg"].get<std::string>().c_str());
		return FALSE;
	}
	json jdata = jBody["data"];
	if (jdata.find("nextbitspace") == jdata.end() || jdata.find("lastsigntime") == jdata.end() || jdata.find("deviceOnlinetime") == jdata.end())
	{
		m_strLastErr = "获取字段失败[nextbitspace][OnlineTime][onlinetime]";
		return FALSE;
	}
	iNextBitSpace = atoi(jdata["nextbitspace"].get<std::string>().c_str());
	strLastSignTime = jdata["lastsigntime"].get<std::string>().c_str();
	OnlineTime = jdata["deviceOnlinetime"].get<std::string>().c_str();

	return TRUE;
}
