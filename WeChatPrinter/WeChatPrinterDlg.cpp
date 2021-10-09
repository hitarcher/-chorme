#include "stdafx.h"
#include "WeChatPrinter.h"
#include "WeChatPrinterDlg.h"
#include "afxdialogex.h"

#include "mystring.h"
#include "myos.h"
#include "mylog.h"
#include "simple_handler.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif            

#define TIMER_RESIGN		1001									//签到
#define TIMER_LOADPAGE		1002									//加载H5方法
#define TIMER_RECV_MSG		1003									//接收RMQ消息
#define TIMER_LOADTEMPORARY 1004									//加载紧急插播函数
#define TIMER_CHOOSEPROGAME 1005									//选择节目
#define TIMER_CHECKINCOMPELEDFILE 1006								//检测未完成的素材
#define TIMER_CHECKMEMORY	1007									//检测内存
#define BTN_ADMIN_LOGOUT	2000									

#define templatezip "template.json"
#define defaultJson "template/default.json"
#define AutoUpdateMsg "自动更新\\Msg.ini"
#define AutoUpdateEXE "自动更新\\AutoUpdate.exe"

CWeChatPrinterDlg*g_CWeChatPrinterDlg = NULL;
static CString g_strRecvMsg = "";									//来自Rmq的消息
BOOL g_bIsTemproaryOn = FALSE;										//当前是临时节目的播出时间。
BOOL g_bDownload = TRUE;											//判断是否需要下载
CString g_strRecordCurrentPrograme = "";							//记录选择节目线程的节目
CString g_strItemid = "";											//手动切换的节目ID
BOOL bUnderSwitchMode = FALSE;										//处于切换模式，此状态优先级高，高于插播，低于重新下发。
static int nCheckTimes = 0;											//检测如果两次还没更新掉，就不更新了。
typedef void(__stdcall *_CallBack_Recv)(char* bMsg);
typedef int(_stdcall *lpRMQ_CALLBACK)(_CallBack_Recv);
typedef int(_stdcall *lpRMQ_SUB)(const char *, int, const char *, const char *, const char *,
	const char *, const char *, int, int);
lpRMQ_SUB _RMQ_SUB;
lpRMQ_CALLBACK _RMQ_CALLBACK;

CConfig	g_Config;

HHOOK hMouseHook;
LRESULT CALLBACK OnMouseEvent(int nCode, WPARAM wParam, LPARAM lParam);

static void CALLBACK _Recv(char* bMsg)
{
	g_strRecvMsg = bMsg;
	g_CWeChatPrinterDlg->SetTimer(TIMER_RECV_MSG, 10, NULL);
}


CWeChatPrinterDlg::CWeChatPrinterDlg(CWnd* pParent /*=NULL*/)
	: CImageDlg(CWeChatPrinterDlg::IDD, pParent)
{
	::CoInitialize(NULL);
	m_strLastErr = "";
	m_bExit = FALSE;
	g_CWeChatPrinterDlg = this;
	m_nMode = 0;
	m_strHtmlPath = "";

	jrsp = "";
	jTemplate = "";
	jTemproary = "";
	jOldrsp = "";
	jDefault = "";
	jOverdue = "";
	jForIE = "";

	m_iNextBitSpace = 1;
	m_strLastSignTime = "";
	m_OnlineTime = "";

	m_rc = CRect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	m_strAdminEnter = "";

	m_hReSign = NULL;
	m_hReSignEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hRMQ = NULL;
	m_hRMQEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hChooseProgram = NULL;
	m_hChooseProgramEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hDviceStatus = NULL;
	m_hDviceStatusEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

// 	m_hHeartBeat = NULL;
// 	m_hHeartBeatEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWeChatPrinterDlg::DoDataExchange(CDataExchange* pDX)
{
	CImageDlg::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_EXPLORER_MAIN, m_netBrower);
}

BEGIN_MESSAGE_MAP(CWeChatPrinterDlg, CImageDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CWeChatPrinterDlg 消息处理程序
BOOL CWeChatPrinterDlg::OnInitDialog()
{
	CImageDlg::OnInitDialog();
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	/************************************************************************/
	/*                        程 序 入 口                                   */
	/************************************************************************/
	//=================================
	// 初始化
	//=================================
	SET_LOGTYPE((LOG_TYPE)(LOGTYPE_DEBUG | LOGTYPE_ERROR | LOGTYPE_SPECIAL));
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "==============START==============", "");
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*///删除超过30天的日志
	LOG_CLEAR(30);
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/// 设置全局钩子，用来键入管理界面，以免被其他控件遮挡
#ifdef STARTHOOK
	hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, OnMouseEvent, theApp.m_hInstance, 0);
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/// 用于更新 自动更新程序
	if (CheckFileExist(GetFullPath("update.bat")))
	{
		StartProcess3(GetFullPath("update.bat")); 
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "OnInitDialog", "执行更新程序，替换文件");
		goto EXIT;
	}
#ifdef CHECKUPDATE
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*///是否启动自动更新检测
	if (FALSE == CheckUpdate())
	{
		goto EXIT;
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*///加载基本配置
	if (FALSE == g_Config.LoadBaseCfg())
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "OnInitDialog", "[g_Config.LoadBaseCfg()][%s]", g_Config.GetLastErr());
		goto EXIT;
	}
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*///加载动态库
	if (FALSE == LoadRMQPubAndRMQSUBDLL())
	{
		goto EXIT;
	}
	DWORD dwThreadId = 0;
	m_hReSign = CreateThread(NULL, 0, ReSignThreadProc, this, 0, &dwThreadId);
	m_hRMQ = CreateThread(NULL, 0, RMQThreadProc, this, 0, &dwThreadId);
	m_hDviceStatus = CreateThread(NULL, 0, DeviceStatusThreadProc, this, 0, &dwThreadId);
	m_hChooseProgram = CreateThread(NULL, 0, ChooseProgramThreadProc, this, 0, &dwThreadId);
	m_hHeartBeat = CreateThread(NULL, 0, HeartBeatThreadProc, this, 0, &dwThreadId);

	// 启动代理
	ProxyStart_http();
	//=================================
	// 窗口属性设置
	//=================================
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*///是否隐藏鼠标
#ifdef DEBUG
	ShowCursor(TRUE);
#else
	ShowCursor(FALSE);
#endif 
	SetWindowText(INFO_PUBLISH_SCREEN_NAME);
	if (g_Config.m_bTopMost)
	{
		::SetWindowPos(GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//最前端
	}
	m_strHtmlPath = "file:///" + GetFullPath(g_Config.m_strRelatePath + "index.html");
	m_strHtmlPath.Replace("\\", "/");
	ConvertGBKToUtf8(m_strHtmlPath);
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*///cef3初始化
	cef_init();
	
	//选择加载的节目json，包含离线压缩包解压，选择紧急插播节目或者是普通节目
	if (FALSE == ChooseJson())
	{
		goto EXIT;
	}
	SetTimer(TIMER_CHOOSEPROGAME,10, NULL);
	SetTimer(TIMER_RESIGN, 10, NULL);
	SetTimer(TIMER_CHECKINCOMPELEDFILE, 3000, NULL);
	SetTimer(TIMER_CHECKMEMORY, 5000, NULL);
	//隔30天删除一次没有归属的素材
	DelateResource();
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "OnInitDialog", "界面初始化成功");
	return TRUE;
EXIT:
	EndDialog(FALSE);
	return FALSE;
}

void CWeChatPrinterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CImageDlg::OnPaint();
	}
}

HCURSOR CWeChatPrinterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//销毁
void CWeChatPrinterDlg::OnDestroy()
{
	m_bExit = TRUE;

	SetEvent(m_hReSignEvent);
	WaitForSingleObject(m_hReSign, 3000);
	CloseHandle(m_hReSign);
	m_hReSign = NULL;

	SetEvent(m_hDviceStatusEvent);
	WaitForSingleObject(m_hDviceStatus, 3000);
	CloseHandle(m_hDviceStatus);
	m_hDviceStatus = NULL;

	SetEvent(m_hRMQEvent);
	WaitForSingleObject(m_hRMQ, 3000);
	CloseHandle(m_hRMQ);
	m_hRMQ = NULL;

	SetEvent(m_hChooseProgramEvent);
	WaitForSingleObject(m_hChooseProgram, 3000);
	CloseHandle(m_hChooseProgram);
	m_hChooseProgram = NULL;

	//cef_close();
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "==============DESTROY==============", "");
	CImageDlg::OnDestroy();
}

void CWeChatPrinterDlg::ProxyStart_http()
{
	// 创建http代理
	server_httpproxy.Post("/cpp", [&](const Request &req, Response &res, const ContentReader &content_reader) {
		std::string body;
		content_reader([&](const char *data, size_t data_length) {
			body.append(data, data_length);
			return true;
		});
	
	    // 处理
	    std::string replay;
		ProxyConsume_http(req.path, body, replay);//处理H5的信息
		//LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "replay  :%s", replay.c_str());
		//res.set_content(replay.c_str(), "json");//返回给H5
	});
	std::thread thread_proxy_http([&]() {
		bool bRet = server_httpproxy.listen("0.0.0.0", 8866);//监听H5的消息
	});
	thread_proxy_http.detach();
}

void CWeChatPrinterDlg::ProxyConsume_http(IN std::string path, IN std::string request, OUT std::string & replay)
{
	json jrequeset = json::parse(easytoGBK(request).c_str());
	
	if (jrequeset["code"] == "zip")
	{
		//保存压缩后的文件
		//	replay = "ziping";
		std::string strBase64 = jrequeset["file"].get<std::string>();
		std::string name = jrequeset["name"];
		Base64decodePic(strBase64, GetFullPath(g_Config.m_strRelatePath + name.c_str()));
	}
	else if (jrequeset["code"] == "zipok")
	{
		//压缩ok   -->  加载loadtem  force
		//	replay = "okokok";
		SetTimer(TIMER_LOADPAGE, 500, NULL);
	}
}

BOOL CWeChatPrinterDlg::PostZipList()
{
	bool b1 = is_64bitsystem();
	float b2 = GetMemory();

	if (b1 || b2 < 4)
	{
		vector<CString>vecTemp;
		vector<CString>vecImg;
		FindContent(jTemplate, vecTemp);
		for (unsigned int i = 0; i < vecTemp.size(); i++)
		{
			if ((vecTemp[i].Find("jpg")>=0) || (vecTemp[i].Find("png") >= 0))
			{
				vecImg.push_back(vecTemp[i]);
			}
		}
		json jTemp;
		jTemp["img"] = vecImg;

		CString strContent = jTemp.dump().c_str();
		ConvertGBKToUtf8(strContent);
		CString strInitInterface;
		strInitInterface.Format(_T("PostZipList('%s');"), strContent);
		cef_exec_js(strInitInterface.GetBuffer(0));
		return FALSE;
	}
	return TRUE;
}

void CWeChatPrinterDlg::LoadTemplate()
{
	std::string strTemp = jForIE.dump();
	CString strContent = strTemp.c_str();
	ConvertGBKToUtf8(strContent);
	CString strInitInterface;
	strInitInterface.Format(_T("loadPage('%s');"), strContent);
	cef_exec_js(strInitInterface.GetBuffer(0));

#ifdef DETAIlEDLOG
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "LoadTemplate", "加载模板成功,加载模板为\r\n%s\r\n", strContent);
#else
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "LoadTemplate", "加载模板成功");
#endif

}

