#include "stdafx.h"
#include "mycef.h"

#include "WeChatPrinter.h"	// �õ� theApp
#include "WeChatPrinterDlg.h" // �õ� maindlg
#include "Heads.h"

CefRefPtr<SimpleApp> m_cef_app;

int cef_init(CString m_strHtmlPath)
{
	CString strTemp = m_strHtmlPath;
	ConvertUtf8ToGBK(strTemp);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "׼��cef ��ʼ����·��Ϊ%s\n", strTemp);

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

	settings.remote_debugging_port = 33220;//����Զ�̵��Զ˿ڣ��������������򿪵��Դ��ڣ��ȽϷ���
	settings.multi_threaded_message_loop = true;//���ѡ���Ӱ�쵽��Ϣѭ����ģʽ����ϸѡ��
	settings.log_severity = LOGSEVERITY_ERROR;
	CefString(&settings.cache_path) = easytoUTF(get_fullpath("cef_catch"));
	CefString(&settings.log_file) = easytoUTF(get_fullpath("cef_catch//cef_log.log"));
	CefString(&settings.framework_dir_path) = easytoUTF(get_fullpath(""));
	CefString(&settings.resources_dir_path) = easytoUTF(get_fullpath("cef_Resources"));
	CefString(&settings.locales_dir_path) = easytoUTF(get_fullpath("cef_Resources\\locales"));


#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
#endif
	//	rect��SetWindowPos ���������λ�ã���SimpleApp ����� CefExecuteProcess ֮ǰ�������⣬����̵�����¡� 2022.1.13
	//	ʵ���벻����������Ӧ����ʲô���⣬�����󲿷ֶ�������ķֱ������Ļ�������Ȳ��ſ��ˡ�
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "���÷ֱ���");
	if (g_Config.m_bAoto)
	{
		maindlg->SetWindowPos(NULL, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "����ȫ��");
	}
	else
	{
		maindlg->SetWindowPos(NULL, g_Config.m_nPositionX, g_Config.m_nPositionY, g_Config.m_nPageWide, g_Config.m_nPageHigh, SWP_SHOWWINDOW);
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "���÷ֱ���Ϊ %dx%d", g_Config.m_nPageWide, g_Config.m_nPageHigh);
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
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "cef ��ʼ��ʧ��");
		return FALSE;
	}
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "cef_init", "cef ��ʼ���ɹ�");
	return TRUE;
}

void cef_close()
{
	/*edit by mingl : ������ģʽ�£�CefShutdown��������������Ƕ�������������������ǵ���������ֱ���˳����ù��������̣�����Ҳ���Բ��ù�*/
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
