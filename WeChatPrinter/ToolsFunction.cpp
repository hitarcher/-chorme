#include <stdafx.h>
#include "ToolsFunction.h"

//3Des����lib��ͷ�ļ�
#include "CommunicateInterface.h"
#ifdef DEBUG                            
#pragma comment(lib, "Communicated.lib")  
#else                                   
#pragma comment(lib, "Communicate.lib")   
#endif  

int cmpfunc(const void * a, const void * b)
{
	return (*(int*)a - *(int*)b);
}

bool IsJsonData(std::string strData)
{
	if (strData[0] != '{')
		return false;

	int num = 1;
	for (unsigned int i = 1; i < strData.length(); ++i)
	{
		if (strData[i] == '{')
		{
			++num;
		}
		else if (strData[i] == '}')
		{
			--num;
		}

		if (num == 0)
		{
			return true;
		}
	}

	return false;
}

CString Encode_3Des(CString strPrivatekey, CString strin)
{
	std::string strKey = strPrivatekey;
	std::string strPlain = strin;
	std::string strCipher = "";
	COMM_DES3_ECB(strKey, strPlain, strCipher, encrpyt);
	return strCipher.c_str();
}

CString Decode_3Des(CString strPrivatekey, CString strin)
{
	std::string strKey = strPrivatekey;
	std::string strPlain = "";
	std::string strCipher = strin;
	COMM_DES3_ECB(strKey, strPlain, strCipher, decrypt);
	return strPlain.c_str();
}

//getFullpath
wstring GetAppPathW()
{
	wchar_t szExePath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, szExePath, MAX_PATH);
	wchar_t *pstr = wcsrchr(szExePath, '\\');
	memset(pstr + 1, 0, 2);
	wstring strAppPath(szExePath);
	return strAppPath;
}

//ɾ�ļ��к�����
bool RemoveDir(const char* szFileDir)
{
	std::string strDir = szFileDir;
	if (strDir.at(strDir.length() - 1) != '\\')
		strDir += '\\';
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile((strDir + "*.*").c_str(), &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_stricmp(wfd.cFileName, ".") != 0 &&
				_stricmp(wfd.cFileName, "..") != 0)
				RemoveDir((strDir + wfd.cFileName).c_str());
		}
		else
		{
			DeleteFile((strDir + wfd.cFileName).c_str());
		}
	} while (FindNextFile(hFind, &wfd));
	FindClose(hFind);
	RemoveDirectory(szFileDir);
	return true;
}

//Ǩ���ļ������Ϊ��ȫ·������ȫ·�����Լ���·���µ�·�ļ����ͣ���jpg,mp4,*�������·���к���ͬ���ļ����򲻻��ƶ���
void RemoveFileToOtherPath(const char* pathOld, const char* pathNew, const char * type)
{
	CFileFind finder;
	char szOldTextPath[MAX_PATH] = "";
	sprintf(szOldTextPath, "%s\\*.%s", pathOld, type);
	char szNewTextPath[MAX_PATH] = "";

	BOOL bWorking = finder.FindFile(szOldTextPath);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		CString strFileName = finder.GetFileName();
		if (finder.IsDirectory())continue;
		sprintf(szNewTextPath, "%s\\%s", pathNew, strFileName.GetBuffer(0));
		strFileName.ReleaseBuffer();
		MoveFile(finder.GetFilePath(), szNewTextPath);
	}
}

//��ý��������ڼ�������0-6����һ=0������=6
int GetWeekDay()
{
	/*
	time_t��һ���洢ʱ����Ϣ��long int
	time(&rawtime) ��ϵͳʱ�����rawtime���time_t��
	struct tm *timeinfo ��tm��һ����ʱ����Ϣ�Ľṹ�� timeinfo��ָ��һ������ṹ���ָ��
	timeinfo=localtime(&rawtime) ��֮ǰtime()�õ�����Ϣת��Ϊstruct tm����
	*/
	struct tm *ptr;
	time_t lt;
	lt = time(NULL);
	ptr = localtime(&lt);
	//mlgb,ϵͳ����0Ϊ���գ�ƽ̨���˲� ����0Ϊ��һ����ô���ظ���.
	int iSysTime = ptr->tm_wday;
	int iPFTime = iSysTime - 1;
	if (iPFTime < 0) iPFTime = 6;

	return  iPFTime;
}

