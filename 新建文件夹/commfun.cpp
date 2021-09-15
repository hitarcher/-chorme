#include "StdAfx.h"
#include "commfun.h"
#include <tlhelp32.H>
#include <Nb30.h>
#include <algorithm>
#pragma comment(lib, "Netapi32.lib")
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")


/************************************************************/
/*	通过注册表获取端口列表									
/************************************************************/
bool GetComPorts(CStringArray* pPortList)
{
	long lReg;  
	HKEY hKey;  
	DWORD MaxValueLength;  
	DWORD dwValueNumber;  
	CString str2[255];
	int comNo = 0;
	
	lReg = RegOpenKeyExA(HKEY_LOCAL_MACHINE,"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE,&hKey);
	if(lReg != ERROR_SUCCESS) return false;
	
	lReg = RegQueryInfoKeyA(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
		&dwValueNumber,	//返回和hKey关联，有效值的个数
		&MaxValueLength,	//返回有效值中最长键名的字节数，不包含null
		NULL, NULL, NULL);
	if(lReg!=ERROR_SUCCESS) 
	{
		RegCloseKey(hKey);
		return false;
	}
	
	LPSTR pValueName, pCOMNumber; 
	DWORD cchValueName,dwValueSize=20;  
	for(comNo = 0; comNo < (int)dwValueNumber; comNo++)  
	{  
		cchValueName = MaxValueLength+1;  
		dwValueSize = 20;  
		pValueName = (LPSTR)VirtualAlloc(NULL,cchValueName,MEM_COMMIT,PAGE_READWRITE); 
		if(!pValueName) 
		{
			RegCloseKey(hKey);
			return false;
		}
		
		lReg = RegEnumValueA(hKey, comNo, pValueName,  //key name
			&cchValueName,  //key number
			NULL, NULL, NULL, NULL);  
		if((lReg != ERROR_SUCCESS)&&(lReg != ERROR_NO_MORE_ITEMS)) 
		{
			VirtualFree(pValueName,0,MEM_RELEASE); 
			RegCloseKey(hKey);
			return false;
		}
		
		pCOMNumber = (LPSTR)VirtualAlloc(NULL,6,MEM_COMMIT,PAGE_READWRITE);
		if(!pCOMNumber)
		{
			VirtualFree(pValueName,0,MEM_RELEASE); 
			RegCloseKey(hKey);
			return false;
		}
		
		lReg = RegQueryValueExA(hKey, pValueName, NULL, NULL, (LPBYTE)pCOMNumber, &dwValueSize);
		if((lReg == ERROR_MORE_DATA) || (lReg != ERROR_SUCCESS))
		{
			VirtualFree(pValueName,0,MEM_RELEASE); 
			VirtualFree(pCOMNumber,0,MEM_RELEASE);
			RegCloseKey(hKey);
			return false;
		}
		str2[comNo] = CString(pCOMNumber);
		VirtualFree(pValueName,0,MEM_RELEASE); 
		VirtualFree(pCOMNumber,0,MEM_RELEASE);
	}
	RegCloseKey(hKey);
	
	//排列
	if(comNo > 1)
	{
		CString min_com(_T("COM255"));
		
		for(int i = 0; i < comNo; i++)
		{
			for(int j = i + 1; j < comNo ; j++)
			{
				if(str2[i].Compare(str2[j]) > 0)
				{
					min_com = str2[j];
					str2[j] = str2[i];
					str2[i] = min_com;
				}
			}
		}
	}
	
	for(int i = 0; i < comNo; i++)
	{
		pPortList->Add(str2[i]);
	}
	return true;
}

CString GetCurTime(TIME_TYPE nType)
{
	CString strTime = "";
	CTime tmCur = CTime::GetCurrentTime();
	switch(nType)
	{
	case DATE_LONG:
		strTime = tmCur.Format("%Y-%m-%d %H:%M:%S");
		break;
	case DATE_LONG2:
		strTime = tmCur.Format("%Y-%m-%d %H:%M:%S.000");
		break;
	case DAY_NORMAL:
		strTime = tmCur.Format("%Y%m%d");
		break;
	case TIME_NORMAL:
		strTime = tmCur.Format("%H%M%S");
		break;
	case DATE_NORMAL:
		strTime = tmCur.Format("%Y%m%d%H%M%S");
		break;
	case DAY_SHORT:
		strTime = tmCur.Format("%m%d");
		break;
	case DAY_LONG:
		strTime = tmCur.Format("%Y-%m-%d");
		break;
	case TIME_LONG:
		strTime = tmCur.Format("%H:%M:%S");
		break;
	case TIME_SHORT1:
		strTime = tmCur.Format("%H:%M");
		break;
	case TIME_SHORT2:
		strTime = tmCur.Format("%H %M");
		break;
	case MONTH_NORMAL:
		strTime = tmCur.Format("%Y%m");
		break;
	case MONTH_LONG:
		strTime = tmCur.Format("%Y年%m月%d日");
		break;
	case BANK_NORMAL:
		strTime = tmCur.Format("%m%d%H%M%S");
		break;
	}
	return strTime.GetBuffer(0);
}

BOOL SetPrivilege()
{
	TOKEN_INFORMATION_CLASS info;
	info = TokenPrivileges;
	TOKEN_PRIVILEGES privileges;
	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		TRACE("OpenProcessToken Error %u\n", GetLastError());
		return FALSE;
	}
	LUID luid;
	if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid))
	{
		TRACE("LookupPrivilegeValue Error %u\n", GetLastError());
		return FALSE;
	}
	privileges.PrivilegeCount = 1;
	privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	privileges.Privileges[0].Luid = luid;
	AdjustTokenPrivileges(hToken, FALSE, &privileges, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
	if (::GetLastError() != ERROR_SUCCESS) 
	{ 
		TRACE("AdjustTokenPrivileges failed: %u\n", ::GetLastError());
		return FALSE;
	}
	return TRUE;
}

