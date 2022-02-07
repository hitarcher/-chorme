#include "stdafx.h"
#include "mycef.h"

#include "WeChatPrinter.h"	// 用到 theApp
#include "WeChatPrinterDlg.h" // 用到 maindlg
#include "Heads.h"

CefRefPtr<SimpleApp> m_cef_app;

int cef_init(CString m_strHtmlPath)
{
	CString strTemp = m_strHtmlPath;
	ConvertUtf8ToGBK(strTemp);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "准备cef 初始化，路径为%s\n", strTemp);

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
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "CefExecuteProcess error_code = %d", exit_code);
		return FALSE;
	}

	CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
	command_line->InitFromString(::GetCommandLineW());
	CefSettings settings;
	if (command_line->HasSwitch("enable-chrome-runtime")) {
		settings.chrome_runtime = true;
	}

	settings.remote_debugging_port = 33220;//设置远程调试端口，可以在浏览器里打开调试窗口，比较方便
	settings.multi_threaded_message_loop = true;//这个选项会影响到消息循环的模式，仔细选择
	settings.log_severity = LOGSEVERITY_ERROR;
	CefString(&settings.cache_path) = easytoUTF(get_fullpath("cef_catch"));
	CefString(&settings.log_file) = easytoUTF(get_fullpath("cef_catch//cef_log.log"));
	CefString(&settings.framework_dir_path) = easytoUTF(get_fullpath(""));
	CefString(&settings.resources_dir_path) = easytoUTF(get_fullpath("cef_Resources"));
	CefString(&settings.locales_dir_path) = easytoUTF(get_fullpath("cef_Resources\\locales"));


#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif
	//	rect和SetWindowPos 必须在这个位置，在SimpleApp 后或者 CefExecuteProcess 之前会有问题，多进程的情况下。 2022.1.13
	//	实在想不起来，自适应会有什么问题，不过大部分都按照配的分辨率来的话，这个先不放开了。
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "设置分辨率");
	if (g_Config.m_bAoto)
	{
		maindlg->SetWindowPos(NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "设置全屏");
	}
	else
	{
		maindlg->SetWindowPos(NULL, g_Config.m_nPositionX, g_Config.m_nPositionY, g_Config.m_nPageWide, g_Config.m_nPageHigh, SWP_SHOWWINDOW);
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "设置分辨率为 %dx%d", g_Config.m_nPageWide, g_Config.m_nPageHigh);
	}
	CRect rect;
 	maindlg->GetWindowRect(&rect);

	//********************************************************************************

	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "new SimpleApp");
	m_cef_app =
		new SimpleApp(
			m_strHtmlPath.GetBuffer(0)
			/*"www.baidu.com"*/
			, maindlg->m_hWnd
			, rect
		);
	if (m_cef_app == NULL)
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "new SimpleApp Error");
		return FALSE;
	}
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "new SimpleApp OK");
	bool bret = CefInitialize(main_args, settings, m_cef_app.get(), sandbox_info);
	if (FALSE == bret)
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "cef 初始化失败");
		return FALSE;
	}
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "cef 初始化成功");
	return TRUE;
}

void cef_close()
{
	/*edit by mingl : 单进程模式下，CefShutdown会出现阻塞；但是多进程下运行正常；考虑到单进程下直接退出不用管其他进程，所以也可以不用管*/
	//CefShutdown();
	/*end by mingl*/
}

void cef_load_url(IN std::string _utf_url, IN int delay)
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

void cef_exec_js(IN std::string _utf_js, IN int delay)
{
	if (delay) Sleep(delay);
	if (m_cef_app == NULL) return cef_exec_js(_utf_js, 500);

	CefRefPtr<SimpleHandler> default_handler = (SimpleHandler*)(void*)m_cef_app->GetDefaultClient();
	if (default_handler == NULL) return cef_exec_js(_utf_js, 500);

	CefRefPtr<CefBrowser> default_browser = default_handler->GetBrowser();
	if (default_browser == NULL) return cef_exec_js(_utf_js, 500);

	CefRefPtr<CefFrame> default_frame = default_browser->GetMainFrame();
	if (default_frame == NULL) return cef_exec_js(_utf_js, 500);
	if (default_frame->IsValid() == false)return cef_exec_js(_utf_js, 500);
	if (default_frame->GetURL().empty() == true) return cef_exec_js(_utf_js, 500);
	default_frame->ExecuteJavaScript(CefString(_utf_js.c_str()), default_frame->GetURL(), 0);
}
