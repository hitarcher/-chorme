#pragma once

#include <iostream>
using namespace std;
#include <afxstr.h>
#include <vector>
#include "LOG2.H"
#include "json.hpp"
#include "Base64.h"
using json = nlohmann::json;

//����ͷ�ļ�
#include <windows.h> 
#include <mmdeviceapi.h> 
#include <endpointvolume.h>
#include <audioclient.h>

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

CString Encode_3Des(CString strPrivatekey, CString strin);

CString Decode_3Des(CString strPrivatekey, CString strin);