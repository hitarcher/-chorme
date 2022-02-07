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
// //截屏头文件
// #include "MakePNG.h"
// 
// //解压缩头文件
// #pragma comment( lib, "coinreceiver.lib" )
// #include "tinyzipinterface.h"
// #pragma comment( lib, "tinyzip.lib" )
// 
// //工具函数头文件
// #include "ToolsFunction.h"
// //内存函数相关头文件
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
