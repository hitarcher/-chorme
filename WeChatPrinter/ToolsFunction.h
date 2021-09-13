#pragma once

#include <iostream>
using namespace std;
#include <afxstr.h>
#include <vector>
#include "LOG2.H"
#include "json.hpp"
#include "Base64.h"
using json = nlohmann::json;

//音量头文件
#include <windows.h> 
#include <mmdeviceapi.h> 
#include <endpointvolume.h>
#include <audioclient.h>

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

CString Encode_3Des(CString strPrivatekey, CString strin);

CString Decode_3Des(CString strPrivatekey, CString strin);