/************************************************************/
/*	通过注册表获取并口列表									
/************************************************************/
bool GetLPTPorts(CStringArray* pPortList)
{
	long lReg;  
	HKEY hKey;  
	DWORD MaxValueLength;  
	DWORD dwValueNumber;  
	CString str2[255];
	int comNo = 0;
	
	lReg = RegOpenKeyExA(HKEY_LOCAL_MACHINE,"HARDWARE\\DEVICEMAP\\PARALLEL_PORTS", 0, KEY_QUERY_VALUE,&hKey);
	if(lReg != ERROR_SUCCESS) return false;
	
	lReg = RegQueryInfoKeyA(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
		&dwValueNumber,	//返回和hKey关联，有效值的个数
		&MaxValueLength,	//返回有效值中最长键名的字节数，不包含null
		NULL, NULL, NULL);
	if(lReg!=ERROR_SUCCESS) 
	{
		RegCloseKey(hKey);
		return false;
	}
	
	LPSTR pValueName, pCOMNumber; 
	DWORD cchValueName,dwValueSize=20;  
	for(comNo = 0; comNo < (int)dwValueNumber; comNo++)  
	{  
		cchValueName = MaxValueLength+1;  
		dwValueSize = 20;  
		pValueName = (LPSTR)VirtualAlloc(NULL,cchValueName,MEM_COMMIT,PAGE_READWRITE); 
		if(!pValueName) 
		{
			RegCloseKey(hKey);
			return false;
		}
		
		lReg = RegEnumValueA(hKey, comNo, pValueName,  //key name
			&cchValueName,  //key number
			NULL, NULL, NULL, NULL);  
		if((lReg != ERROR_SUCCESS)&&(lReg != ERROR_NO_MORE_ITEMS)) 
		{
			VirtualFree(pValueName,0,MEM_RELEASE); 
			RegCloseKey(hKey);
			return false;
		}
		
		pCOMNumber = (LPSTR)VirtualAlloc(NULL,6,MEM_COMMIT,PAGE_READWRITE);
		if(!pCOMNumber)
		{
			VirtualFree(pValueName,0,MEM_RELEASE); 
			RegCloseKey(hKey);
			return false;
		}
		
		lReg = RegQueryValueExA(hKey, pValueName, NULL, NULL, (LPBYTE)pCOMNumber, &dwValueSize);
		if((lReg == ERROR_MORE_DATA) || (lReg != ERROR_SUCCESS))
		{
			VirtualFree(pValueName,0,MEM_RELEASE); 
			VirtualFree(pCOMNumber,0,MEM_RELEASE);
			RegCloseKey(hKey);
			return false;
		}
		str2[comNo] = CString(pCOMNumber);
		VirtualFree(pValueName,0,MEM_RELEASE); 
		VirtualFree(pCOMNumber,0,MEM_RELEASE);
	}
	RegCloseKey(hKey);
	
	//排列
	if(comNo > 1)
	{
		CString min_com(_T("\\DosDevices\\LPT255"));
		
		for(int i = 0; i < comNo; i++)
		{
			for(int j = i + 1; j < comNo ; j++)
			{
				if(str2[i].Compare(str2[j]) > 0)
				{
					min_com = str2[j];
					str2[j] = str2[i];
					str2[i] = min_com;
				}
			}
		}
	}
	
	for(int i = 0; i < comNo; i++)
	{
		pPortList->Add(str2[i]);
	}
	return true;
}

