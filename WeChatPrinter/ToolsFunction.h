#pragma once

#include <iostream>
using namespace std;
#include <afxstr.h>
#include <vector>
#include "LOG2.H"
#include "json.hpp"
using json = nlohmann::json;

//����ͷ�ļ�
#include <windows.h> 
#include <mmdeviceapi.h> 
#include <endpointvolume.h>
#include <audioclient.h>

#include <Windows.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include "myos.h"
#include <io.h>

#include "CommonFun.h"
#include "mystring.h"
int cmpfunc(const void * a, const void * b);

//��ó���ĵ�ǰ·��
wstring GetAppPathW();

/************************************************************************\
* ɾ���������ļ��к��ڲ����е��ļ�										*
* ��Σ��ļ��еľ���·������D:\\1\\										*
* ����ֵ�� True �ɹ��� False ʧ��										*
\************************************************************************/
bool RemoveDir(const char* szFileDir);

/************************************************************************\
* Ǩ���ļ��������·���к���ͬ���ļ����򲻻��ƶ�						*
* ��Σ���ȫ·������ȫ·�����Լ���·���µ�·�ļ����ͣ��� *,jpg,mp4		*
* ����ֵ�� 																*
\************************************************************************/
void RemoveFileToOtherPath(const char* pathOld, const char* pathNew, const char * type);

//������ڼ�
int GetWeekDay();

//�޸�����������������ֵ
//    -2 �ָ�����
//    -1 ����
//    0~100:��������
bool SetVolumeLevel(int level);

//����ͼƬ������ͼƬ·��
CString Base64EncodePic(CString strPicPath);

//�ж��Ƿ���json�ļ����Ƿ���Ϲ淶
bool IsJsonData(std::string strData);

//ʹ��˽Կ������3DES����
CString Encode_3Des(CString strPrivatekey, CString strin);

//ʹ��˽Կ������3DES����
CString Decode_3Des(CString strPrivatekey, CString strin);

//����ļ��Ƿ����
BOOL CheckFileExist(CString filepath);

//
CString GetFileName(CString strFilePath);


float GetMemory();

BOOL Base64decodePic(std::string  strBase64, CString strFilePath);

void Base64ToPicture(CString sBase64, CString picPath);

int UrlEncodeUtf8_(LPCSTR pszUrl, LPSTR pszEncode, int nEncodeLen);

BOOL HTTP_Download2(CString strURL, CString strFilePath, CString strUsername, CString strPassword, CString strProxyList);
