
// WeChatPrinterDlg.h : 头文件
#pragma once
#include <mshtml.h>
#include <shellapi.h>
#include "Trade.h"
#include "CommTbl.h"
#include "SqliteFun.h"
#include "CommonFun.h"
#include "Shell.h"
#include "ImageDlg.h"
#include "LOG2.H"
#include <sqlite3.h>
#include "CommTbl.h"
#include "json.hpp"
using json = nlohmann::json;
#include "Admins.h"
#include "Config.h"
#include <stdio.h>

#ifdef DEBUG                            
#pragma comment(lib, "CommUtilsd.lib")  
#else                                   
#pragma comment(lib, "CommUtils.lib")   
#endif         

//截屏头文件
#include "MakePNG.h"

//解压缩头文件
#pragma comment( lib, "coinreceiver.lib" )
#include "tinyzipinterface.h"
#pragma comment( lib, "tinyzip.lib" )

//工具函数头文件
#include "ToolsFunction.h"
//内存函数相关头文件
#include <psapi.h>

//CEF3
#include "mycef.h"
#include "simple_app.h"
#include "simple_handler.h"
/************************************************************************/
/*                              常    量                                */
/************************************************************************/
enum PROGAMETYPE
{
	SINGLEPROGRAM = 1,			//节目
	MULITIPLEPROGRAM = 2		//节目单
};
enum PROGAMMODE
{
	NEW_JSON = 1,				//默认节目，新节目,在线节目
	TEMPORARY_JSON = 2			//紧急插播(只有单节目)
};
enum SINGLEPRO
{
	PLAY_ALLDAY = 1,			//单，全天
	PLAY_DIVIDE = 2				//单，分时段
};
enum MULITIPEPRO
{
	PLAY_REPEATDAY = 1,			//多，日重复，一个表单里，模式全都只会为1
	PLAY_WEEKDAY = 2,			//多，周重复，一个表单里，模式全都只会为2
	PLAY_CUSTOMIZE = 3			//多，自定义，一个表单里，模式全都只会为3
};


// CWeChatPrinterDlg 对话框
class CWeChatPrinterDlg : public CImageDlg/*,public IDispatch*/
{
// 构造
public:
	CWeChatPrinterDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_WECHATPRINTER_DIALOG };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bExit;													
	CString m_strLastErr;											
	CTrade g_toolTrade;
	int m_nMode;													//单节目还是多节目

	CString m_strHtmlPath;											//H5页面路径

	//签到线程
	HANDLE m_hReSign;
	HANDLE m_hReSignEvent;

	//RabbitMQ调用
	HANDLE m_hRMQ;
	HANDLE m_hRMQEvent;

	//上传设备状态
	HANDLE m_hDviceStatus;
	HANDLE m_hDviceStatusEvent;

	//判断节目选择
	HANDLE m_hChooseProgram;
	HANDLE m_hChooseProgramEvent;

	//心跳
	HANDLE m_hHeartBeat;
	HANDLE m_hHeartBeatEvent;
	
	json jrsp;														//正在使用中的节目 
	json jTemplate;													//平常加载的节目	tempalte.json
	json jTemproary;												//紧急插播节目		temproary.json
	json jOldrsp;													//取消节目时，用来保存原节目
	json jDefault;													//默认节目			default.json
	json jOverdue;													//上一次下发的节目	templateold.json
	json jForIE;													//当前传给h5使用的内容，即字段itemtemplatejson

	json GetCurrentJson() { return jrsp; }
	void ChangeCurrentJson(json jTemp) { jrsp = jTemp; }

	static CWeChatPrinterDlg* m_pThis;

	int m_iNextBitSpace ;										    //下次心跳时间
	CString m_strLastSignTime;									    //最新签到时间
	CString m_OnlineTime;										    //在线时长
	CString m_strOrgCode;											//签到后机构号  
	CString m_strDeviceType;									    //签到后设备类型
																    
	CefRefPtr<SimpleApp> m_cef_app;								    
	void cef_init();											    
	void cef_close();											    
	void cef_load_url(IN std::string _utf_url, IN int delay=0);
	void cef_exec_js(IN std::string _utf_js, IN int delay = 0); 
	// 加载模板
	void LoadTemplate();

	//签到线程
	static DWORD WINAPI	ReSignThreadProc(LPVOID pParam);
	DWORD WINAPI ReSignThreadContent(LPVOID pParam);

	// RMQ线程
	static DWORD WINAPI	RMQThreadProc(LPVOID pParam);
	DWORD WINAPI RMQThreadContent(LPVOID pParam);

	//上送设备状态
	static DWORD WINAPI	DeviceStatusThreadProc(LPVOID pParam);
	DWORD WINAPI DeviceStatusThreadContent(LPVOID pParam);

	//判断节目选择
	static DWORD WINAPI	ChooseProgramThreadProc(LPVOID pParam);
	DWORD WINAPI ChooseProgramThreadContent(LPVOID pParam);

	//心跳
	static DWORD WINAPI HeartBeatThreadProc(LPVOID pParam);
	DWORD WINAPI HeartBeatThreadContent(LPVOID pParam);

	// RMQ下发操作
	BOOL RMQ_DealCustomMsg(CString strMsg);	
	//加载Pub和SUB两个动态库
	BOOL LoadRMQPubAndRMQSUBDLL();	
	//SUB连接服务器并设置接收回调函数
	BOOL RMQ_SUBConnect();			

	//删除旧的素材
	void DelateResource();

	//检测json里是否有资源与旧json重复。2，3，4，5 json与1比较，1json里有的素材是2，3，4，5没有的话就删除这些素材
	BOOL CheckOldResource(vector<json>vecJson);

	//解析，下载，修改资源路径，重新加载json，判断是否是节目还是节目单
	BOOL ParseNewTemplateOfH5(CString strJson, BOOL bIsSingleProgram  , /*BOOL bIsOffline =FALSE,*/ BOOL bParseOnly = FALSE);

	//选择加载的节目，包含离线压缩包解压，选择紧急插播节目或者是普通节目
	BOOL ChooseJson();

	//判断并加载紧急插播节目
	BOOL LoadTemporayJson();

	//判断有无离线节目压缩包
	BOOL LoadOfflinePacket();

	//检查自动更新程序是否启动
	BOOL CheckUpdate();

	// 屏幕截图,返回图片的路径
	CString ScreenShot(void);
	
	//检测本地是否还有没有下载完的文件
	BOOL CheckIncompleted();

	//修改节目的截止日期和截止时间，修改节目单的截止日期但不改时间。
	//截至时间修改，根据当前下发的节目是哪一种来修改程序中的时间，以及本地的json,
	//如果当前是普通节目，修改时间，修改本地时间，重新加载，如果是紧急插播，只修改紧急插播的
	BOOL ExtendDeadline(CString strProgammeID, CString strNewDeadline, vector<CString> vecTime);

	BOOL StartProcess3(CString strpath);

	//启动离线节目，将当前的template 改成templateonline。templaiteOffline 改成 template