void CWeChatPrinterDlg::cef_init()
{
	// Enable High-DPI support on Windows 7 or newer.
	CefEnableHighDPISupport();

	void* sandbox_info = nullptr;

#if defined(CEF_USE_SANDBOX)
	CefScopedSandboxInfo scoped_sandbox;
	sandbox_info = scoped_sandbox.sandbox_info();
#endif

	CefMainArgs main_args(theApp.m_hInstance);
	int exit_code = CefExecuteProcess(main_args, nullptr, sandbox_info);
	if (exit_code >= 0) {
		return;
	}
	CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
	command_line->InitFromString(::GetCommandLineW());
	CefSettings settings;
	if (command_line->HasSwitch("enable-chrome-runtime")) {
		settings.chrome_runtime = true;
	}

	settings.remote_debugging_port = 33220;
	settings.multi_threaded_message_loop = true;
	CefString(&settings.cache_path) = easytoUTF(get_fullpath("cef_catch"));
	CefString(&settings.log_file) = easytoUTF(get_fullpath("cef_catch//cef_log.log"));
	CefString(&settings.framework_dir_path) = easytoUTF(get_fullpath(""));
	CefString(&settings.resources_dir_path) = easytoUTF(get_fullpath("cef_Resources"));
	CefString(&settings.locales_dir_path) = easytoUTF(get_fullpath("cef_Resources\\locales"));

#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif
	//********************************************************************************
	//实在想不起来，自适应会有什么问题，不过大部分都按照配的分辨率来的话，这个先不放开了。
	SetWindowPos(NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_DRAWFRAME);

	//********************************************************************************

	CRect rect; GetWindowRect(&rect);
	m_cef_app =
		new SimpleApp(
			m_strHtmlPath.GetBuffer(0)
			/*"www.baidu.com"*/
			, m_hWnd
			, rect
		);
	CefInitialize(main_args, settings, m_cef_app.get(), sandbox_info);

}

void CWeChatPrinterDlg::cef_close()
{
	/*edit by mingl : 单进程模式下，CefShutdown会出现阻塞；但是多进程下运行正常；考虑到单进程下直接退出不用管其他进程，所以也可以不用管*/
	//CefShutdown();
	/*end by mingl*/
}

void CWeChatPrinterDlg::cef_load_url(IN std::string _utf_url, IN int delay)
{
	if (delay) Sleep(delay);
	if (m_cef_app == NULL) return cef_load_url(_utf_url, 500);

	CefRefPtr<SimpleHandler> default_handler = (SimpleHandler*)(void*)m_cef_app->GetDefaultClient();
	if (default_handler == NULL) return cef_load_url(_utf_url, 500);

	CefRefPtr<CefBrowser> default_browser = default_handler->GetBrowser();
	if (default_browser == NULL) return cef_load_url(_utf_url, 500);

	CefRefPtr<CefFrame> default_frame = default_browser->GetMainFrame();
	if (default_frame == NULL) return cef_load_url(_utf_url, 500);

	default_frame->LoadURL(_utf_url.c_str());
}

void CWeChatPrinterDlg::cef_exec_js(IN std::string _utf_js, IN int delay)
{
	if (delay) Sleep(delay);
	if (m_cef_app == NULL) return cef_exec_js(_utf_js, 500);

	CefRefPtr<SimpleHandler> default_handler = (SimpleHandler*)(void*)m_cef_app->GetDefaultClient();
	if (default_handler == NULL) return cef_exec_js(_utf_js, 500);

	CefRefPtr<CefBrowser> default_browser = default_handler->GetBrowser();
	if (default_browser == NULL) return cef_exec_js(_utf_js, 500);

	CefRefPtr<CefFrame> default_frame = default_browser->GetMainFrame();
	if (default_frame == NULL) return cef_exec_js(_utf_js, 500);

	if (default_frame->GetURL().empty() == true) return cef_exec_js(_utf_js, 500);
	default_frame->ExecuteJavaScript(CefString(_utf_js.c_str()), default_frame->GetURL(), 0);
}

/****************************		节目相关函数		*****************************************/

//入参template/1.json，出参json内容,如果文件不存在，可能虎生成一个空的文件
json LoadjsonFile(CString strJsonName)
{
	CFile file;
	CFileStatus status;
	CString strConfigFile = GetFullPath(g_Config.m_strRelatePath + strJsonName);
	json jTemp = "";
	if (FALSE == CheckFileExist(strConfigFile))
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "LoadjsonFile", "[LoadjsonFile][%s]文件不存在", strConfigFile);
		return jTemp;
	}
	//1 先判断文件是否存在
	if (file.Open(strConfigFile, CFile::modeReadWrite) == FALSE &&
		file.Open(strConfigFile, CFile::modeCreate | CFile::modeReadWrite) == FALSE)
	{
		CString strLastErr = "";
		strLastErr.Format("文件打开失败，可能权限不够！（%s）", strConfigFile);
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadjsonFile", "[LoadjsonFile][%s]", strLastErr);
		return jTemp;
	}
	file.GetStatus(status);
	//2 再判断文件是否为空
	if (status.m_size == 0)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadjsonFile", "[LoadjsonFile][文件为空]");
		return jTemp;
	}
	else
	{
		char* pTemplate = (char*)calloc((int)status.m_size + 1, sizeof(char));
		file.Read(pTemplate, (int)status.m_size);
		CString strJson = pTemplate;
		ConvertUtf8ToGBK(strJson);
		jTemp = json::parse(strJson.GetBuffer(0));
		strJson.ReleaseBuffer();
		free(pTemplate);
		pTemplate = NULL;
	}
	file.Close();
	return jTemp;
}

//判断有无并加载紧急插播节目
BOOL CWeChatPrinterDlg::LoadTemporayJson()
{
	CString strTargetFile = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
	CString strErrMsg = "";
	if (CheckFileExist(strTargetFile))
	{
		jTemproary = LoadjsonFile(g_Config.m_strTemporaryJson);
		if (jTemproary == "")
		{
			strErrMsg = "jTemproary 内容为空";
			goto EXIT;
		}
		if (jTemproary.find("body") == jTemproary.end())
		{
			strErrMsg = "节目格式不正确";
			goto EXIT;
		}
		json jBody = jTemproary["body"];
		if (jBody.find("datalist") != jBody.end())
		{
			strErrMsg = "节目格式不正确，非单节目";
			goto EXIT;
		}

		CString strMode = jTemproary["body"]["data"]["releasemode"].get<std::string>().c_str();
		if (atoi(strMode) != TEMPORARY_JSON)
		{
			strErrMsg = "发布模式不符合";
			goto EXIT;
		}
		CString strEndDate = jTemproary["body"]["data"]["itemenddate"].get<std::string>().c_str();
		if (CheckTimeLimit(strEndDate))
		{
			strErrMsg = "插播节目超时，删除临时节目" + strEndDate;
			goto EXIT;
		}
		return TRUE;
	}
	return FALSE;
EXIT:
	DeleteFile(strTargetFile);
	LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadTemporayJson", strErrMsg);
	return FALSE;
}

//判断有无离线节目压缩包
BOOL CWeChatPrinterDlg::LoadOfflinePacket()
{
	//1 找到指定路径下的指定名称ZIP包
	CString strZipName1 = g_Config.m_strOrgCode + "_ITEM.zip";
	CString strZipName2 = g_Config.m_strOrgCode + "_ITEMLIST.zip";
	CString strTargetFile = "";
	int nProgameType = 0;
	if (CheckFileExist(GetFullPath(g_Config.m_strRelatePath + strZipName1)))
	{
		strTargetFile = GetFullPath(g_Config.m_strRelatePath + strZipName1);
		nProgameType = SINGLEPROGRAM;
	}
	else if (CheckFileExist(GetFullPath(g_Config.m_strRelatePath + strZipName2)))
	{
		strTargetFile = GetFullPath(g_Config.m_strRelatePath + strZipName2);
		nProgameType = MULITIPLEPROGRAM;
	}
	else
	{
		return FALSE;
	}

	CString strResourcePath = "OfflineResource\\";
	CString strResourceFullPath = GetFullPath(g_Config.m_strRelatePath + strResourcePath);
	CString strNewPath = GetFullPath(g_Config.m_strRelatePath + "www\\static\\");
	// 开始解压
	RN_RESULT nResult = _unzip(strTargetFile, strResourceFullPath);

	if (nResult)// 解压失败
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadOfflineJson", "解压离线节目包失败！（错误代码：%d）", nResult);
		return FALSE;
	}
	else// 解压成功
	{
		json jOffline = LoadjsonFile(strResourcePath + templatezip);
		if (jOffline == "")
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadOfflineJson", templatezip, "内容为空");
			return FALSE;
		}
		json jHead = jOffline["head"];
		if (jHead.find("organcode") == jHead.end() || jHead.find("devicecode") == jHead.end())
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadOfflineJson", "获取[organcode]或[devicecode]字段失败");
			return FALSE;
		}
		CString strOrgancode = jHead["organcode"].get<std::string>().c_str();
		CString strDevicecode = jHead["devicecode"].get<std::string>().c_str();
		if (strOrgancode.CompareNoCase(g_Config.m_strOrgCode) || strDevicecode.Find(g_Config.m_strDeviceCode) < 0)
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadOfflineJson", "字段值不符 Organcode = [%s],Devicecode = [%s] \n当前 Organcode = [%s],Devicecode = [%s]"
				, strOrgancode, strDevicecode, g_Config.m_strOrgCode, g_Config.m_strDeviceCode);
			return FALSE;
		}
		CString strOFFLineJson = jOffline.dump(4).c_str();
		if (strOFFLineJson.Find("upload") >= 0)
		{
			g_bDownload = FALSE;
			ParseNewTemplateOfH5(strOFFLineJson, nProgameType == SINGLEPROGRAM ? TRUE : FALSE);
			g_bDownload = TRUE;

			//DeleteFile(strResourceFullPath + "template.json");
			//移动压缩路径所有文件到 NewPath
			RemoveFileToOtherPath(strResourceFullPath, strNewPath, "*");
			// 删除更新文件
			DeleteFile(strTargetFile);
			//删文件夹和内容
			RemoveDir(strResourceFullPath);
		}
	}
	return TRUE;
}

//选择加载的节目，包含离线压缩包解压，选择紧急插播节目或者是普通节目
BOOL CWeChatPrinterDlg::ChooseJson()
{
	jDefault = LoadjsonFile(defaultJson);
	if ("" == jDefault)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ChooseJson", "jDefault 内容为空");
		return FALSE;
	}
	jTemplate = LoadjsonFile(g_Config.m_strTempalteJson);
	if ("" == jTemplate)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ChooseJson", "jTemplate 内容为空");
		//没有template.json 就播放default.json
		ChangeCurrentJson(jDefault);
	}
	else
	{
		ChangeCurrentJson(jTemplate);
	}
	jOverdue = LoadjsonFile(g_Config.m_strOldTemplate);

	//判断有无离线节目压缩包
	LoadOfflinePacket();
	//判断有无紧急插播节目且是否合规，没有就加载原节目
	if (LoadTemporayJson())
	{
		ChangeCurrentJson(jTemproary);
	}

	return TRUE;
}

