#pragma once
#ifndef HEADS_H_
#define HEADS_H_
#include "Config.h"
#include <stdio.h>
#include "CommonFun.h"
#include "LOG2.H"
#include "json.hpp"
using json = nlohmann::json;

#ifdef DEBUG                            
#pragma comment(lib, "CommUtilsd.lib")  
#else                                   
#pragma comment(lib, "CommUtils.lib")   
#endif    

#include "mystring.h"
#include "myos.h"
#include "mylog.h"
#include "Config.h"
// 
// //����ͷ�ļ�
// #include "MakePNG.h"
// 
// //��ѹ��ͷ�ļ�
// #pragma comment( lib, "coinreceiver.lib" )
// #include "tinyzipinterface.h"
// #pragma comment( lib, "tinyzip.lib" )
// 
// //���ߺ���ͷ�ļ�
// #include "ToolsFunction.h"
// //�ڴ溯�����ͷ�ļ�
// #include <psapi.h>
// 
// 
// //CEF3
// #include "mycef.h"
// #include "simple_app.h"
// #include "simple_handler.h"
// 
// #include "mystring.h"
// #include "myos.h"
// #include "mylog.h"
// #include "myimagecompress.h"
// #include "simple_handler.h"
// 
// #include "ToolsFunction.h"

#endif 
