#include "stdafx.h"
#include "Adbshell.h"

pFuncAddr g_pFunAddr = NULL;
DWORD g_dwTimeOut = 20*1000;
DWORD g_dwBeginTime = 0;
BOOL g_bLoopEnd = TRUE;
BOOL g_bChkOrIns = TRUE;
BOOL g_bChkDevice = FALSE;
BOOL g_bCheckBusy = FALSE;


DWORD __stdcall ThreadAdbshellProc(void *pVoid)
{
	if (pVoid != NULL) {
		CAdbshell *pShell = (CAdbshell *)pVoid;
		pShell->Loop();
	}
	return 0;
}

DWORD __stdcall ThreadCheckProc(void *pVoid)
{
	if (pVoid != NULL) {
		CAdbshell *pShell = (CAdbshell *)pVoid;
		pShell->LoopCheck();
	}
	return 0;
}

DWORD __stdcall CheckDeviceProc(void *pVoid)
{
	if (pVoid != NULL) {
		CAdbshell *pShell = (CAdbshell *)pVoid;
		pShell->CheckDevice();
	}
	return 0;
}

CAdbshell::CAdbshell(void)
{
	m_hEvent = NULL;
	//m_hProcess = NULL;
}

CAdbshell::~CAdbshell(void)
{
}

//启动adb shell，如果成功则进入#状态，可以使用RunCmd执行命令
BOOL CAdbshell::Start(CString strCommand)
{
	m_strOutput.Empty();
	SECURITY_ATTRIBUTES sat;
	STARTUPINFO startupinfo;
	PROCESS_INFORMATION pinfo;

	sat.nLength = sizeof(SECURITY_ATTRIBUTES);
	sat.bInheritHandle = TRUE;
	sat.lpSecurityDescriptor = NULL;
	if (CreatePipe(&m_hReadPipe, &m_hWritePipe, &sat, NULL) == FALSE) {
		return FALSE;
	}
	if (CreatePipe(&m_hReadPipe2, &m_hWritePipe2, &sat, NULL) == FALSE) {
		return FALSE;
	}
	startupinfo.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&startupinfo);
	startupinfo.hStdError = m_hWritePipe;
	startupinfo.hStdOutput = m_hWritePipe;
	startupinfo.hStdInput = m_hReadPipe2;
	startupinfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupinfo.wShowWindow = SW_HIDE;
	if (CreateProcess(NULL, strCommand.GetBuffer(0), NULL, NULL, TRUE, 0, NULL, NULL, &startupinfo, &pinfo) == FALSE)
	{
		//DWORD dwError = GetLastError();
		return FALSE;
	}
	CloseHandle(m_hWritePipe);
	CloseHandle(m_hReadPipe2);
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	//m_hProcess = pinfo.hProcess;

	DWORD dwThread = FALSE;
	hThread = CreateThread(NULL, 0, ThreadAdbshellProc, this, 0, &dwThread);//这个参数你根据你的需要决定要不要
	if (hThread == NULL)
	{
		return FALSE;
	}
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	return TRUE;
}

BOOL CAdbshell::StartCheck(CString strCommand)
{
	m_strOutput.Empty();
	SECURITY_ATTRIBUTES sat;
	STARTUPINFO startupinfo;
	PROCESS_INFORMATION pinfo;

	sat.nLength = sizeof(SECURITY_ATTRIBUTES);
	sat.bInheritHandle = TRUE;
	sat.lpSecurityDescriptor = NULL;
	if (CreatePipe(&m_hReadPipe3, &m_hWritePipe3, &sat, NULL) == FALSE) {
		return FALSE;
	}
	if (CreatePipe(&m_hReadPipe4, &m_hWritePipe4, &sat, NULL) == FALSE) {
		return FALSE;
	}
	startupinfo.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&startupinfo);
	startupinfo.hStdError = m_hWritePipe3;
	startupinfo.hStdOutput = m_hWritePipe3;
	startupinfo.hStdInput = m_hReadPipe4;
	startupinfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupinfo.wShowWindow = SW_HIDE;
	if (CreateProcess(NULL, strCommand.GetBuffer(0), NULL, NULL, TRUE, 0, NULL, NULL, &startupinfo, &pinfo) == FALSE)
	{
		//DWORD dwError = GetLastError();
		return FALSE;
	}
	CloseHandle(m_hWritePipe3);
	CloseHandle(m_hReadPipe4);
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	//m_hProcess = pinfo.hProcess;

	DWORD dwThread = FALSE;
	hThreadCheck = CreateThread(NULL, 0, ThreadCheckProc, this, 0, &dwThread);//这个参数你根据你的需要决定要不要
	if (hThreadCheck == NULL)
	{
		return FALSE;
	}
	return TRUE;
}
//这里输入的是在adb shell状态下的批处理命令，如果有多条请以n分隔
BOOL CAdbshell::RunCmd(const CString&strCmdline)
{
	CHAR bufferReadFile[1024] = {0};
	DWORD dwReadFile = 0;
	BOOL bSuccess = TRUE;
	CString strOneCmd;
	int nPos1 = 0;
	int nPos2 = 0;
	DWORD dwWrite = 0;
	if (strCmdline.GetLength() < 2)
	{
		return FALSE;
	}
	//先等待线程函数准备好
	WaitForSingleObject(m_hEvent, INFINITE);
	while (TRUE)
	{
		nPos1 = nPos2;
		nPos2 = strCmdline.Find('n', nPos1);
		if (nPos2 == -1)
		{
			nPos2 = strCmdline.GetLength();
		}
		strOneCmd = strCmdline.Mid(nPos1, nPos2 - nPos1).Trim();
		//命令长度至少为2
		if (strOneCmd.GetLength() >= 2)
		{
			strOneCmd += "rn";
			//执行
			bSuccess = WriteFile(m_hWritePipe2, strOneCmd, strOneCmd.GetLength(), &dwWrite, NULL);
		}
		++nPos2;
		if (nPos2 >= strCmdline.GetLength())
		{
			break;		
		}
	}
	return bSuccess;
}