//解析节目
BOOL CWeChatPrinterDlg::ParseNewTemplateOfH5(CString strJson, BOOL bIsSingleProgram,/* BOOL bIsOffline,*/ BOOL bParseOnly)
{
	// 解析json
	json jALL = json::parse(strJson.GetBuffer(0));
	strJson.ReleaseBuffer();
	json j = jALL["body"]["data"];
	//发布模式，有默认节目和紧急插播两种
	int iReleseMode = NEW_JSON;
	if (bIsSingleProgram)
	{
		iReleseMode = atoi(j["releasemode"].get<std::string>().c_str());
	}
	else
	{
		/*From 平台，如果传的多节目里没有时间(timeseg)字段，就不要进行下载，节省资源
		单节目，如果是全天，没时间也可以。多节目没有全天模式。只有日重复有这种情况
		2020.6.15 carouselmode 也是没有的*/
		j = jALL["body"]["datalist"];
		json jTemp;
		for (json::iterator it = j.begin(); it != j.end(); ++it)
		{
			json jTemp2 = *it;
			if (jTemp2.find("carouselmode") != jTemp2.end())
			{
				jTemp.push_back(jTemp2);
			}
		}
		j = jTemp;
	}

	for (unsigned int i = 0; bIsSingleProgram || i < j.size(); i++)
	{
		json jPage;
		//判断是否是单模板
		if (bIsSingleProgram)
			jPage = j["itemtemplatejson"]["page"];
		else
			jPage = j[i]["itemtemplatejson"]["page"];
		if (jPage.is_array() == FALSE)
		{
			m_strLastErr = "解析template json[page]格式不正确";
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ParseNewTemplateOfH5", "[err][%s]", m_strLastErr);
			return FALSE;
		}
		json jArray_edit = json::array();//创建一个空数组
		json jo = json::object();		 //创建一个空对象
		for (json::iterator it = jPage.begin(); it != jPage.end(); jArray_edit.push_back(jo), ++it)
		{
			jo = *it;
			json jArray_edit2 = json::array();//创建一个空数组
			json jo2 = json::object();		  //创建一个空对象
			json jElements = jo["elements"];
			for (json::iterator it2 = jElements.begin(); it2 != jElements.end(); jArray_edit2.push_back(jo2), ++it2)
			{
				// array转object
				jo2 = *it2;
				if (jo2.is_object() == false) continue;
				std::string strFilePath = "www/static/";

				//-----------------//
				//下载group里的内容//
				//-----------------//
				json jArray_edit3 = json::array();//创建一个空数组
				json jo3 = json::object();		  //创建一个空对象
				json jGroup = jo2["group"];
				for (json::iterator it3 = jGroup.begin(); it3 != jGroup.end(); jArray_edit3.push_back(jo3), ++it3)
				{
					jo3 = *it3;
					//-------------------//
					//下载bgImg里的内容//
					//-------------------//
					std::string strUrl = jo3["bgImg"].get<std::string>();
					if ("" != strUrl)
					{
						std::string strFilePath = "template/";
						// http下载并更改路径
						CString strLocalPath = "";
						strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
						if (bParseOnly == TRUE) continue;
						DownLoadFile(strUrl.c_str(), strLocalPath);
						jo3["bgImg"] = strLocalPath;
					}
					//排除掉非 img、vidoe、carousel 里的content资源下载，例如btn的content可能是一串中文
					std::string strType = jo3["type"].get<std::string>();
					if (strType == "img" || strType == "video" || strType == "carousel")
					{
						json::iterator posGroupContent = jo3.find("content");
						json jGroupUrls(posGroupContent.value());
						std::string strUrl = *jGroupUrls.begin();
						if ("" == strUrl)  continue;
						// http下载并更改路径
						CString strLocalPath = "";
						strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
						jGroupUrls = strLocalPath.GetBuffer(0);
						strLocalPath.ReleaseBuffer();
						if (bParseOnly == TRUE) continue;
						DownLoadFile(strUrl.c_str(), strLocalPath);
						jo3["content"] = jGroupUrls;
					}
				}
				jo2["group"] = jArray_edit3;

				//--------------------//
				//下载resource里的内容//
				//--------------------//
				json jArray_edit4 = json::array();//创建一个空数组
				json jo4 = json::object();		  //创建一个空对象
				json jResource = jo2["resource"];
				for (json::iterator it3 = jResource.begin(); it3 != jResource.end(); jArray_edit4.push_back(jo4), ++it3)
				{
					jo4 = *it3;
					json::iterator posGroupContent = jo4.find("content");
					json jResource(posGroupContent.value());
					std::string strUrl = *jResource.begin();
					if ("" == strUrl)  continue;
					// http下载并更改路径
					CString strLocalPath = "";
					strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
					jResource = strLocalPath.GetBuffer(0);
					strLocalPath.ReleaseBuffer();
					if (bParseOnly == TRUE) continue;
					DownLoadFile(strUrl.c_str(), strLocalPath);
					jo4["content"] = jResource;
				}
				jo2["resource"] = jArray_edit4;

				//-------------------//
				//下载content里的内容//
				//-------------------//
				//排除掉非 img、vidoe、carousel 里的content资源下载，例如btn的content可能是一串中文
				std::string strType = jo2["type"].get<std::string>();
				if (strType == "img" || strType == "video" || strType == "carousel")
				{
					json::iterator posContent = jo2.find("content");
					if (posContent == jo2.end() || posContent.value().is_string() == false) continue;
					json jUrls(posContent.value());
					std::string strUrl = *jUrls.begin();
					if ("" == strUrl)  continue;
					// http下载并更改路径
					CString strLocalPath = "";
					strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));//取 最后一个斜杠后面的内容
					jUrls = strLocalPath.GetBuffer(0);
					strLocalPath.ReleaseBuffer();
					if (bParseOnly == TRUE) continue;
					DownLoadFile(strUrl.c_str(), strLocalPath);
					jo2["content"] = jUrls;
				}

				//-------------------//
				//下载bgImg里的内容//
				//-------------------//
				std::string strUrl = jo2["bgImg"].get<std::string>();
				if ("" != strUrl)
				{
					std::string strFilePath = "template/";
					// http下载并更改路径
					CString strLocalPath = "";
					strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
					if (bParseOnly == TRUE) continue;
					DownLoadFile(strUrl.c_str(), strLocalPath);
					jo2["bgImg"] = strLocalPath;
				}
			}
			//-----------------//
			//下载bgImg里的内容//
			//-----------------//
			std::string strUrl = jo["bgImg"].get<std::string>();
			if ("" != strUrl)
			{
				std::string strFilePath = "template/";
				// http下载并更改路径
				CString strLocalPath = "";
				strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
				if (bParseOnly == TRUE) continue;
				DownLoadFile(strUrl.c_str(), strLocalPath);
				jo["bgImg"] = strLocalPath;
			}
			jo["elements"] = jArray_edit2;
		}

		jPage = jArray_edit;
		if (bIsSingleProgram)
		{
			j["itemtemplatejson"]["page"] = jPage;
			break;
		}
		else
		{
			j[i]["itemtemplatejson"]["page"] = jPage;
		}
		if (bParseOnly == TRUE) return TRUE;
	}
	//替换jALL里的内容
	if (bIsSingleProgram)
		jALL["body"]["data"] = j;
	else
		jALL["body"]["datalist"] = j;

	//保存到的json名称，template或者temporary，默认或者紧急插播
	std::string strPath = g_Config.m_strTempalteJson;

	//紧急插播，保存的文件名也不一样，和原文件不冲突。但是别通过 CheckOldResource 函数改原文件名称
	if (TEMPORARY_JSON == iReleseMode)
	{
		CString strOldTemplatePath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
		if (jTemproary != "")
		{
			vector<json>vecJson;
			//先添加旧的紧急插播节目
			vecJson.push_back(jTemproary);
			vecJson.push_back(jTemplate);
			vecJson.push_back(jOverdue);
			vecJson.push_back(jDefault);
			vecJson.push_back(jALL);
			if (CheckFileExist(strOldTemplatePath))
			{
				CheckOldResource(vecJson);
			}
		}
		strPath = g_Config.m_strTemporaryJson;
		g_bIsTemproaryOn = TRUE;//当下发下来后，无论如何先激活，交给判断线程来处理
		jTemproary = jALL;
	}
	else
	{
		//正常下发
		CString strOldTemplatePath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strOldTemplate);
		if (jOverdue != "")
		{
			vector<json>vecJson;
			//先添加老json
			vecJson.push_back(jOverdue);
			vecJson.push_back(jTemplate);
			vecJson.push_back(jTemproary);
			vecJson.push_back(jDefault);
			vecJson.push_back(jALL);
			if (CheckFileExist(strOldTemplatePath))
			{
				CheckOldResource(vecJson);
			}
		}
		//判断template.json 是否存在，存在的话就可以删除old.json ,并重命名原template.json 为old
		CString strTemplatePath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTempalteJson);
		if (CheckFileExist(strTemplatePath))
		{
			DeleteFile(strOldTemplatePath);
			rename(strTemplatePath, strOldTemplatePath);
		}
		jOverdue = jTemplate;
		jTemplate = jALL;
	}

	ChangeCurrentJson(jALL);
	std::string strNewJson = jALL.dump(4);
	CFile filenew(GetFullPath(g_Config.m_strRelatePath + strPath.c_str()), CFile::modeCreate | CFile::modeWrite);
	CString strUtf = strNewJson.c_str();
	ConvertGBKToUtf8(strUtf);
	filenew.Write(strUtf, strUtf.GetLength());
	filenew.Close();
	return TRUE;
}

//检测json里是否有资源与旧json重复。2，3，4，5 json与1比较，1json里有的素材是2，3，4，5没有的话就删除这些素材
BOOL  CWeChatPrinterDlg::CheckOldResource(vector<json>vecJson)
{
	//将三个json里的素材全部统计出来，然后和旧的进行比较，
	//比较完的，将没有重复的素材进行删除处理，删除旧json,将当前json改成旧json完毕
	vector<vector<CString>>vecResource;
	vector<CString>vecLeft;
	vector<CString>vecOld;
	if (vecJson[0] == "")
	{
		return FALSE;
	}
	for (unsigned int i = 0; i < vecJson.size(); i++)
	{
		json jTemp = vecJson[i];
		if (jTemp == "") continue;
		vector<CString>vecTemp;
		FindContent(jTemp, vecTemp);
		vecResource.push_back(vecTemp);
	}
	vecOld = vecResource[0];
	for (unsigned int i = 0; i < vecOld.size(); i++)
	{
		BOOL bFind = FALSE;
		for (unsigned int m = 1; m < vecResource.size(); m++)
		{
			vector<CString>vecTemp2 = vecResource[m];
			for (unsigned int n = 0; n < vecTemp2.size(); n++)
			{
				if (vecOld[i].CompareNoCase(vecTemp2[n]) == 0)
				{
					bFind = TRUE;
					break;
				}
			}
			if (bFind)break;
		}
		if (bFind)continue;
		vecLeft.push_back(vecOld[i]);
	}
	for (unsigned int i = 0; i < vecLeft.size(); i++)
	{
		CString strTemp = g_Config.m_strRelatePath + vecLeft[i];
		DeleteFile(GetFullPath(strTemp));
	}
	return TRUE;
}

//检测本地是否还有没有下载完的文件
BOOL CWeChatPrinterDlg::CheckIncompleted()
{
	CString strPath = GetFullPath(g_Config.m_strRelatePath + "www\\static\\*.download");
	//查找本地有没有download文件，有就拼字段，去调用rmqde函数，调接口。
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strPath);
	BOOL bFind = FALSE;
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		CString strname = finder.GetFileName();
		bFind = TRUE;
		break;
	}	if (bFind)
	{
		CString strOrder = "PUBLISH|" + g_Config.m_strDeviceCode + "|";
		json jTemp = GetCurrentJson();
		CString strItemID = jTemp["body"]["data"]["itemid"].get<std::string>().c_str();
		CString strType = "ITEM";
		json jBody = jTemp["body"];
		//if (jBody.find("datalist") != jBody.end())
		if (jBody["datalist"].is_null() == FALSE)
		{
			strType = "ITEMLIST";
		}
		RMQ_DealCustomMsg(strOrder + strItemID + "|" + strType);
		return FALSE;
	}
	nCheckTimes = 0;
	return TRUE;
}
/****************************		功能函数		*****************************************/

