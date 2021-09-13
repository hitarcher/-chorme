#pragma once
#include <WinDef.h>
#include <atlstr.h>
#include "LOG2.H"
#include "CommonFun.h"

typedef void (CALLBACK *StdOutputCallBack)(const char* pOutPut);
typedef void(CALLBACK*pFuncAddr)(int);

void CallerBackFun(pFuncAddr InstallCallBack);

class CShell
{
public:
	CShell(StdOutputCallBack callback);
	~CShell(void);

public:
	//启动adb shell 返回1 表示busy状态
	DWORD Start(CString strCommand);

	//退出shell命令状态，关闭进程
	DWORD Stop();

	// wait
	DWORD Waiting(DWORD dTime = INFINITE);

private:
	static DWORD WINAPI	ShellProc(LPVOID pParam);
	DWORD WINAPI ShellProcContent(LPVOID pParam);


private:
	HANDLE	m_hThread;
	PROCESS_INFORMATION m_pinfo;
	StdOutputCallBack StdOutput;

	BOOL m_bExit;

	HANDLE	m_hReadPipe;
	HANDLE	m_hWritePipe;
};