CString GetBase64Pic(CString strPath)
{
	HANDLE hFile;
	hFile = CreateFile(strPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD dFileSize = GetFileSize(hFile, NULL);
	char * pBuffer = new char[dFileSize + 1];

	if (pBuffer == NULL)
		return false;

	memset(pBuffer, 0, dFileSize);

	DWORD dReadSize(0);
	if (!ReadFile(hFile, pBuffer, dFileSize, &dReadSize, NULL))
	{
		delete[]pBuffer;
		CloseHandle(hFile);
		return false;
	}
	string strData = base64_encode((const unsigned char*)pBuffer, dReadSize);
	CString strReturn = strData.c_str();
	delete[]pBuffer;
	CloseHandle(hFile);
	return strReturn;
}

//����:
//    -2 �ָ�����
//    -1 ����
//    0~100:��������
bool SetVolumeLevel(int level)
{
	HRESULT hr;
	IMMDeviceEnumerator* pDeviceEnumerator = 0;
	IMMDevice* pDevice = 0;
	IAudioEndpointVolume* pAudioEndpointVolume = 0;
	IAudioClient* pAudioClient = 0;

	try {
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pDeviceEnumerator);
		if (FAILED(hr)) throw "CoCreateInstance";
		hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
		if (FAILED(hr)) throw "GetDefaultAudioEndpoint";
		hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pAudioEndpointVolume);
		if (FAILED(hr)) throw "pDevice->Active";
		hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
		if (FAILED(hr)) throw "pDevice->Active";

		if (level == -2) {
			hr = pAudioEndpointVolume->SetMute(FALSE, NULL);
			if (FAILED(hr)) throw "SetMute";
		}
		else if (level == -1) {
			hr = pAudioEndpointVolume->SetMute(TRUE, NULL);
			if (FAILED(hr)) throw "SetMute";
		}
		else {
			if (level < 0 || level>100) {
				hr = E_INVALIDARG;
				throw "Invalid Arg";
			}

			float fVolume;
			fVolume = level / 100.0f;
			hr = pAudioEndpointVolume->SetMasterVolumeLevelScalar(fVolume, &GUID_NULL);
			if (FAILED(hr)) throw "SetMasterVolumeLevelScalar";

			pAudioClient->Release();
			pAudioEndpointVolume->Release();
			pDevice->Release();
			pDeviceEnumerator->Release();
			return true;
		}
	}
	catch (...) {
		if (pAudioClient) pAudioClient->Release();
		if (pAudioEndpointVolume) pAudioEndpointVolume->Release();
		if (pDevice) pDevice->Release();
		if (pDeviceEnumerator) pDeviceEnumerator->Release();
		throw;
	}
	return false;
}