//排除掉容器里重复的元素，和为""的值
template<typename T>
void deduplication(T& c)
{
	//①首先将vector排序
	sort(c.begin(), c.end());
	//②然后使用unique算法,unique返回值是重复元素的开始位置。
	T::iterator new_end = unique(c.begin(), c.end());//"删除"相邻的重复元素
	//③最后删除后面的那段重复部分
	c.erase(new_end, c.end());//删除(真正的删除)重复的元素

	for (unsigned int i = 0; i < c.size(); i++)
	{
		if (c[i] == "")
		{
			c.erase(c.begin() + i);
			break;
		}
	}
}

BOOL  FindContent(json jALL, vector<CString> &vecTemp)
{
	if (jALL == "")
	{
		return FALSE;
	}
	json j = jALL["body"]["data"];
	json jBody = jALL["body"];
	BOOL bIsSingleProgram = FALSE;
	if (j.find("itemtemplatejson") != j.end() && jBody.find("datalist") == jBody.end())
	{
		bIsSingleProgram = TRUE;
	}
	else
		j = jALL["body"]["datalist"];

	for (unsigned int i = 0; bIsSingleProgram || i < j.size(); i++)
	{
		json jPage;
		//判断是否是单模板
		if (bIsSingleProgram)
			jPage = j["itemtemplatejson"]["page"];
		else
			jPage = j[i]["itemtemplatejson"]["page"];
		if (jPage.is_array() == FALSE)
		{
			CString strLastErr = "解析template json[page]格式不正确";
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "FindContent", "[err][%s]", strLastErr);
			return FALSE;
		}

		json jArray_edit = json::array();//创建一个空数组
		json jo = json::object();		 //创建一个空对象
		for (json::iterator it = jPage.begin(); it != jPage.end(); jArray_edit.push_back(jo), ++it)
		{
			jo = *it;
			json jArray_edit2 = json::array();//创建一个空数组
			json jo2 = json::object();		 //创建一个空对象
			json jElements = jo["elements"];
			for (json::iterator it2 = jElements.begin(); it2 != jElements.end(); jArray_edit2.push_back(jo2), ++it2)
			{
				// array转object
				jo2 = *it2;
				if (jo2.is_object() == false) continue;
				std::string strFilePath = "www/static/";

				//-----------------//
				//group里的内容//
				//-----------------//
				json jArray_edit3 = json::array();//创建一个空数组
				json jo3 = json::object();		  //创建一个空对象
				json jGroup = jo2["group"];
				for (json::iterator it3 = jGroup.begin(); it3 != jGroup.end(); jArray_edit3.push_back(jo3), ++it3)
				{
					jo3 = *it3;
					//-----------------//
					//bgImg里的内容//
					//-----------------//
					std::string strUrl = jo3["bgImg"].get<std::string>();
					vecTemp.push_back(strUrl.c_str());

					//排除掉非 img、vidoe、carousel 里的content资源下载，例如btn的content可能是一串中文
					std::string strType = jo3["type"].get<std::string>();
					if (strType == "img" || strType == "video" || strType == "carousel")
					{
						json::iterator posGroupContent = jo3.find("content");
						json jGroupUrls(posGroupContent.value());
						std::string strUrl = *jGroupUrls.begin();
						if ("" == strUrl)  continue;
						vecTemp.push_back(strUrl.c_str());
					}
				}
				//--------------------//
				//resource里的内容//
				//--------------------//
				json jArray_edit4 = json::array();//创建一个空数组
				json jo4 = json::object();		  //创建一个空对象
				json jResource = jo2["resource"];
				for (json::iterator it3 = jResource.begin(); it3 != jResource.end(); jArray_edit4.push_back(jo4), ++it3)
				{
					jo4 = *it3;
					json::iterator posGroupContent = jo4.find("content");
					json jResource(posGroupContent.value());
					std::string strUrl = *jResource.begin();
					if ("" == strUrl)  continue;
					vecTemp.push_back(strUrl.c_str());

				}
				//-------------------//
				//content里的内容//
				//-------------------//
				//排除掉非 img、vidoe、carousel 里的content资源下载，例如btn的content可能是一串中文
				std::string strType = jo2["type"].get<std::string>();
				if (strType == "img" || strType == "video" || strType == "carousel")
				{
					json::iterator posContent = jo2.find("content");
					if (posContent == jo2.end() || posContent.value().is_string() == false) continue;
					json jUrls(posContent.value());
					std::string strUrl = *jUrls.begin();
					if ("" == strUrl)  continue;
					vecTemp.push_back(strUrl.c_str());
				}
				//-----------------//
				//bgImg里的内容//
				//-----------------//
				std::string strUrl = jo2["bgImg"].get<std::string>();
				vecTemp.push_back(strUrl.c_str());

			}
			//-----------------//
			//bgImg里的内容//
			//-----------------//
			std::string strUrl = jo["bgImg"].get<std::string>();
			vecTemp.push_back(strUrl.c_str());
		}
		if (bIsSingleProgram)
		{
			break;
		}
	}

	//排除掉为空，重复的选项
	// int ary[] = { 1, 1, 2, 3, 2, 4, 3 };
	// vector<int> vec(ary, ary + sizeof(ary) / sizeof(int));
	// deduplication(vec);
	deduplication(vecTemp);
	return TRUE;
}

void DownLoadFile(CString strUrl, CString strLocalPath)
{
#ifndef DOWNLOAD_NO
	if (FALSE == g_bDownload)
	{
		return;
	}

	CString strTargetFile = GetFullPath(g_Config.m_strRelatePath + strLocalPath);
	if (FALSE == CheckFileExist(strTargetFile))
	{
		//int nDownload = HTTP_Download(g_Config.m_strHttpDwonloadUrl + strUrl, strTargetFile);
		int nDownload = HTTP_Download2(g_Config.m_strHttpDwonloadUrl + strUrl, strTargetFile,"admin","123456","");
		if (nDownload != TRUE)
		{
			CString strLastErr = "";
			strLastErr.Format("下载失败[%s]", g_Config.m_strHttpDwonloadUrl + strUrl);
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "DownLoadFile", "[HTTP_Download()][err][%d][%s]",
				nDownload, strLastErr);
		}
		else
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "DownLoadFile", "[HTTP_Download()][ok][%s]",
				g_Config.m_strHttpDwonloadUrl + strUrl);
		}
	}
#endif
}

BOOL CheckTimeLimit(CString strEndDate)
{
	CString strResult = "";
	CString strCurDate = GetCurTime(DAY_LONG).c_str();
	if (strCurDate > strEndDate)
	{
		CString strYear = strEndDate.Mid(0, 4);
		CString strMonth = strEndDate.Mid(5, 2);
		CString strDay = strEndDate.Mid(8, 2);
		strResult.Format("节目已过期，截至时间至%s年%s月%s日", strYear, strMonth, strDay);
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "CheckTimeLimit", "%s", strResult);
		return TRUE;
	}
	return FALSE;
}

vector<int> getHours(CString strHours)
{
	//08:00-09:00|11:00-12:00
	vector<int>vecTime;
	vector<CString>vecHours = SplitString(strHours, "|");
	CString strTemp = "";
	for (unsigned int i = 0; i < vecHours.size(); i++)
	{
		vector<CString>vecHM = SplitString(vecHours[i], "-");
		if (vecHM.size() % 2 != 0)
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "getHours", "日期格式错误[%s]", strHours);
			return vecTime;
		}
		int iBeginTime = atoi(vecHM[0].Mid(0, 2) + vecHM[0].Mid(3, 2) + "00");
		int iLastTime = atoi(vecHM[1].Mid(0, 2) + vecHM[1].Mid(3, 2) + "00");
		vecTime.push_back(iBeginTime);
		vecTime.push_back(iLastTime);
	}
	return vecTime;
}

void getDays(CString strDays, vector<int> &vecDay, vector<int> &vecHM)
{
	vector<CString>vecDate = SplitString(strDays, "|");

	CString strTemp = "";
	CString strHM = "";
	for (unsigned int i = 0; i < vecDate.size(); i++)
	{
		if (vecDate[i].Find(":") < 0)
		{
			CString strTemp2 = vecDate[i];
			strTemp2.Replace("-", "");
			vecDay.push_back(atoi(strTemp2));
		}
		else
			strTemp += vecDate[i] + "|";
	}
	strHM = strTemp.Left(strTemp.GetLength() - 1);
	vecHM = getHours(strHM);

}

int GetWaitTime(vector<int>vecHM, int iCurrentTime, int iPlayMode)
{
	int isize = vecHM.size();
	int iRemainingTime = 0;
	int * pTime = new int[isize + 1];
	for (unsigned int i = 0; i < vecHM.size(); i++)
		pTime[i] = vecHM[i];
	pTime[isize] = iCurrentTime;
	qsort(pTime, isize + 1, sizeof(int), cmpfunc);//900，1020，1000，1100，1200排序

	int iNum = 0;
	for (iNum = 0; iNum < isize; iNum++)
	{
		if (pTime[iNum] == iCurrentTime)//在范围内的已经被排除了。所以目的是找到1100
		{
			break;
		}
	}

	if (iNum == isize)//处于末位，如1300，那要转一天了。
	{
		//当传入的为周重复和自定义模式，在函数中判断为末尾需要转一天的情况，就不执行倒计时
		if (iPlayMode > PLAY_REPEATDAY)
		{
			iRemainingTime = -1;
		}
		else
			iRemainingTime = 24 * 60 * 60 - (iCurrentTime / 10000 - pTime[0] / 10000) * 60 * 60
			- (iCurrentTime / 100 % 100 - pTime[0] / 100 % 100) * 60
			- (iCurrentTime % 100 % 100 - pTime[0] % 100 % 100);
	}
	else
	{
		// 		int a = (pTime[iNum + 1] / 10000 - iCurrentTime / 10000) * 60 * 60;
		// 		int b = (pTime[iNum + 1] / 100 % 100 - iCurrentTime / 100 % 100) * 60;
		// 		int c= (pTime[iNum + 1] % 100%100 );
		// 		int d = iCurrentTime % 100 % 100;//154000 直接余10000的话，得到的是4000，余掉了1w，剩下千位
		iRemainingTime = (pTime[iNum + 1] / 10000 - iCurrentTime / 10000) * 60 * 60
			+ (pTime[iNum + 1] / 100 % 100 - iCurrentTime / 100 % 100) * 60
			+ (pTime[iNum + 1] % 100 % 100 - iCurrentTime % 100 % 100);
	}

	delete[]pTime;
	pTime = NULL;

	return iRemainingTime;
}

int GetCloseTime(vector<int>vecHM, int iCurrentTime)
{
	int isize = vecHM.size();
	int * pTime = new int[isize + 1];
	for (unsigned int i = 0; i < vecHM.size(); i++)
		pTime[i] = vecHM[i];
	pTime[isize] = iCurrentTime;
	qsort(pTime, isize + 1, sizeof(int), cmpfunc);//900，1020，1000，1100，1200排序

	int iNum = 0;
	for (iNum = 0; iNum < isize; iNum++)
	{
		if (pTime[iNum] == iCurrentTime)//在范围内的已经被排除了。所以目的是找到1100
		{
			break;
		}
	}

	int	iCloseRemainingTime = (pTime[iNum + 1] / 10000 - iCurrentTime / 10000) * 60 * 60
		+ (pTime[iNum + 1] / 100 % 100 - iCurrentTime / 100 % 100) * 60
		+ (pTime[iNum + 1] % 100 % 100 - iCurrentTime % 100 % 100);

	delete[]pTime;
	pTime = NULL;

	return iCloseRemainingTime;
}