CString GetYesterdayTime(TIME_TYPE nType)
{
	CString strTime = "";
	CTime tmCur = CTime::GetCurrentTime() - CTimeSpan(1,0,0,0);
	switch(nType)
	{
	case DATE_LONG:
		strTime = tmCur.Format("%Y-%m-%d %H:%M:%S");
		break;
	case DATE_LONG2:
		strTime = tmCur.Format("%Y-%m-%d %H:%M:%S.000");
		break;
	case DAY_NORMAL:
		strTime = tmCur.Format("%Y%m%d");
		break;
	case TIME_NORMAL:
		strTime = tmCur.Format("%H%M%S");
		break;
	case DATE_NORMAL:
		strTime = tmCur.Format("%Y%m%d%H%M%S");
		break;
	case DAY_SHORT:
		strTime = tmCur.Format("%m%d");
		break;
	case DAY_LONG:
		strTime = tmCur.Format("%Y-%m-%d");
		break;
	case TIME_LONG:
		strTime = tmCur.Format("%H:%M:%S");
		break;
	case TIME_SHORT1:
		strTime = tmCur.Format("%H:%M");
		break;
	case TIME_SHORT2:
		strTime = tmCur.Format("%H %M");
		break;
	case MONTH_NORMAL:
		strTime = tmCur.Format("%Y%m");
		break;
	case MONTH_LONG:
		strTime = tmCur.Format("%Y年%m月%d日");
		break;
	case BANK_NORMAL:
		strTime = tmCur.Format("%m%d%H%M%S");
		break;
	}
	return strTime.GetBuffer(0);
}

std::vector<CString> SplitString(CString sData, const char* sFlag)
{
    std::vector<CString> vct;
    int iBegin=0, iEnd=0;
    CString sTemp;
    if(sFlag == "") return vct;
    iEnd = sData.Find(sFlag, iBegin);
    while(iEnd != -1)
    {
        sTemp = sData.Mid(iBegin, iEnd-iBegin);
        vct.push_back(sTemp);
        iBegin = iEnd+strlen(sFlag);
        iEnd = sData.Find(sFlag, iBegin);
    }
    if(iBegin < sData.GetLength())
    {
        sTemp = sData.Mid(iBegin, sData.GetLength()-iBegin);
        vct.push_back(sTemp);
    }
	else
	{
        vct.push_back("");
	}
    
    return vct;
}

int CompareDoubleMax(INDEX_DOUBLE d1,INDEX_DOUBLE d2)
{
	return (d1.fContent > d2.fContent);
}

int CompareDoubleMin(INDEX_DOUBLE d1,INDEX_DOUBLE d2)
{
	return (d1.fContent < d2.fContent);
}

int GetMaxIndex(std::vector<INDEX_DOUBLE> vctHist,double fBaseline)
{
	sort(vctHist.begin(),vctHist.end(),CompareDoubleMax);
	if(vctHist[0].fContent < fBaseline) return -1;
	return vctHist[0].nIndex;
}

int GetMinIndex(std::vector<INDEX_DOUBLE> vctHist,double fBaseline)
{
	sort(vctHist.begin(),vctHist.end(),CompareDoubleMin);
	if(vctHist[0].fContent > fBaseline) return -1;
	return vctHist[0].nIndex;
}