CString Base64EncodePic(CString strPicPath)
{
	HANDLE hFile;
	hFile = CreateFile(strPicPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD dFileSize = GetFileSize(hFile, NULL);
	char * pBuffer = new char[dFileSize + 1];

	if (pBuffer == NULL)
		return false;

	memset(pBuffer, 0, dFileSize);

	DWORD dReadSize(0);
	if (!ReadFile(hFile, pBuffer, dFileSize, &dReadSize, NULL))
	{
		delete[]pBuffer;
		CloseHandle(hFile);
		return false;
	}
	std::string strData = "";
	strData = base64_encode((unsigned const char*)pBuffer, dReadSize);


	delete[]pBuffer;
	CloseHandle(hFile);
	return strData.c_str();
}

BOOL Base64decodePic(std::string  strBase64, CString strFilePath)
{
	HANDLE hFile;
	hFile = CreateFile(strFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	std::string strData = base64_decode(strBase64);
	DWORD dwReturn;
	if (!WriteFile(hFile, strData.data(), strData.length(), &dwReturn, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	return TRUE;
}	


BOOL CheckFileExist(CString filepath)
{
	return exist(filepath.GetBuffer(0)) ? TRUE : FALSE;
}


CString GetFileName(CString strFilePath)
{
	CString str = strFilePath;
	str.Replace("\\", "/");
	return str.Mid(str.ReverseFind('/') + 1, str.GetLength());
}

#define _WIN32_WINNT  0x0501 // I misunderstand that

float GetMemory()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex); // I misunderstand that
	GlobalMemoryStatusEx(&statex);
	return (float)statex.ullTotalPhys / (1024 * 1024 * 1024);
}



int UrlEncodeUtf8_(LPCSTR pszUrl, LPSTR pszEncode, int nEncodeLen)
{
	int nRes = 0;
	//�������
	wchar_t* pWString = NULL;
	char* pString = NULL;
	char* pResult = NULL;

	do
	{
		if (pszUrl == NULL)
			break;

		//�Ƚ��ַ����ɶ��ֽ�ת����UTF-8����  
		int nLength = MultiByteToWideChar(CP_ACP, 0, pszUrl, -1, NULL, 0);

		//����Unicode�ռ�  
		pWString = new wchar_t[nLength + 1];
		if (pWString == NULL)
			break;

		memset(pWString, 0, (nLength + 1) * sizeof(wchar_t));
		//��ת����Unicode
		MultiByteToWideChar(CP_ACP, 0, pszUrl, -1, pWString, nLength);

		//����UTF-8�ռ�
		nLength = WideCharToMultiByte(CP_UTF8, 0, pWString, -1, NULL, 0, NULL, NULL);
		pString = new char[nLength + 1];
		if (pString == NULL)
			break;

		memset(pString, 0, nLength + 1);
		//Unicodeת��UTF-8
		nLength = WideCharToMultiByte(CP_UTF8, 0, pWString, -1, pString, nLength, NULL, NULL);

		pResult = new char[nLength * 3];
		if (pResult == NULL)
			break;

		memset(pResult, 0, nLength * 3);
		char* pTmp = pResult;
		static char hex[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

		for (int i = 0; i < nLength; i++)
		{
			unsigned char c = pString[i];
			if (c == 0)
			{
				break;
			}

			if (c > 0x20 && c < 0x7f)// ���ֻ���ĸ
			{
				*pTmp++ = c;
			}
			else if (c == 0x20)// �����ո�  
			{
				*pTmp++ = '%';
				*pTmp++ = hex[c / 16];
				*pTmp++ = hex[c % 16];
			}
			else// ���б���
			{
				*pTmp++ = '%';
				*pTmp++ = hex[c / 16];
				*pTmp++ = hex[c % 16];
			}
		}
		nLength = strlen(pResult);
		nRes = nLength;
		if (pszEncode == NULL || nEncodeLen < nLength)
			break;

		memcpy(pszEncode, pResult, nLength);
	} while (0);

	if (pWString != NULL)
		delete[]pWString;

	if (pString != NULL)
		delete[]pString;

	if (pResult != NULL)
		delete[]pResult;

	return nRes;
}


BOOL HTTP_Download2(CString strURL, CString strFilePath, CString strUsername, CString strPassword, CString strProxyList)
{
	BOOL bRes = FALSE;
#define PATH_LEN 1024
	//����http����
	TCHAR szUserName[MAX_PATH] = { 0 };
	TCHAR szPassword[MAX_PATH] = { 0 };
	_tcscpy(szUserName, strUsername);
	_tcscpy(szPassword, strPassword);

	const TCHAR szHeaders[] = _T("Accept: */*\r\nUser-Agent:  Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)\r\n");    //Э��

	CInternetSession    aInternetSession;        //һ���Ự
	CHttpConnection*    pHttpConnection = NULL;    //����
	CHttpFile*          pHttpFile = NULL;
	DWORD               dwFileStatus;
	INTERNET_PORT       nPort;
	DWORD               dwServiceType;
	DWORD               dwDownloadSize = 0;
	CString             strServer;
	CString             strObject;
	CString				strDownloadFile = "";
	const int nBufferSize = 4096;
	TCHAR szURL[nBufferSize] = { 0 };
	UrlEncodeUtf8_(strURL, szURL, nBufferSize);
	strURL = szURL;

	//�ֽ�URL
	if (AfxParseURL(strURL, dwServiceType, strServer, strObject, nPort))
	{
		//�������������HTTP����
		if (dwServiceType != AFX_INET_SERVICE_HTTP && dwServiceType != AFX_INET_SERVICE_HTTPS)
		{
			//���سɹ�
			return bRes;
		}
	}

	try
	{
		if (!strProxyList.IsEmpty())
		{
			INTERNET_PROXY_INFO proxyinfo;
			proxyinfo.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
			proxyinfo.lpszProxy = strProxyList;
			proxyinfo.lpszProxyBypass = NULL;
			aInternetSession.SetOption(INTERNET_OPTION_PROXY, (LPVOID)&proxyinfo, sizeof(INTERNET_PROXY_INFO));
		}

		pHttpConnection = aInternetSession.GetHttpConnection(strServer, nPort);
		do
		{
			//���ʧ�����߳��˳�
			if (pHttpConnection == NULL)
				break;

			//ȡ��HttpFile����
			pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,
				strObject,
				NULL,
				1,
				NULL,
				NULL,
				INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);

			if (pHttpFile == NULL)
				break;

			pHttpFile->SetOption(INTERNET_OPTION_PROXY_USERNAME, szUserName, _tcslen(szUserName) + 1);
			pHttpFile->SetOption(INTERNET_OPTION_PROXY_PASSWORD, szPassword, _tcslen(szPassword) + 1);

			if (!pHttpFile->AddRequestHeaders(szHeaders))//��������ͷ
				break;

			if (!pHttpFile->SendRequest())//�����ļ�����
				break;

			if (!pHttpFile->QueryInfoStatusCode(dwFileStatus))//��ѯ�ļ�״̬
				break;

			if ((dwFileStatus / 100) * 100 != HTTP_STATUS_OK)//����ļ�״̬����
				break;

			CFile aFile;
			int pos = strFilePath.ReverseFind('.');
			strDownloadFile = strFilePath.Left(pos) + ".download";

			if (!aFile.Open(strDownloadFile, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
				break;

			DWORD dwFileLen;
			DWORD dwWordSize = sizeof(dwFileLen);
			pHttpFile->QueryInfo(HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH, &dwFileLen, &dwWordSize, NULL);

			do
			{
				BYTE szBuffer[PATH_LEN] = { 0 };
				DWORD dwLen = pHttpFile->Read(szBuffer, PATH_LEN);//��������
				if (dwLen == 0)
					break;

				aFile.Write(szBuffer, dwLen);
				dwDownloadSize += dwLen;
			} while (1);

			aFile.Close();
			DWORD dw = 0;
			if (InternetQueryDataAvailable((HINTERNET)(*pHttpFile), &dw, 0, 0) && (dw == 0))
				bRes = TRUE;//���óɹ���־

			if (dwDownloadSize != dwFileLen)
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "HTTP_Download2", "ʵ������dwDownloadSize =%ld��Ӧ����dwFileLen=%ld��[%s]����ʧ��", dwDownloadSize, dwFileLen, strFilePath);
				bRes = FALSE;
			}

		} while (0);

		if (pHttpFile != NULL)
			delete pHttpFile;

		if (pHttpConnection != NULL)
			delete pHttpConnection;

		//�ر�http����
		aInternetSession.Close();


		rename(strDownloadFile, strFilePath);
	}
	//�쳣����
	catch (CInternetException* e)
	{
		TCHAR   szCause[MAX_PATH] = { 0 };
		//ȡ�ô�����Ϣ
		e->GetErrorMessage(szCause, MAX_PATH);

		TRACE("internet exception:%s\n", szCause);//������Ϣд����־

		e->Delete();//ɾ���쳣����

		if (pHttpFile != NULL)//ɾ��http�ļ�����
			delete pHttpFile;

		if (pHttpConnection != NULL)//ɾ��http���Ӷ���
			delete pHttpConnection;

		aInternetSession.Close();//�ر�http����
	}
	catch (...)
	{
		if (pHttpFile != NULL)//ɾ��http�ļ�����
			delete pHttpFile;

		if (pHttpConnection != NULL)//ɾ��http���Ӷ���
			delete pHttpConnection;

		aInternetSession.Close();//�ر�http����
	}

	return bRes;
}