BOOL CWeChatPrinterDlg::ExtendDeadline(CString strProgammeID, CString strNewDeadline, vector<CString> vecTime)
{
	json jALL = GetCurrentJson();
	if (jALL == "")
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ExtendDeadline", "当前节目为空，异常");
		return FALSE;
	}
	if (jALL == jDefault)
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ExtendDeadline", "当前是默认节目，无法延长时间");
		return FALSE;
	}
	json jData = jALL["body"]["data"];
	if (jData.find("itemenddate") != jData.end())
	{
		CString strItemID = jData["itemid"].get<std::string>().c_str();
		if (strItemID.CompareNoCase(strProgammeID) != 0)
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ExtendDeadline", "指令的节目ID与节目ID不符");
			return FALSE;
		}
		jData["itemenddate"] = strNewDeadline;
		int  nPlaymode = atoi(jData["playmode"].get<std::string>().c_str());
		if (nPlaymode != PLAY_ALLDAY)
		{
			CString strTimeseg = "";
			for (unsigned int i = 0; i < vecTime.size(); i++)
			{
				strTimeseg += vecTime[i];
				if (vecTime.begin() + i + 1 == vecTime.end())
				{
					break;
				}
				strTimeseg += "|";
			}
			if (strTimeseg != "")
				jData["timeseg"] = strTimeseg;
		}
	}
	else if (jData.find("itemlistendtime") != jData.end())
	{
		CString strItemListID = jData["itemlistid"].get<std::string>().c_str();
		if (strItemListID.CompareNoCase(strProgammeID) != 0)
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ExtendDeadline", "指令的节目单ID与节目单ID不符");
			return FALSE;
		}
		jData["itemlistendtime"] = strNewDeadline;
	}
	jALL["body"]["data"] = jData;

	//根据当前下发的节目是哪一种来修改程序中的时间，以及本地的json
	CString strJsonPath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTempalteJson);

	int iReleseMode = 0;
	if (jData.find("releasemode") != jData.end())
	{
		iReleseMode = atoi(jData["releasemode"].get<std::string>().c_str());
	}

	if (iReleseMode == TEMPORARY_JSON)
	{
		jTemproary = jALL;
		strJsonPath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
	}
	else
	{
		jTemplate = jALL;
	}
	//修改当前播放节目
	ChangeCurrentJson(jALL);
	std::string strNewJson = jALL.dump(4);
	CFile filenew(strJsonPath, CFile::modeCreate | CFile::modeWrite);
	CString strUtf = strNewJson.c_str();
	ConvertGBKToUtf8(strUtf);
	filenew.Write(strUtf, strUtf.GetLength());
	filenew.Close();
	return TRUE;
}

// 屏幕截图,返回图片的路径
CString CWeChatPrinterDlg::ScreenShot(void)
{
	CWnd *pDesktop = GetDesktopWindow();
	CDC *pDC = pDesktop->GetDC();
	CRect rect;

	//获取窗口的大小  
	pDesktop->GetClientRect(&rect);

	//保存到的文件名
	CString strFileName(GetAppPathW().c_str());
	strFileName += _T("ScreenShot\\");
	CreateDirectory((LPCTSTR)strFileName, NULL);
	CTime t = CTime::GetCurrentTime();
	CString tt = t.Format("%Y%m%d_%H%M%S");
	strFileName += tt;
	strFileName += _T(".png");
	//保存为PNG
	CMakePNG MakePNG;
	BOOL bret = MakePNG.MakePNG(pDC->m_hDC, rect, strFileName);
	if (FALSE == bret)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ScreenShot", " \n截图失败-[%s]\n", MakePNG.getErrorMsg());
	}
	ReleaseDC(pDC);
	return strFileName;
}

//检查自动更新程序是否启动
BOOL CWeChatPrinterDlg::CheckUpdate()
{
	//检验自动更新程序是否启动，没有的话就将自己关闭再启动自己。
	CString strMsgPath = GetFullPath(AutoUpdateMsg);
	if (FALSE == CheckFileExist(strMsgPath))
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "CheckUpdate", " [%s]未找到", strMsgPath);
		CString strPath = GetFullPath(AutoUpdateEXE);
		//StartProcess2(strPath, FALSE);//2020.12.29 失效了，打开闪退,原因：路径里有空格
		StartProcess3(strPath);
		return FALSE;
	}
	BOOL bPassed = GetPrivateProfileInt("Time", "Passed", 0, strMsgPath);
	BOOL bUpdated = GetPrivateProfileInt("Time", "Updated", 0, strMsgPath);
	int iProcessID = FindProcess("AutoUpdate.exe");
	if (FALSE == bPassed || FALSE == bUpdated || iProcessID <= 0)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "CheckUpdate", "%d %d %d", bPassed, bUpdated, iProcessID);

		if (iProcessID <= 0)//一定是由自动更新启动，且还未关闭的情况下启动本EXE。
		{
			CString strPath = GetFullPath(AutoUpdateEXE);
			StartProcess3(strPath);
		}
		return FALSE;
	}
	return TRUE;
}

//删除旧的素材
void CWeChatPrinterDlg::DelateResource()
{
	CString strOldTemplatePath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
	if (jTemproary != "")
	{
		vector<json>vecJson;
		//先添加旧的紧急插播节目
		vecJson.push_back(jTemproary);
		vecJson.push_back(jTemplate);
		vecJson.push_back(jOverdue);
		vecJson.push_back(jDefault);
		if (CheckFileExist(strOldTemplatePath))
		{
			CheckOldResource(vecJson);
		}
	}
}

//睡眠函数1，倒计时进入休眠
void CWeChatPrinterDlg::sleepFunction1(int Time)//伪动态的sleep，用于长时间睡眠却无法主动退出的线程
{
	int iTime = Time / 1000;
	//退出条件：时间到达，退出程序，休眠
	while (--iTime && FALSE == m_bExit)
	{
		Sleep(1000);
	}
}

/****************************	RMQ		*****************************************/
BOOL CWeChatPrinterDlg::LoadRMQPubAndRMQSUBDLL()
{
	HINSTANCE hRMQDll = LoadLibrary("RMQ.dll");
	_RMQ_SUB = (lpRMQ_SUB)GetProcAddress(hRMQDll, "_RMQ_SUB");
	_RMQ_CALLBACK = (lpRMQ_CALLBACK)GetProcAddress(hRMQDll, "_RMQ_CALLBACK");
	if (NULL == hRMQDll)
	{
		m_strLastErr = "RMQ_SUB动态库加载失败...";
		goto EXIT;
	}
	if (NULL == _RMQ_CALLBACK)
	{
		m_strLastErr = "_RMQ_CALLBACK加载失败...";
		goto EXIT;
	}
	if (NULL == _RMQ_SUB)
	{
		m_strLastErr = "_RMQ_SUB加载失败...";
		goto EXIT;
	}
	_RMQ_CALLBACK(_Recv);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "LoadRMQPubAndRMQSUBDLL", "动态库加载成功");
	return TRUE;
EXIT:
	LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadRMQPubAndRMQSUBDLL", "动态库加载失败:[%s]", m_strLastErr);
	return FALSE;
}
////////////////////////////////////////////////////////////////////
BOOL CWeChatPrinterDlg::RMQ_SUBConnect()
{
	if (g_Config.m_strDeviceCode.GetLength() <= 0 || g_Config.m_strIPAddress.GetLength() <= 0 || g_Config.m_nPort < 0
		|| g_Config.m_strAccount.GetLength() <= 0 || g_Config.m_strPassword.GetLength() <= 0 || g_Config.m_nModeName < 0
		|| g_Config.m_strExchangeName.GetLength() <= 0 || g_Config.m_vecRouteKey.size() <= 0 || g_Config.m_nChannel < 0)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQSUBConnect", "配置文件参数填写异常...");
		return FALSE;
	}
	CString strRouteKey = "";
	for (unsigned int i = 0; i < g_Config.m_vecRouteKey.size(); i++)
	{
		if ("" != g_Config.m_vecRouteKey[i])
			strRouteKey += ("" == strRouteKey ? "" : "|") + g_Config.m_vecRouteKey[i];
	}

	int nState = _RMQ_SUB(g_Config.m_strIPAddress, g_Config.m_nPort, g_Config.m_strAccount,
		g_Config.m_strPassword, g_Config.m_strExchangeName, g_Config.m_strDeviceCode,
		strRouteKey, g_Config.m_nModeName, g_Config.m_nChannel);

	if (nState != 0)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQSUBConnect", "连接RabbitMQ平台失败...");
		return FALSE;
	}
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQSUBConnect", "连接RabbitMQ平台成功...");

	return TRUE;
}
////////////////////////////////////////////////////////////////////
BOOL CWeChatPrinterDlg::RMQ_DealCustomMsg(CString strMsg)
{
	if (strMsg == "") return FALSE;
	std::vector<CString> vct = SplitString(strMsg.GetBuffer(), "|");
	strMsg.ReleaseBuffer();
	if (vct[0].CompareNoCase("OFFLINE") != 0)
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_RMQ, "RMQ_DealCustomMsg", "[%s]", strMsg);
	}
	else
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_OFFLINE, "RMQ_DealCustomMsg", "[%s]\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", strMsg);
	}
	if (vct.size() >= 1)
	{
		if (vct[0].CompareNoCase("PUBLISH") == 0 && vct.size() == 4)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "收到更新节目指令");
			CString strJson = "";
			m_nMode = 0;
			if (vct[3] == "ITEMLIST") m_nMode = MULITIPLEPROGRAM;
			else if (vct[3] == "ITEM") m_nMode = SINGLEPROGRAM;
			else return FALSE;
			BOOL bRet = g_toolTrade.GetTemplate(g_Config.m_strHttpUrl, strJson, m_nMode);
			if (bRet)
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "GetTemplate 获取节目JSON成功");
				bRet = g_toolTrade.UpLoadProGrameStatus(g_Config.m_strHttpUrl, m_nMode, 2);
				if (bRet)
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadProGrameStatus 上传正在下载节目状态成功");
				}
				else
				{
					LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadProGrameStatus 上传正在下载节目状态失败[%s]", g_toolTrade.GetLastErr());
				}
				bRet = ParseNewTemplateOfH5(strJson, m_nMode == SINGLEPROGRAM ? TRUE : FALSE);
				if (bRet)
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "ParseNewTemplateOfH5 解析更新节目成功");
					g_strItemid = "";
					SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
				}
				else
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "ParseNewTemplateOfH5 解析更新节目失败");
				}
				//上传是否下载模板成功信息
				bRet = g_toolTrade.UpLoadProGrameStatus(g_Config.m_strHttpUrl, m_nMode, bRet == TRUE ? 3 : 4);
				if (bRet)
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadProGrameStatus 上传更新节目消息成功");
				}
				else
				{
					LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadProGrameStatus 上传更新节目消息失败[%s]", g_toolTrade.GetLastErr());
				}
				SetTimer(TIMER_CHECKINCOMPELEDFILE, 1000, NULL);
			}
			else
			{
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "GetTemplate 获取更新节目失败 [%s]", g_toolTrade.GetLastErr());
			}
		}
		else if (vct[0].CompareNoCase("DEFAULT") == 0)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "取消发布，加载默认节目");
			//避免紧急插播状态对后续程序加载造成影响
			g_bIsTemproaryOn = FALSE;
			CString strJsonPath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
			//避免紧急插播的优先级对后续普通节目造成影响，直接删除
			DeleteFile(strJsonPath);
			ChangeCurrentJson(jDefault);
			SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
		}
		else if (vct[0].CompareNoCase("OFFLINE") == 0)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_OFFLINE, "RMQ_DealCustomMsg", "收到上送设备状态消息");
			SetEvent(m_hDviceStatusEvent);
			// 			if (FALSE == g_bIsOnLinePro)
			// 			{
			// 				SwitchJsonToOnline();
			// 			}
			// 			SetTimer(TIMER_LOADOFFLINE, OFFLINETIMELIMTE * 1000, NULL);
		}
		else if (vct[0].CompareNoCase("DEVICE") == 0)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "收到更新程序通知");
			CString strPath = GetFullPath(AutoUpdateEXE);
			StartProcess3(strPath);
			//StartProcess(strPath, "", 0, 0);
			EndDialog(FALSE);
		}
		else if (vct[0].CompareNoCase("EXCHANGETIME") == 0 && vct.size() >= 3)
		{
			vector<CString> vecTime;
			vecTime.assign(vct.begin() + 3, vct.end());
			if (ExtendDeadline(vct[1], vct[2], vecTime))
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "设置新的截止时间到%s", vct[2]);
				SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
			}
		}
		else if (vct[0].CompareNoCase("DEVICECONTROLL") == 0)//设备控制
		{
			if (vct[1].CompareNoCase(g_Config.m_strDeviceCode) == 0)
			{
				if (vct[2].CompareNoCase("TURNOFF") == 0)//关机
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "设备即将关机");
					ShutDown();
				}
				else if (vct[2].CompareNoCase("TURNBACK") == 0)//重启
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "设备即将重启");
					Reboot();
				}
				else if (vct[2].CompareNoCase("VOLUMECONTRO") == 0)//控制音量
				{
					CString strVolumn = vct[3];
					strVolumn.Replace("%", "");
					int nVolumn = atoi(strVolumn);
					if (nVolumn >= 0 && nVolumn <= 100)
					{
						SetVolumeLevel(nVolumn);
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "当前音量设置为 %d", nVolumn);
					}
				}
				else if (vct[2].CompareNoCase("SCREENCUT") == 0)//截屏上传
				{
					CString strPicName = vct[3];
					CString strFile = ScreenShot();
					CString strBase64Pic = Base64EncodePic(strFile);
					BOOL bRet = g_toolTrade.UpLoadPic(g_Config.m_strHttpUrl, strBase64Pic, strPicName);
					if (bRet)
					{
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadPic 上传截图成功");
					}
					else
					{
						LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadPic 上传截图失败 [%s]", g_toolTrade.GetLastErr());
					}
				}
				else if (vct[2].CompareNoCase("TURNSTATUS") == 0)
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "主动刷新通知成功");
					SetEvent(m_hDviceStatusEvent);
				}
			}
		}
		else if (vct[0].CompareNoCase("CONTROL") == 0)
		{
			if (g_strItemid == vct[1])
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "节目的itemid为%s,与当前相同，不切换", g_strItemid);
				return TRUE;
			}
			g_strItemid = vct[1];
			bUnderSwitchMode = TRUE;
			SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "切换节目的itemid为%s", g_strItemid);
		}
		else if (vct[0].CompareNoCase("CONTROLRECOVERY") == 0)
		{
			bUnderSwitchMode = FALSE;
			SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "恢复正常播放");
		}
		else
		{
			// 未知消息
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "[未知消息][%s]", vct[0]);
		}
	}
	return TRUE;
}

