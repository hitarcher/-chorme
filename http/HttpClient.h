#pragma once
/************************Http: Get、Post的封装类***************/  

#include   "wininet.h"    
#include   "afxinet.h"  

/*********************类定义***********************************/  
class   CHttpClient        
{    
public:    
	CHttpClient(void);
	~CHttpClient(void);

	void addParam(CString name, CString value);    
	CString doPost(CString href);    
	CString doGet(CString href);     
private:    
	CString CONTENT;    
	int CL;    
	CStringList values;    
	CStringList names;    
};   

