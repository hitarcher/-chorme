#include "stdafx.h"
#include "COMMFCN.h"

CConfig	g_Config;

UINT g_bLogType = LOGTYPE_DEBUG;
#define MAX_BUFF 1024
BOOL Automatic_log(BYTE bType, LPCTSTR strFile, LPCTSTR lpszFormat, ...)
{
	char* pBuf = (char*)malloc(MAX_BUFF+1);
	if(!pBuf) return FALSE;
	memset(pBuf, 0x00, MAX_BUFF+1);

	va_list argList;
	va_start(argList, lpszFormat);
	int iRet = _vsnprintf(pBuf, MAX_BUFF, lpszFormat, argList);
	if(iRet==-1)
	{
		char* pBuf2 = (char*)realloc(pBuf, 50*MAX_BUFF+1);
		memset(pBuf2, 0x00, 50*MAX_BUFF+1);
		if(pBuf2)
		{
			pBuf = pBuf2;
			_vsnprintf(pBuf, 50*MAX_BUFF, lpszFormat, argList);
		}
	}
	va_end(argList);

	BOOL bRet = FALSE;
	switch(g_bLogType & bType){
	case LOGTYPE_DEBUG:
		{
			CString strContent = "";
			strContent.Format("<<%s#%s>>	%s",GetCurTime(TIME_LONG,NULL).c_str(),LOG_MSG,pBuf);
			bRet = WriteFileEx(FILE_IMPORTANT, 
				GetPathLogFile(strFile,"").GetBuffer(0), strContent.GetBuffer(0), NULL);
		}
		break;
	case LOGTYPE_ERROR:
		{
			CString strContent = "";
			strContent.Format("<<%s#%s>>	%s",GetCurTime(TIME_LONG,NULL).c_str(),LOG_ERR,pBuf);
			bRet = WriteFileEx(FILE_IMPORTANT, 
				GetPathLogFile(strFile,"").GetBuffer(0), strContent.GetBuffer(0), NULL);
		}
		break;
	case LOGTYPE_SPECIAL:
		{
			CString strContent = "";
			strContent.Format("<<%s#%s>>	",GetCurTime(TIME_LONG,NULL).c_str(),pBuf);
			bRet = WriteFileEx(FILE_IMPORTANT, 
				GetPathLogFile(strFile,"").GetBuffer(0), strContent.GetBuffer(0), NULL);
		}
		break;
	default:
		break;
	}

	free(pBuf);
	return bRet;
}

BOOL Automatic_level(UINT uLevel)
{
	g_bLogType = uLevel;
	return TRUE;
}


CString ReadFileContent(const char* pFile)
{
	CStdioFile file;
	CString szLine = "";
	CString strText = "";
	if (!file.Open(pFile, CFile::modeRead))
	{
		return "";
	}
	//逐行读取字符串
	while( file.ReadString( szLine ) )
	{
		strText += szLine;
	}
	file.Close();
	return strText;
}


BOOL WriteFileContent(const char* pFile, CString strContent)
{
	CStdioFile file;
	CString szLine = "";
	CString strText = "";
	file.Open(pFile, CFile::modeWrite | CFile::modeCreate);
	file.Seek(0,CFile::end);
	file.WriteString(strContent);
	file.Close();
	return TRUE;
}


BOOL GetHttpMsg(CString strHttpAddr, CString& strHtml)
{
	CInternetSession session;
	CHttpFile *file = NULL;
	try
	{
		file = (CHttpFile*)session.OpenURL(strHttpAddr);
	}
	catch(CInternetException * m_pException)
	{
		file = NULL;
		m_pException->m_dwError;
		m_pException->Delete();
		session.Close();
		return FALSE;
	}
	CString strLine;
	if(file != NULL)
	{
		while(file->ReadString(strLine) != NULL)
		{
			strHtml += strLine;
		}
	}
	else
	{
		return FALSE;
	}
	session.Close();
	file->Close();
	delete file;
	file = NULL;
	return TRUE;
}


//给指定url发请求, 返回请求后的结果
CString SendURLPost(CString strServerName, CString strFormActionUrl, CString strPostStr)
{
	CString strFormData(strPostStr.GetBuffer(0));    // 需要提交的数据
	CInternetSession sess("HttpClient");

	//连结超时没效果的, 发送超时与接收超时有用, 连结超时可以用多线程WaitForSingleObject来实现
	sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 5000); // 5000毫秒的连接超时
	sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 5000); // 5000毫秒的发送超时
	sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 5000); // 5000毫秒的接收超时
	sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 5000); // 5000毫秒的发送超时
	sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 5000); // 5000毫秒的接收超时
	sess.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1); // 1次重试

	CString strHeaders = "Content-Type: application/x-www-form-urlencoded"; // 请求头
	CHttpConnection *pConnection = sess.GetHttpConnection(strServerName.GetBuffer(0));
	CHttpFile *pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, strFormActionUrl.GetBuffer(0));

	CString strSentence, strGetSentence = "";
	if(pFile)
	{
		BOOL result = pFile->SendRequest(strHeaders, (LPVOID)(LPCTSTR)strFormData, strFormData.GetLength());
		while(pFile->ReadString(strSentence))  // 读取提交数据后的返回结果
			strGetSentence = strGetSentence + strSentence + char(13) + char(10);
		pFile->Close();
		delete pFile;
		sess.Close();
		return strGetSentence;
	}
	return "";
}

//CString GetCurTime(TIME_TYPE nType)
//{
//	CString strTime = "";
//	CTime tmCur = CTime::GetCurrentTime();
//	switch(nType)
//	{
//	case DATE_LONG:
//		strTime = tmCur.Format("%Y-%m-%d %H:%M:%S");
//		break;
//	case DATE_LONG2:
//		strTime = tmCur.Format("%Y-%m-%d %H:%M:%S.000");
//		break;
//	case DAY_NORMAL:
//		strTime = tmCur.Format("%Y%m%d");
//		break;
//	case TIME_NORMAL:
//		strTime = tmCur.Format("%H%M%S");
//		break;
//	case DATE_NORMAL:
//		strTime = tmCur.Format("%Y%m%d%H%M%S");
//		break;
//	case DAY_SHORT:
//		strTime = tmCur.Format("%m%d");
//		break;
//	case DAY_LONG:
//		strTime = tmCur.Format("%Y-%m-%d");
//		break;
//	case TIME_LONG:
//		strTime = tmCur.Format("%H:%M:%S");
//		break;
//	case TIME_SHORT1:
//		strTime = tmCur.Format("%H:%M");
//		break;
//	case TIME_SHORT2:
//		strTime = tmCur.Format("%H %M");
//		break;
//	case MONTH_NORMAL:
//		strTime = tmCur.Format("%Y%m");
//		break;
//	case MONTH_LONG:
//		strTime = tmCur.Format("%Y年%m月%d日");
//		break;
//	case BANK_NORMAL:
//		strTime = tmCur.Format("%m%d%H%M%S");
//		break;
//	}
//	return strTime.GetBuffer(0);
//}


CString GetFileName(CString strFilePath)
{
	CString str = strFilePath;
	str.Replace("\\", "/");
	return str.Mid(str.ReverseFind('/')+1, str.GetLength());
}