/****************************	线程	*****************************************/
DWORD CWeChatPrinterDlg::RMQThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg* pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->RMQThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::RMQThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		Sleep(100);

		DWORD dwResult = WaitForSingleObject(m_hRMQEvent, INFINITE);

		if (TRUE == m_bExit) return TRUE;

		if (dwResult == WAIT_OBJECT_0)
		{
			RMQ_DealCustomMsg(g_strRecvMsg);
		}
		ResetEvent(m_hRMQEvent);
	}

	return TRUE;
}

DWORD CWeChatPrinterDlg::ReSignThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg* pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->ReSignThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::ReSignThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		DWORD dwResult = WaitForSingleObject(m_hReSignEvent, INFINITE);
		if (TRUE == m_bExit) return TRUE;
		if (dwResult == WAIT_OBJECT_0)
		{
			BOOL bRet = g_toolTrade.Login(g_Config.m_strHttpUrl, m_iNextBitSpace, m_strLastSignTime, m_OnlineTime,m_strOrgCode,m_strDeviceType);
			if (FALSE == bRet)
			{
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ReSignThreadContent", "[g_toolTrade.Login][err][%s]", g_toolTrade.GetLastErr());
				//签到失败，20s重新签到一次
				SetTimer(TIMER_RESIGN, 20 * 1000, NULL);
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ReSignThreadContent", "签到失败，20s后重新签到...");
			}
			else
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ReSignThreadContent", "签到成功");
				//覆盖原先的机构号和设备类型，再加载RMQ配置，再连接平台
				g_Config.WriteStringToCfgDev("RMQ","OrgCode", m_strOrgCode);
				g_Config.WriteStringToCfgDev("RMQ", "DeviceType", m_strDeviceType);
				g_Config.LoadRMQCfg();
				RMQ_SUBConnect();

				//测试功能，心跳，暂未上线
				//SetEvent(m_hHeartBeatEvent);
				break;
			}
		}
		ResetEvent(m_hReSignEvent);
	}
	return TRUE;
}

DWORD CWeChatPrinterDlg::DeviceStatusThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg* pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->DeviceStatusThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::DeviceStatusThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		DWORD dwResult = WaitForSingleObject(m_hDviceStatusEvent, INFINITE);
		if (TRUE == m_bExit) return TRUE;
		if (dwResult == WAIT_OBJECT_0)
		{
			//上传设备状态
			BOOL bRet = g_toolTrade.UpLoadDeviceStatus(g_Config.m_strHttpUrl);

			if (bRet)
			{
				CTime tmBegin = CTime::GetCurrentTime();
				CString str_Begin = tmBegin.Format("%Y%m%d%H%M%S");
				LOG2(LOGTYPE_DEBUG, LOG_NAME_OFFLINE, "DeviceStatusThreadContent", "[设备状态上传成功！][%s]", str_Begin);
			}
			else
			{
				CTime tmBegin = CTime::GetCurrentTime();
				CString str_Begin = tmBegin.Format("%Y%m%d%H%M%S");
				LOG2(LOGTYPE_DEBUG, LOG_NAME_OFFLINE, "DeviceStatusThreadContent", "[设备状态上传失败！][%s][%s]", str_Begin, g_toolTrade.GetLastErr());
			}
		}
		ResetEvent(m_hDviceStatusEvent);
	}
	return TRUE;
}