//退出shell命令状态，关闭进程。不能通过TerminateProcess方式结束，否则会有读取不全的情况
BOOL CAdbshell::Stop()
{
	RunCmd("exitnexit");
	WaitForSingleObject(hThread, INFINITE);
	return TRUE;
	//if ( m_hProcess!=NULL ) {
	//	TerminateProcess(m_hProcess, -1);
	//	CloseHandle(m_hProcess);
	//	m_hProcess = NULL;
	//}

	//return TRUE;
}
//读取输出结果,调用前请务必调用Stop先结束
CString CAdbshell::GetOutput()
{
	WaitForSingleObject(hThread, INFINITE);

	//两个exit要去掉
	int nPos1 = 0;
	//int nPos2 = 0;

	nPos1 = m_strOutput.Find("# exit");
	if (nPos1 == -1) {
		nPos1 = m_strOutput.Find("$ exit");
	}
	if (nPos1 != -1) {
		m_strOutput = m_strOutput.Left(nPos1);
		nPos1 = m_strOutput.ReverseFind('n');
		m_strOutput = m_strOutput.Left(nPos1);
	}
	return m_strOutput;
}

BOOL CAdbshell::Loop()
{
	CHAR buffer[1024] = { 0 };
	DWORD dwRead = 0;
	while (TRUE)
	{
		RtlZeroMemory(buffer, _countof(buffer));
		SetEvent(m_hEvent);
		if (ReadFile(m_hReadPipe, buffer, _countof(buffer), &dwRead, NULL) == FALSE)
		{
			break;
		}
		else
		{
			m_strOutput += buffer;
			if (g_pFunAddr)
			{
				CString str_Result = "";
				str_Result.Format("%s", buffer);
				{
					if (-1 != str_Result.Find("Failure")) // 没找到
					{
						int nRet = str_Result.Find("INSTALL_CANCELED_BY_USER");
						if (-1 != nRet)
						{
							g_pFunAddr(1);
						}
						else if (-1 != str_Result.Find("INSTALL_FAILED_ALREADY_EXISTS"))
						{
							g_pFunAddr(2);
						}
						else
						{
							g_pFunAddr(3);
						}
						break;
					}
					if (-1 != str_Result.Find("Success"))
					{
						g_pFunAddr(0);
						break;
					}
				}
			}
		}
	}
	CloseHandle(m_hReadPipe);
	CloseHandle(m_hWritePipe2);
	CloseHandle(m_hEvent);
	m_hEvent = NULL;
	CloseHandle(hThread);
	hThread = NULL;
	return TRUE;
}

BOOL CAdbshell::LoopCheck()
{
	CHAR buffer[1024] = { 0 };
	DWORD dwRead = 0;
	while (TRUE)
	{
		RtlZeroMemory(buffer, _countof(buffer));
		if (ReadFile(m_hReadPipe3, buffer, _countof(buffer), &dwRead, NULL) == FALSE)
		{
			break;
		}
		else
		{
			if (g_pFunAddr)
			{
				CString str_Result = "";
				str_Result.Format("%s", buffer);
				int nIndex = str_Result.Find("List of devices attached");
				if (-1 != nIndex)
				{
					CString strTmp = str_Result.Mid(nIndex + 24, str_Result.GetLength() - nIndex - 24);
					if (-1 != strTmp.Find("device"))
					{
						/// 找到设备
						g_pFunAddr(10);
						g_bChkDevice = TRUE;
						break;
					}
				}
				g_bChkDevice = FALSE;

			}
		}
	}
	CloseHandle(m_hReadPipe3);
	CloseHandle(m_hWritePipe4);
	CloseHandle(m_hEvent);
	m_hEvent = NULL;
	CloseHandle(hThreadCheck);
	hThreadCheck = NULL;
	g_bLoopEnd = TRUE;
	return TRUE;
}

BOOL CAdbshell::CheckDevice()
{
	while (GetTickCount()- g_dwBeginTime <= g_dwTimeOut)
	{
		Sleep(50);
		if (g_bChkDevice) break;
		if (g_bLoopEnd)
		{
			StartCheck("\"" + GetFullPath("adb\\adb.exe") + "\" devices ");
			g_bLoopEnd = FALSE;
		}
			
	}
	if (FALSE == g_bChkDevice)
	{
		/// 检测设备超时
		g_pFunAddr(11);
	}
	g_bCheckBusy = FALSE;
	return TRUE;
}

BOOL CAdbshell::CheckDevice(DWORD dwTimeOut)
{

	if (g_bCheckBusy)
	{
		return FALSE;
	}
	g_dwTimeOut = dwTimeOut;
	g_dwBeginTime = GetTickCount();
	g_bCheckBusy = TRUE;
	g_bChkDevice = FALSE;
	DWORD dwThread = FALSE;
	hThread = CreateThread(NULL, 0, CheckDeviceProc, this, 0, &dwThread);
	if (hThread == NULL)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CAdbshell::InstallApk(CString strCommand)
{
	Start(strCommand);
	return TRUE;
}

void CallerBackFun(pFuncAddr InstallCallBack)
{
	if (InstallCallBack)
	{
		g_pFunAddr = InstallCallBack;
	}
}
