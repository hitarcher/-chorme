#pragma once

#include <iostream>
using namespace std;
#include <afxstr.h>
#include <vector>
#include "LOG2.H"
#include "json.hpp"
using json = nlohmann::json;

//音量头文件
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

//获得程序的当前路径
wstring GetAppPathW();

/************************************************************************\
* 删除包括该文件夹和内部所有的文件										*
* 入参：文件夹的绝对路径，如D:\\1\\										*
* 返回值： True 成功， False 失败										*
\************************************************************************/
bool RemoveDir(const char* szFileDir);

/************************************************************************\
* 迁移文件，如果新路径中含有同名文件，则不会移动						*
* 入参：旧全路径，新全路径，以及旧路径下的路文件类型，如 *,jpg,mp4		*
* 返回值： 																*
\************************************************************************/
void RemoveFileToOtherPath(const char* pathOld, const char* pathNew, const char * type);

//获得星期几
int GetWeekDay();

//修改音量，输入音量的值
//    -2 恢复静音
//    -1 静音
//    0~100:音量比例
bool SetVolumeLevel(int level);

//加密图片，传入图片路径
CString Base64EncodePic(CString strPicPath);

//判断是否是json文件，是否符合规范
bool IsJsonData(std::string strData);

//使用私钥，进行3DES加密
CString Encode_3Des(CString strPrivatekey, CString strin);

//使用私钥，进行3DES解密
CString Decode_3Des(CString strPrivatekey, CString strin);

//检测文件是否存在
BOOL CheckFileExist(CString filepath);

//
CString GetFileName(CString strFilePath);


float GetMemory();

BOOL Base64decodePic(std::string  strBase64, CString strFilePath);

void Base64ToPicture(CString sBase64, CString picPath);

int UrlEncodeUtf8_(LPCSTR pszUrl, LPSTR pszEncode, int nEncodeLen);

BOOL HTTP_Download2(CString strURL, CString strFilePath, CString strUsername, CString strPassword, CString strProxyList);