DWORD CWeChatPrinterDlg::ChooseProgramThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg *pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->ChooseProgramThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::ChooseProgramThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		Sleep(10);
		DWORD dwResult = WaitForSingleObject(m_hChooseProgramEvent, INFINITE);
		if (TRUE == m_bExit) return TRUE;
		if (dwResult == WAIT_OBJECT_0)
		{
			BOOL bInTimeArea = FALSE;													//是否在播放时间区域内
			CString strCurDate = GetCurTime(DATE_NORMAL).c_str();						//YYYYMMDDHHMMSS, atoi()有效长度为10
			int iCloseRemainingTime = 0;												//距离关闭所剩余时间
			int iRemainingTime = 0;														//距离播出所剩余时间
			int iPlayMode = 0;															//轮播模式
			int iReleaseMode = 1;														//插播模式	
			int iCurDate = atoi(strCurDate.Left(8));									//获得当前日期，YYYYMMDD，20200407
			int iCurTime = atoi(strCurDate.Mid(8, 6));									//获得当前时间，HHMMSS的格式，即101830
			vector<int> vecHM;															//保存当日节目的所有时间
			CString strTempItemID = "";													//循环中临时存放节目id
			json jTemp = GetCurrentJson();
			//如果template被删除，当前节目可能为空，此时使用默认
			if (jTemp == "")
			{
				ChangeCurrentJson(jDefault);
				jTemp = jDefault;
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "当前无其他节目，开始播放默认节目，全天");
			}
			//单节目		
			if (jTemp["body"].find("datalist") == jTemp["body"].end())
			{
				json jData = jTemp["body"]["data"];
				iPlayMode = atoi(jData["playmode"].get<std::string>().c_str());
				iReleaseMode = atoi(jTemp["body"]["data"]["releasemode"].get<std::string>().c_str());
				if (iReleaseMode == NEW_JSON && FALSE == g_bIsTemproaryOn)
				{
					//1 全天播放模式
					if (PLAY_ALLDAY == iPlayMode)
					{
						jForIE = jData["itemtemplatejson"];
						SetTimer(TIMER_LOADPAGE, 500, NULL);
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "开始播放当前节目，单节目，全天");
					}
					//2 分时段播放模式
					else if (PLAY_DIVIDE == iPlayMode)
					{
						jForIE = jData["itemtemplatejson"];
						CString strPlaytime = jData["timeseg"].get<std::string>().c_str();
						vecHM = getHours(strPlaytime);
						//Mode 1 在范围内，立即播放。倒计时关闭时间
						for (unsigned int i = 0; i < vecHM.size(); i += 2)
						{
							if (iCurTime >= vecHM[i] && iCurTime <= vecHM[i + 1])
							{
								bInTimeArea = TRUE;
								SetTimer(TIMER_LOADPAGE, 500, NULL);
								iCloseRemainingTime = GetCloseTime(vecHM, iCurTime);
								SetTimer(TIMER_CHOOSEPROGAME, iCloseRemainingTime * 1000, NULL);
								LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "开始播放当前节目，单节目，将于%d分钟后切换到其他节目", (int)(iCloseRemainingTime/60));
								break;
							}
						}
						//Mode 2 不在范围内，倒计时播放
						if (FALSE == bInTimeArea)
						{
							jForIE = jDefault["body"]["data"]["itemtemplatejson"];//修改当前json,加载默认的
							SetTimer(TIMER_LOADPAGE, 500, NULL);
							iRemainingTime = GetWaitTime(vecHM, iCurTime);
							SetTimer(TIMER_CHOOSEPROGAME, iRemainingTime * 1000, NULL);
							LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "开始播放默认节目，单节目，将于%d分钟后切换当前节目", (int)(iRemainingTime / 60));

						}
					}
				}
				else if (iReleaseMode == TEMPORARY_JSON)
				{
					//要求紧急插播节目存在 并且 不在等待时间段

					json jData = jTemp["body"]["data"];
					iPlayMode = atoi(jData["playmode"].get<std::string>().c_str());
					//1 全天播放模式
					if (PLAY_ALLDAY == iPlayMode)
					{
						g_bIsTemproaryOn = TRUE;
						jForIE = jData["itemtemplatejson"];
						SetTimer(TIMER_LOADPAGE, 500, NULL);
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "开始播放紧急插播节目，全天");
					}
					//2 分时段播放模式
					else if (PLAY_DIVIDE == iPlayMode)
					{
						jForIE = jData["itemtemplatejson"];
						CString strPlaytime = jData["timeseg"].get<std::string>().c_str();
						vecHM = getHours(strPlaytime);
						//Mode 1 在范围内，立即播放。倒计时关闭时间
						for (unsigned int i = 0; i < vecHM.size(); i += 2)
						{
							if (iCurTime >= vecHM[i] && iCurTime <= vecHM[i + 1])
							{
								g_bIsTemproaryOn = TRUE;
								bInTimeArea = TRUE;
								SetTimer(TIMER_LOADPAGE, 500, NULL);
								iCloseRemainingTime = GetCloseTime(vecHM, iCurTime);
								SetTimer(TIMER_CHOOSEPROGAME, iCloseRemainingTime * 1000, NULL);
								LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "开始播放紧急插播节目，将于%d分钟后切换到其他节目", (int)(iCloseRemainingTime / 60));
								break;
							}
						}
						//Mode 2 不在范围内，倒计时播放
						if (FALSE == bInTimeArea)
						{
							int g_iTemproaryRemainingTime = GetWaitTime(vecHM, iCurTime);		  //紧急节目等待播放时间
							SetTimer(TIMER_LOADTEMPORARY, g_iTemproaryRemainingTime * 1000, NULL);//倒计时，killertime不会杀死这个事件，与加载节目的time事件不冲突。
							ChangeCurrentJson(jTemplate);											//加载原先的节目
							g_bIsTemproaryOn = FALSE;
							SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
							LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "开始播放紧急插播节目，将于%d分钟后切换当前节目", (int)(iRemainingTime / 60));
						}
					}
				}
			}
			//多节目
			else
			{
				json jDataList = jTemp["body"]["datalist"];
				json jTodayData;														//当日的datalist数组成员
				int iCurWeekDay = GetWeekDay();											//今天的星期
				vector<int> vecALLHM;													//保存当日节目单里的所有时间，用于倒计时启动节目

				for (json::iterator it = jDataList.begin(); it != jDataList.end(); ++it)
				{
					jTodayData = *it;
					if (jTodayData.find("carouselmode") != jTodayData.end())// 当平台下发中的日重复没有时间，这个东西没有，时间数组内容也是 nulll
					{
						iPlayMode = atoi(jTodayData["carouselmode"].get<std::string>().c_str());
					}
					else
					{
						goto NO_TIME;
					}
					jForIE = jTodayData["itemtemplatejson"];

					strTempItemID = jTodayData["itemid"].get< std::string>().c_str();
					if (bUnderSwitchMode)
					{
						if (g_strItemid.CompareNoCase(strTempItemID) == 0 )
						{
							SetTimer(TIMER_LOADPAGE, 500, NULL);
							g_strItemid = strTempItemID;
							bUnderSwitchMode = FALSE;
							break;
						}
						continue;
					}

					for (json::iterator it = jTodayData["timesegitem"].begin(); it != jTodayData["timesegitem"].end(); ++it)
					{
						BOOL bFindDay = FALSE;											//是否找到当日的datalist数组成员
						json jTemp = *it;
						CString strPlaytime = jTemp["timeseg"].get<std::string>().c_str();
						if (PLAY_WEEKDAY == iPlayMode || PLAY_REPEATDAY == iPlayMode)
							vecHM = getHours(strPlaytime);
						// 一、判断日期是否在范围内
						//周重复
						if (PLAY_WEEKDAY == iPlayMode)
						{
							CString strWeekDay = "";									//存放节目里的星期
							if (jTemp.find("weekday") != jTemp.end())
							{
								strWeekDay = jTemp["weekday"].get<std::string>().c_str();
							}
							if (atoi(strWeekDay) == iCurWeekDay)
							{
								bFindDay = TRUE;
							}
							if (FALSE == bFindDay) continue;//ok
						}
						//自定义
						else if (PLAY_CUSTOMIZE == iPlayMode)
						{
							vector<int> vecDay;											//存放自定义节目中的日期区域
							getDays(strPlaytime, vecDay, vecHM);
							//分为日期，和日期区域两种
							if (1 == vecDay.size())
							{
								if (iCurDate == vecDay[0]) bFindDay = TRUE;
							}
							else if (2 == vecDay.size())
							{
								if (iCurDate >= vecDay[0] && iCurDate <= vecDay[1]) bFindDay = TRUE;
							}
							if (FALSE == bFindDay) continue;
						}
						bFindDay = TRUE;//日重复，固定为找到的

						//二、 判断时间是否在范围内
						for (unsigned int i = 0; i < vecHM.size(); i += 2)
						{
							if (iCurTime >= vecHM[i] && iCurTime <= vecHM[i + 1])
							{
								bInTimeArea = TRUE;
								break;
							}
							vecALLHM.push_back(vecHM[i]);
							vecALLHM.push_back(vecHM[i + 1]);
						}
						if (bInTimeArea) break;
					}
				NO_TIME:
					//如果该节目单，在正确的日期，非最后一个成员 在有效的时间段，循环
					if (FALSE == bInTimeArea && (it + 1) != jDataList.end()) continue;

					//有符合时间的就退出循环
					if (bInTimeArea)
					{
						SetTimer(TIMER_LOADPAGE, 500, NULL);
						iCloseRemainingTime = GetCloseTime(vecHM, iCurTime);
						SetTimer(TIMER_CHOOSEPROGAME, iCloseRemainingTime * 1000, NULL);
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "开始播放多节目，将于%d分钟后切换到其他节目", (int)(iCloseRemainingTime / 60));
						break;
					}
					// 当在所有的节目里都找不到播出时间时，倒计时最近的播放
					else
					{
						jForIE = jDefault["body"]["data"]["itemtemplatejson"];//修改当前json,加载默认的
						SetTimer(TIMER_LOADPAGE, 500, NULL);
						//当传入的为周重复和自定义模式，在函数中判断为末尾需要转一天的情况，就不执行倒计时
						iRemainingTime = GetWaitTime(vecALLHM, iCurTime, iPlayMode);
						if (iRemainingTime >= 0)
						{
							SetTimer(TIMER_CHOOSEPROGAME, iRemainingTime * 1000, NULL);
							LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "开始播放默认节目，将于%d分钟后切换多节目", (int)(iRemainingTime / 60));
						}
						else
						{
							LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "当日无符合是简单的多节目，开始播放默认节目");
						}
						break;
					}

				}
			}
			ResetEvent(m_hChooseProgramEvent);
		}
	}
	return TRUE;
}

DWORD CWeChatPrinterDlg::HeartBeatThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg* pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->HeartBeatThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::HeartBeatThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		DWORD dwResult = WaitForSingleObject(m_hHeartBeatEvent, INFINITE);
		if (TRUE == m_bExit) return TRUE;
		if (dwResult == WAIT_OBJECT_0)
		{
			sleepFunction1(m_iNextBitSpace * 1000);
			if (TRUE == m_bExit) return TRUE;
			BOOL bRet = g_toolTrade.HeartBeat(g_Config.m_strHttpUrl, m_iNextBitSpace, m_strLastSignTime, m_OnlineTime);
			if (FALSE == bRet)
			{
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "HeartBeat", "[g_toolTrade.HeartBeat][err][%s]", g_toolTrade.GetLastErr());
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "HeartBeatThreadContent", "访问心跳接口失败...");
			}
		}
	}
	ResetEvent(m_hHeartBeatEvent);
	return TRUE;
}

/************************************************************************/
/*                               定时器                                  */
/************************************************************************/
void CWeChatPrinterDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString strTemporaryJsonPath = "";

	KillTimer(nIDEvent);
	switch (nIDEvent)
	{
	case TIMER_RESIGN:
		SetEvent(m_hReSignEvent);
		break;
	case TIMER_LOADPAGE:
		if (PostZipList())
		{
			LoadTemplate();
		}
		break;
	case TIMER_RECV_MSG:				//获取数据
		SetEvent(m_hRMQEvent);
		break;
	case TIMER_LOADTEMPORARY:			//加载紧急插播的节目
		ChangeCurrentJson(jTemproary);
		g_bIsTemproaryOn = TRUE;
		SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
		break;
	case TIMER_CHOOSEPROGAME:
		SetEvent(m_hChooseProgramEvent);
		break;
	case TIMER_CHECKINCOMPELEDFILE:		//检查文件是否完整
		if (nCheckTimes++ < 3)
		{
			CheckIncompleted();
		}
		else
			nCheckTimes = 0;
		break;
	case TIMER_CHECKMEMORY:
		GetSystemMemoryInfo();
		SetTimer(TIMER_CHECKMEMORY, 1000 * 120, NULL);
		break;
	default:
		break;
	}
	CImageDlg::OnTimer(nIDEvent);
}

BEGIN_EVENTSINK_MAP(CWeChatPrinterDlg, CImageDlg)
//	ON_EVENT(CWeChatPrinterDlg, IDC_EXPLORER_MAIN, 102, CWeChatPrinterDlg::OnStatustextchangeExplorerMain, VTS_BSTR)
END_EVENTSINK_MAP()
/************************************************************************/
/*                            HOOK                                      */
/************************************************************************/
LRESULT CALLBACK OnMouseEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_LBUTTONDOWN)
	{
		static CPoint point;
		GetCursorPos(&point);
		g_CWeChatPrinterDlg->OnAdminEnter(point);
	}
	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

void CWeChatPrinterDlg::OnAdminEnter(CPoint point)
{
	const int nFalg = 150;
	CRect rect1(0, 0, nFalg, nFalg);
	CRect rect2(m_rc.Width() - nFalg, 0, m_rc.Width(), nFalg);
	CRect rect3(0, m_rc.Height() - nFalg, nFalg, m_rc.Height());
	CRect rect4(m_rc.Width() - nFalg, m_rc.Height() - nFalg, m_rc.Width(), m_rc.Height());
	if (rect1.PtInRect(point))
	{
		m_strAdminEnter += "1";
	}
	else if (rect2.PtInRect(point))
	{
		m_strAdminEnter += "2";
	}
	else if (rect3.PtInRect(point))
	{
		m_strAdminEnter += "3";
	}
	else if (rect4.PtInRect(point))
	{
		m_strAdminEnter += "4";
	}
	else
	{
		m_strAdminEnter = "";
	}

	TRACE("屏幕密码是：%s\n", m_strAdminEnter);
#ifdef EASY_PASSWD
	if (m_strAdminEnter.Find("1") >= 0) ::PostMessage(m_hWnd, WM_COMMAND, BTN_ADMIN_LOGOUT, NULL);
#else
	if (m_strAdminEnter.Find("33") >= 0) ::PostMessage(m_hWnd, WM_COMMAND, BTN_ADMIN_LOGOUT, NULL);
#endif // EASY_PASSWD
}

BOOL CWeChatPrinterDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	switch (wParam)
	{
	case BTN_ADMIN_LOGOUT:
	{
		{
		//	::SetWindowPos(GetSafeHwnd(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//最前端
			m_strAdminEnter = "";
			CAdmins m_admin;
			int nResult = m_admin.DoModal();
			if (nResult == 1)
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "OnCommand", "用户输入密码：33 后程序退出");
				EndDialog(TRUE);
			}
		//	::SetWindowPos(GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//最前端
		}
	}
	break;
	default:
		break;
	}
	return CImageDlg::OnCommand(wParam, lParam);
}

BOOL CWeChatPrinterDlg:: StartProcess3(CString strpath)
{
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "StartProcess3", "[%s]", "\"" + strpath + "\"");
 	BOOL b1 = StartProcess2("\"" + strpath + "\"", TRUE);//成功与失败都是返回1
//	BOOL b2 = StartProcess(strpath, "", 0, 0);
//  	if (b1 == FALSE)
//  	{
// 		BOOL b2 = StartProcess(strpath, "", 0, 0);
// 		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "StartProcess3", "StartProcess 失效[%d]，启动StartProcess2 [%d]", b1, b2);
//  	}
	return TRUE;
}

