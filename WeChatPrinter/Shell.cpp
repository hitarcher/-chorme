#include "stdafx.h"
#include "Shell.h"


pFuncAddr g_pFunAddr = NULL;

CShell::CShell(StdOutputCallBack callback)
{
	m_bExit = FALSE;
	m_hReadPipe = NULL;
	m_hWritePipe = NULL;
	m_hThread = NULL;
	StdOutput = callback;
	memset((char*)&m_pinfo, 0x00, sizeof(m_pinfo));
}

CShell::~CShell(void)
{
	Stop();
}

//启动adb shell
DWORD CShell::Start(CString strCommand)
{
	// 是否已经创建并运行
	if (m_hThread) return 999;
	m_bExit = FALSE;
	CString str_Error = "";
	// 获取本进程的标准输出
	HANDLE hTemp = GetStdHandle(STD_OUTPUT_HANDLE);
	// creat pip
	SECURITY_ATTRIBUTES sat;
	sat.nLength = sizeof(SECURITY_ATTRIBUTES);
	sat.bInheritHandle = TRUE;
	sat.lpSecurityDescriptor = NULL;
	if (CreatePipe(&m_hReadPipe, &m_hWritePipe, &sat, NULL) == FALSE) {
		str_Error = GetLastSysErr();
		LOG(LOGTYPE_ERROR, LOG_NAME_SHELL, "Start", "创建管道失败[%s]", str_Error);
		g_pFunAddr(0);
		return 2;
	}
	// creat process
	STARTUPINFO startupinfo;
	startupinfo.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&startupinfo);
	startupinfo.hStdError = m_hWritePipe;
	startupinfo.hStdOutput = m_hWritePipe;
	startupinfo.hStdInput = m_hReadPipe;
	startupinfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupinfo.wShowWindow = SW_HIDE;
	if (CreateProcess(NULL, strCommand.GetBuffer(0), NULL, NULL, TRUE, 0, NULL, NULL, &startupinfo, &m_pinfo) == FALSE) {
		str_Error = GetLastSysErr();
		LOG(LOGTYPE_ERROR, LOG_NAME_SHELL, "Start", "创建进程（adb）失败[%s]", str_Error);
		g_pFunAddr(1);
		return 3;
	}
	SetStdHandle(STD_OUTPUT_HANDLE, hTemp); // 恢复本进程的标准输出
	CloseHandle(m_hWritePipe); // 关闭写句柄//必须加这一句 ，否则卡死
	m_hWritePipe = NULL;

	// creat thread
	DWORD dwThread = FALSE;
	m_hThread = CreateThread(NULL, 0, ShellProc, this, 0, &dwThread);//这个参数你根据你的需要决定要不要
	if (m_hThread == NULL) {
		str_Error = GetLastSysErr();
		LOG(LOGTYPE_ERROR, LOG_NAME_SHELL, "Start", "创建线程失败[%s]", str_Error);
		g_pFunAddr(2);
		return 4;
	}
	return 0;
}

//退出shell命令状态，关闭进程。不能通过TerminateProcess方式结束，否则会有读取不全的情况
DWORD CShell::Stop()
{
	m_bExit = TRUE;

	// 杀掉进程
	if ( m_pinfo.hProcess!=NULL ) {
		TerminateProcess(m_pinfo.hProcess, -1);
		CloseHandle(m_pinfo.hProcess);
		m_pinfo.hProcess = NULL;
		CloseHandle(m_pinfo.hThread);
		memset((char*)&m_pinfo, 0x00, sizeof(m_pinfo));
	}
	// 关闭线程
	WaitForSingleObject(m_hThread, 1000);
	CloseHandle(m_hThread);
	m_hThread = NULL;

	return 0;
}

DWORD CShell::Waiting(DWORD dTime)
{
	if(m_hThread) WaitForSingleObject(m_hThread, dTime);
	return 0;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
DWORD CShell::ShellProc(LPVOID pParam)
{
	CShell* pDlg = (CShell*)pParam;
	return pDlg->ShellProcContent(pParam);
}

DWORD CShell::ShellProcContent(LPVOID pParam)
{
	CHAR buffer[1024] = { 0 };
	DWORD dwRead = 0;

	while (FALSE == m_bExit)
	{
		RtlZeroMemory(buffer, _countof(buffer));
		if (ReadFile(m_hReadPipe, buffer, _countof(buffer), &dwRead, NULL) == FALSE)
		{
			break;
		}
		else
		{
			StdOutput(buffer);
		}
	}
	// 关闭本线程
	CloseHandle(m_hThread);
	m_hThread = NULL;
	return TRUE;
}

void CallerBackFun(pFuncAddr InstallCallBack)
{
	if (InstallCallBack)
	{
		g_pFunAddr = InstallCallBack;
	}
}
