#pragma once
#include <WinDef.h>
#include <atlstr.h>
#include "CommonFun.h"
typedef void(CALLBACK*pFuncAddr)(int);

void CallerBackFun(pFuncAddr InstallCallBack);

class CAdbshell
{
public:
	CAdbshell(void);
	~CAdbshell(void);
public:
	//����adb shell������ɹ������#״̬������ʹ��RunCmdִ������
	BOOL Start(CString strCommand);

	BOOL StartCheck(CString strCommand);

	//����Ƿ�����豸
	BOOL CheckDevice(DWORD dwTimeOut);

	//��װAPK
	BOOL InstallApk(CString strCommand);

	//�������������adb shell״̬�µ��������������ж�������n�ָ�
	BOOL RunCmd(const CString&strCmdline);

	//�˳�shell����״̬���رս���
	BOOL Stop();

	//��ȡ������,����ǰ����ص���Stop�Ƚ���

	CString GetOutput();
	

public:
	BOOL Loop();
	BOOL LoopCheck();
	BOOL CheckDevice();
private:
	HANDLE	hThread;
	HANDLE	hThreadCheck;
	CString m_strOutput;
	//HANDLE	m_hProcess;

	//ָʾ�߳�׼���ã����Խ�������������ź���
	HANDLE	m_hEvent;
	HANDLE	m_hReadPipe;
	HANDLE	m_hReadPipe2;
	HANDLE	m_hWritePipe;
	HANDLE	m_hWritePipe2;

	HANDLE	m_hReadPipe3;
	HANDLE	m_hReadPipe4;
	HANDLE	m_hWritePipe3;
	HANDLE	m_hWritePipe4;
};