int UrlEncodeUtf8_(LPCSTR pszUrl, LPSTR pszEncode, int nEncodeLen)
{
	int nRes = 0;
	//定义变量
	wchar_t* pWString = NULL;
	char* pString = NULL;
	char* pResult = NULL;

	do
	{
		if (pszUrl == NULL)
			break;

		//先将字符串由多字节转换成UTF-8编码  
		int nLength = MultiByteToWideChar(CP_ACP, 0, pszUrl, -1, NULL, 0);

		//分配Unicode空间  
		pWString = new wchar_t[nLength + 1];
		if (pWString == NULL)
			break;

		memset(pWString, 0, (nLength + 1) * sizeof(wchar_t));
		//先转换成Unicode
		MultiByteToWideChar(CP_ACP, 0, pszUrl, -1, pWString, nLength);

		//分配UTF-8空间
		nLength = WideCharToMultiByte(CP_UTF8, 0, pWString, -1, NULL, 0, NULL, NULL);
		pString = new char[nLength + 1];
		if (pString == NULL)
			break;

		memset(pString, 0, nLength + 1);
		//Unicode转到UTF-8
		nLength = WideCharToMultiByte(CP_UTF8, 0, pWString, -1, pString, nLength, NULL, NULL);

		pResult = new char[nLength * 3];
		if (pResult == NULL)
			break;

		memset(pResult, 0, nLength * 3);
		char* pTmp = pResult;
		static char hex[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

		for (int i = 0; i < nLength; i++)
		{
			unsigned char c = pString[i];
			if (c == 0)
			{
				break;
			}

			if (c > 0x20 && c < 0x7f)// 数字或字母
			{
				*pTmp++ = c;
			}
			else if (c == 0x20)// 包含空格  
			{
				*pTmp++ = '%';
				*pTmp++ = hex[c / 16];
				*pTmp++ = hex[c % 16];
			}
			else// 进行编码
			{
				*pTmp++ = '%';
				*pTmp++ = hex[c / 16];
				*pTmp++ = hex[c % 16];
			}
		}
		nLength = strlen(pResult);
		nRes = nLength;
		if (pszEncode == NULL || nEncodeLen < nLength)
			break;

		memcpy(pszEncode, pResult, nLength);
	} while (0);

	if (pWString != NULL)
		delete[]pWString;

	if (pString != NULL)
		delete[]pString;

	if (pResult != NULL)
		delete[]pResult;

	return nRes;
}

BOOL HTTP_Download2(CString strURL, CString strFilePath, CString strUsername, CString strPassword, CString strProxyList)
{
	BOOL bRes = FALSE;
#define PATH_LEN 1024
	//建立http连接
	TCHAR szUserName[MAX_PATH] = { 0 };
	TCHAR szPassword[MAX_PATH] = { 0 };
	_tcscpy(szUserName, strUsername);
	_tcscpy(szPassword, strPassword);

	const TCHAR szHeaders[] = _T("Accept: */*\r\nUser-Agent:  Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)\r\n");    //协议

	CInternetSession    aInternetSession;        //一个会话
	CHttpConnection*    pHttpConnection = NULL;    //链接
	CHttpFile*          pHttpFile = NULL;
	DWORD               dwFileStatus;
	INTERNET_PORT       nPort;
	DWORD               dwServiceType;
	DWORD               dwDownloadSize = 0;
	CString             strServer;
	CString             strObject;
	CString				strDownloadFile = "";
	const int nBufferSize = 4096;
	TCHAR szURL[nBufferSize] = { 0 };
	UrlEncodeUtf8_(strURL, szURL, nBufferSize);
	strURL = szURL;

	//分解URL
	if (AfxParseURL(strURL, dwServiceType, strServer, strObject, nPort))
	{
		//如果服务类型是HTTP下载
		if (dwServiceType != AFX_INET_SERVICE_HTTP && dwServiceType != AFX_INET_SERVICE_HTTPS)
		{
			//返回成功
			return bRes;
		}
	}

	try
	{
		if (!strProxyList.IsEmpty())
		{
			INTERNET_PROXY_INFO proxyinfo;
			proxyinfo.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
			proxyinfo.lpszProxy = strProxyList;
			proxyinfo.lpszProxyBypass = NULL;
			aInternetSession.SetOption(INTERNET_OPTION_PROXY, (LPVOID)&proxyinfo, sizeof(INTERNET_PROXY_INFO));
		}

		pHttpConnection = aInternetSession.GetHttpConnection(strServer, nPort);
		do
		{
			//如果失败则线程退出
			if (pHttpConnection == NULL)
				break;

			//取得HttpFile对象
			pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,
				strObject,
				NULL,
				1,
				NULL,
				NULL,
				INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);

			if (pHttpFile == NULL)
				break;

			pHttpFile->SetOption(INTERNET_OPTION_PROXY_USERNAME, szUserName, _tcslen(szUserName) + 1);
			pHttpFile->SetOption(INTERNET_OPTION_PROXY_PASSWORD, szPassword, _tcslen(szPassword) + 1);

			if (!pHttpFile->AddRequestHeaders(szHeaders))//增加请求头
				break;

			if (!pHttpFile->SendRequest())//发送文件请求
				break;

			if (!pHttpFile->QueryInfoStatusCode(dwFileStatus))//查询文件状态
				break;

			if ((dwFileStatus / 100) * 100 != HTTP_STATUS_OK)//如果文件状态正常
				break;

			CFile aFile;
			int pos =strFilePath.ReverseFind('.');
			strDownloadFile = strFilePath.Left(pos) +".download";

			if (!aFile.Open(strDownloadFile, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
				break;

			DWORD dwFileLen;
			DWORD dwWordSize = sizeof(dwFileLen);
			pHttpFile->QueryInfo(HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH, &dwFileLen, &dwWordSize, NULL);

			do
			{
				BYTE szBuffer[PATH_LEN] = { 0 };
				DWORD dwLen = pHttpFile->Read(szBuffer, PATH_LEN);//接收数据
				if (dwLen == 0)
					break;

				aFile.Write(szBuffer, dwLen);
				dwDownloadSize += dwLen;
			} while (1);

			aFile.Close();
			DWORD dw = 0;
			if (InternetQueryDataAvailable((HINTERNET)(*pHttpFile), &dw, 0, 0) && (dw == 0))
				bRes = TRUE;//设置成功标志

			if (dwDownloadSize != dwFileLen)
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "HTTP_Download2", "实际下载dwDownloadSize =%ld，应下载dwFileLen=%ld，[%s]下载失败", dwDownloadSize,dwFileLen, strFilePath);
				bRes = FALSE;
			}

		} while (0);

		if (pHttpFile != NULL)
			delete pHttpFile;

		if (pHttpConnection != NULL)
			delete pHttpConnection;

		//关闭http连接
		aInternetSession.Close();
	

		rename(strDownloadFile, strFilePath);
	}
	//异常处理
	catch (CInternetException* e)
	{
		TCHAR   szCause[MAX_PATH] = { 0 };
		//取得错误信息
		e->GetErrorMessage(szCause, MAX_PATH);

		TRACE("internet exception:%s\n", szCause);//错误信息写入日志

		e->Delete();//删除异常对象

		if (pHttpFile != NULL)//删除http文件对象
			delete pHttpFile;

		if (pHttpConnection != NULL)//删除http连接对象
			delete pHttpConnection;

		aInternetSession.Close();//关闭http连接
	}
	catch (...)
	{
		if (pHttpFile != NULL)//删除http文件对象
			delete pHttpFile;

		if (pHttpConnection != NULL)//删除http连接对象
			delete pHttpConnection;

		aInternetSession.Close();//关闭http连接
	}

	return bRes;
}

//MFC下获取系统内存和当前进程的内存使用情况
void GetSystemMemoryInfo()
{
	CString strInfo;
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	DWORDLONG physical_memory = statex.ullTotalPhys / (1024 * 1024);
	DWORDLONG avalid_memory = statex.ullAvailPhys / (1024 * 1024);
	DWORDLONG virtual_totalmemory = statex.ullTotalVirtual / (1024 * 1024);
	DWORDLONG virtual_memory = statex.ullAvailVirtual / (1024 * 1024);
	DWORDLONG usePhys = physical_memory - avalid_memory;
	DWORDLONG useVirtual = virtual_totalmemory - virtual_memory;

	float percent_memory = ((float)usePhys / (float)physical_memory) * 100;
	float percent_memory_virtual = ((float)useVirtual / (float)virtual_totalmemory) * 100;
	strInfo.Format("物理内存使用率:%.2f%% 物理内存:%lld MB 可用物理内存：%lld MB\n", percent_memory, physical_memory, avalid_memory);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_MEMORY, "GetSystemMemoryInfo", "%s", strInfo);
	strInfo.Format("虚拟内存使用率:%.2f%% 虚拟内存:%lld MB 可用虚拟内存：%lld MB \n", percent_memory_virtual, virtual_totalmemory, virtual_memory);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_MEMORY, "GetSystemMemoryInfo", "%s", strInfo);

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	DWORD pid = GetCurrentProcessId();
	HANDLE handle;
	handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));

	int usedMemory = 0;

	PSAPI_WORKING_SET_INFORMATION workSet;
	memset(&workSet, 0, sizeof(workSet));
	BOOL bOk = QueryWorkingSet(handle, &workSet, sizeof(workSet));
	if (bOk || (!bOk && GetLastError() == ERROR_BAD_LENGTH))
	{
		int nSize = sizeof(workSet.NumberOfEntries) + workSet.NumberOfEntries * sizeof(workSet.WorkingSetInfo);
		char* pBuf = new char[nSize];
		if (pBuf)
		{
			QueryWorkingSet(handle, pBuf, nSize);
			PSAPI_WORKING_SET_BLOCK* pFirst = (PSAPI_WORKING_SET_BLOCK*)(pBuf + sizeof(workSet.NumberOfEntries));
			DWORD dwMem = 0;
			for (ULONG_PTR nMemEntryCnt = 0; nMemEntryCnt < workSet.NumberOfEntries; nMemEntryCnt++, pFirst++)
			{
				if (pFirst->Shared == 0)
				{
					dwMem += si.dwPageSize;
				}
			}
			delete pBuf;
			pBuf = NULL;
			if (workSet.NumberOfEntries > 0)
			{
				usedMemory = dwMem / (1024 * 1024);
			}
		}
	}
	strInfo.Format("进程id:%d 已使用内存 %d MB\n", pid, usedMemory);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_MEMORY, "GetSystemMemoryInfo", "%s", strInfo);
	CloseHandle(handle);

	//虚拟内存使用率 >85 或者  已使用内存 >1100 MB 就重启程序
	if (/*percent_memory_virtual >85 ||*/ usedMemory >1100)
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_MEMORY, "GetSystemMemoryInfo", "\n程序即将重启\n");
		RobotProgamme();
	}
}

void RobotProgamme()
{
	// TODO: 在此添加控件通知处理程序代码
	::PostMessage(AfxGetMainWnd()->m_hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
	//获取exe程序当前路径
	TCHAR szAppName[MAX_PATH];
	::GetModuleFileName(theApp.m_hInstance, szAppName, MAX_PATH);
	CString strAppFullName;
	strAppFullName.Format(_T("%s"), szAppName);
	//重启程序
	STARTUPINFO StartInfo;
	PROCESS_INFORMATION procStruct;
	memset(&StartInfo, 0, sizeof(STARTUPINFO));
	StartInfo.cb = sizeof(STARTUPINFO);
	::CreateProcess(
		(LPCTSTR)strAppFullName,
		NULL,
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&StartInfo,
		&procStruct);
}
