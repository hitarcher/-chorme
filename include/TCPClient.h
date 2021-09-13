
#if !defined(AFX_TCPCLIENT_H__4834B9CB_1749_4DCA_8C54_2F5DCF9A4A03__INCLUDED_)
#define AFX_TCPCLIENT_H__4834B9CB_1749_4DCA_8C54_2F5DCF9A4A03__INCLUDED_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TCPCLIENT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
//  functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#include <winsock2.h>
#include <winbase.h>
#include <afxtempl.h>


#define	UM_CLIENT_MSG		(WM_USER + 300)
#define UM_SERVER_MSG		(WM_USER + 301)

#define SOCKET_TIMEOUT		120000
#define WAITMSG_TIMEOUT		150000

//消息包类型定义
#define MSG_SOCKET_ERROR		1			//消息错误

#include "BasePack.h"

// This class is exported from the TCPClient.dll
class  CTCPClient
{
public:
	CTCPClient();
	virtual ~CTCPClient();
	
public:
	

public:
	const char*	GetLastError();
	BOOL	Init(HWND hWnd, int nMsgType = USE_BASE_TYPE_HEADER);
	void	SetRecvWnd(HWND hWnd);
	BOOL	ConnectToServer(LPCTSTR lpszIP, u_short nPort);
	BOOL	AddSendMsg(LPBASE_MSG_PACK pMsg);
	LPBASE_MSG_PACK	GetMsgPack();
	void	DisconnectServer();
	void	SetSocketTimeOut(int nTime);
	//返回值>0未连接服务器或连接服务器失败，=0成功，其他发送或接受消息失败
	short	ExecuteSendAndRecv(LPCTSTR lpszIP, u_short nPort, const LPBASE_MSG_PACK pSendPack, LPBASE_MSG_PACK* pRecvPack, int nMsgType = USE_BASE_TYPE_HEADER);

	virtual	int		GetHeaderSize(int nType);					//获得包头大小，必须填写此函数
	virtual UINT	GetPackSize(char* szHeaderPack, int nType);	//获得包的大小，包括包头长度。必须填写此函数
	void	SafeDeletePack(LPBASE_MSG_PACK* ppPack);
	
	BOOL	WaitGetMsgEvent();

protected:
	int		m_nClientSocket;
	HWND	m_hWnd;
	BOOL	m_bIsExitThread;
	BOOL	m_bRecvExit;
	BOOL	m_bExecuteExit;
	BOOL	m_bServerClosed;
	BOOL	m_bGetMsgSelf;
	HANDLE	m_hRecvMsgEvent;
	HANDLE	m_hSendMsgEvent;
	HANDLE	m_hGetMsgEvent;
	HANDLE	m_hRecvMsgThread;
	HANDLE	m_hSendMsgThread;
	HANDLE	m_hExecuteMsgThread;
	CRITICAL_SECTION	m_SendSection;
	CRITICAL_SECTION	m_RecvSection;
	CList<LPBASE_MSG_PACK, LPBASE_MSG_PACK>	m_lspRecvMsg;
	CList<LPBASE_MSG_PACK, LPBASE_MSG_PACK>	m_lspSendMsg;
	int		m_nTime;
	CString	m_strLastError;
	int		m_nSocketType;
	
protected:
	static DWORD WINAPI	RecvThreadProc(LPVOID pParam);
	static DWORD WINAPI SendThreadProc(LPVOID pParam);
	static DWORD WINAPI	ExecuteThreadProc(LPVOID pParam);

	BOOL	ConnectToServer(sockaddr_in *pAddr);
	BOOL	ConnectToServer(sockaddr_in *pAddr, int iTimeOut);
	LPBASE_MSG_PACK	GetRecvMsg();
	virtual BOOL	GetServerAddr(sockaddr_in *pAddr, LPCTSTR lpszServerName, LPCTSTR lpszIP, u_short nPort);
	void	AddErrorMsg(const char* szError);
	CString	ConvertToHString(const char* szData, int nLen);
	short	StrCopy(char* szDest, const char* szSrc, int nDestMaxLen);
	void	WriteLogFile(LPCTSTR lpszData);
	
	virtual DWORD	RecvThreadContent();
	virtual DWORD	SendThreadContent();
	virtual DWORD	ExecuteThreadContent();

	virtual	void	AddRecvMsg(LPBASE_MSG_PACK pMsg);
	virtual	void	ExitThread();
	virtual	void	ServerClosed();
	virtual void	ResetParam();

public:
	
};

// backup
//int CTCPClient::GetHeaderSize(int nType)	//获得包头大小，必须填写此函数
//{
//	if (nType == USE_BASE_TYPE_HEADER)
//	{
//		return sizeof(BASE_MSG);
//	}
//	else if (nType == USE_UNION_HEADER)
//	{
//		return 2;
//	}
//	else if (nType == USE_STRING_HEADER)
//	{
//		return m_uStrHeadLen;
//	}
//	else
//	{
//		return sizeof(BASE_MSG);
//	}
//}
//
//UINT CTCPClient::GetPackSize(char* szHeaderPack, int nType)	//获得包的大小，包括包头长度。必须填写此函数
//{
//	if (nType == USE_BASE_TYPE_HEADER)
//	{
//		LPBASE_MSG pMsg = (LPBASE_MSG)szHeaderPack;
//		return pMsg->uSize;
//	}
//	else if (nType == USE_UNION_HEADER)
//	{
//		unsigned char cH = (unsigned char)szHeaderPack[0];
//		unsigned char cL = (unsigned char)szHeaderPack[1];
//		return cH * 256 + cL + 2;
//	}
//	else if (nType == USE_STRING_HEADER)
//	{
//		char szLen[128] = { 0 };
//		memcpy(szLen, szHeaderPack, m_uStrHeadLen);
//		return m_bstringInclude ? (atoi(szLen)) : (atoi(szLen) + m_uStrHeadLen);
//	}
//	else
//	{
//		LPBASE_MSG pMsg = (LPBASE_MSG)szHeaderPack;
//		return pMsg->uSize;
//	}
//}

#endif