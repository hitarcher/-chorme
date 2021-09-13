#include "stdafx.h"
#include "HttpClient.h"


CHttpClient::CHttpClient(void)
{
}


CHttpClient::~CHttpClient(void)
{
}

/*************************类实现********************************/  
// 添加字段  
void CHttpClient::addParam(CString name, CString value)    
{    
    names.AddTail((LPCTSTR)name);    
    values.AddTail((LPCTSTR)value);    
    CString strEquel = "=";    
    CString strAnd = "&";    
    CONTENT = CONTENT + name + strEquel + value + strAnd;    
    CL = CONTENT.GetLength();    
}   

// 以http Get方式请求URL  
CString CHttpClient::doGet(CString   href)    
{    
    CString httpsource = "";    
    CInternetSession session1(NULL,0);    
    CHttpFile* pHTTPFile=NULL;    
    try
	{    
        pHTTPFile=(CHttpFile*)session1.OpenURL(href);    
        //session1.    
    }  
    catch(...)
    {    
        pHTTPFile=NULL;    
    }    
    if(pHTTPFile)    
    {    
        CString text;
        for(int i=0;pHTTPFile->ReadString(text);i++)    
        {    
            httpsource=httpsource+text+"\r\n";    
        }    
        pHTTPFile->Close();    
        delete pHTTPFile;  
		return   httpsource;  
    }
	return "";   
} 

// 以Http Post方式请求URL  
CString CHttpClient::doPost(CString   href)    
{    
    CString httpsource="";    
    CInternetSession session1;    
    CHttpConnection* conn1=NULL;    
    CHttpFile* pFile = NULL;        
    CString strServerName;    
    CString strObject;    
    INTERNET_PORT   nPort;    
    DWORD dwServiceType;    
    AfxParseURL((LPCTSTR)href, dwServiceType, strServerName, strObject, nPort);    
    DWORD retcode;                
    char* outBuff = CONTENT.GetBuffer(1000);    
    try        
    {                    
        conn1 = session1.GetHttpConnection(strServerName, nPort);            
        pFile = conn1->OpenRequest(0,strObject,NULL,1,NULL,"HTTP/1.1",INTERNET_FLAG_EXISTING_CONNECT|INTERNET_FLAG_NO_AUTO_REDIRECT);            
        pFile->AddRequestHeaders("Content-Type:   application/x-www-form-urlencoded");            
        pFile->AddRequestHeaders("Accept:   */*");                            
        pFile->SendRequest(NULL,0,outBuff,strlen(outBuff)+1);                    
        pFile->QueryInfoStatusCode(retcode);                    
    }                              
    catch(CInternetException* e)
	{
		return "RequestTimedOut";
	};                    
    if(pFile)    
    {    
        CString text;    
        for(int i=0;pFile->ReadString(text);i++)    
        {    
            httpsource=httpsource+text+"\r\n";    
        }    
        pFile->Close();
		delete pFile;            
		delete conn1;            
		session1.Close(); 
		return httpsource; 
    }
	return "";
}    
  