// 	void SwitchJsonToOffline();
// 	
// 	//启动在线节目，将当前的template 改成templaiteOffline。templateonline 改成 template
// 	void SwitchJsonToOnline();
	void sleepFunction1(int Time);//伪动态的sleep，用于长时间睡眠却无法主动退出的线程

public:
	// 窗口大小
	CRect	m_rc;
	//退出时的密码
	CString m_strAdminEnter;

	void OnAdminEnter(CPoint point);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()

};
/************************************************************************\
* 检测json里所有的资源路径，排除掉重复的，保存到容器中					*
* 入参：jALL ：平台节目json ；vecTemp ：空容器，存放资源路径			*
* 返回值：true 成功，False 失败											*
\************************************************************************/
BOOL FindContent(json jALL, vector<CString> &vecTemp);		

//入参template/1.json，出参json内容
json LoadjsonFile(CString strJsonName);

/************************************************************************\
* 下载文件																*
* 入参：strUrl：文件url ；strLocalPath 文件的局部地址，如“www/static/1.jpg”
* 返回值：无															*
\************************************************************************/
void DownLoadFile(CString strUrl, CString strLocalPath);

/************************************************************************\
* 将json里传入的时间分配到容器中										*
* 入参：08:00-09:00|11:00-12:00
* 返回值：80000，90000，110000等的int类型容器							*
\************************************************************************/
vector<int> getHours(CString strHours);	

/************************************************************************\
* 将json里传入的时间分配到容器中										*
* 入参：strDays ：2019-02-25|2019-02-31|08:00-09:00|11:00-12:00；
		vecDay 存放日期字符串；vecHM 存放时间字符串						*
* 返回值： 20190225,80000,120000										*
\************************************************************************/
void getDays(CString strDays, vector<int> &vecDay, vector<int> &vecHM);	


/************************************************************************\
* 判断节目日期是否过期													*
* 入参：截至日期，格式：如2021-01-11									*
* 返回值： false为没过期，true为过期									*
\************************************************************************/
BOOL  CheckTimeLimit(CString strEndDate);			

/************************************************************************\
* 获取节目剩余播出时间													*
* 入参：vecHM 该节目所有的时间段，由getHours获取；						*
*		iCurrentTime 当前时间，HHMMSS的格式；							*
*		iPlayMode 播放格式，参考MULITIPEPRO								*
* 返回值： 剩余播出时间,单位秒											*
\************************************************************************/
int GetWaitTime(vector<int>vecHM, int iCurrentTime, int iPlayMode = 1);

/************************************************************************\
* 获取节目剩余关闭时间													*
* 入参：vecHM 该节目所有的时间段，由getHours获取；						*
*		iCurrentTime 当前时间，HHMMSS的格式；							*
* 返回值： 剩余关闭时间,单位秒											*
\************************************************************************/
int GetCloseTime(vector<int>vecHM, int iCurrentTime);				

BOOL HTTP_Download2(CString strURL, CString strFilePath, CString strUsername, CString strPassword, CString strProxyList);

void GetSystemMemoryInfo();

//重启程序
void RobotProgamme();
/************************************************************************/
/*                            全局变量申明                              */
/************************************************************************/
extern CConfig	g_Config;