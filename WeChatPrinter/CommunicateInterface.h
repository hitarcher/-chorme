/*
* Copyright (c) YYYY，某公司
* All rights reserved.
*
* 文件名称：CommunicateInterface.h
* 摘    要：实现与ATMP的通信及其算法
*
* 当前版本：1.0.4.0
* 作    者：mingl
* 完成日期：2017/2/14
*/
#pragma once
#ifndef __COMMUNICATE_INTERFACE_H__
#define __COMMUNICATE_INTERFACE_H__

#ifdef RN_API_EXPORTS
#define RN_API __declspec(dllexport) WINAPI
#else
#define RN_API __declspec(dllimport) WINAPI
#endif

#include <string>
using namespace std;

typedef DWORD RN_RESULT;
typedef DWORD RN_STATUS;

typedef enum {
	LOGLEVEL_NULL    = 0,
	LOGLEVEL_ERR     = 1,
	LOGLEVEL_ENTER   = 2,
	LOGLEVEL_DETAIL  = 3
}LOGLEVEL;

typedef enum {
	SUCCESS       = 0,	// 成功
	ERR_LMK_NUL   = 1,	// LMK为空
	ERR_LMK_FORM  = 2,	// LMK格式错误
	ERR_WK_NUL    = 3,	// WK为空
	ERR_WK_FORM   = 4,	// WK格式错误
	ERR_MAC_FROM  = 5,	// 计算MAC的数据格式错误
	ERR_COMM_CONN = 6,	// 无法连接服务器
	ERR_COMM_FROM = 7,	// 数据包接收失败
	ERR_ECB_EN    = 8,	// 3DES ECB模式加密失败
	ERR_ECB_DE    = 9	// 3DES ECB模式解密失败
}ERR_CODE;

typedef enum {
	encrpyt = 0,
	decrypt
}ENCRYPT_TYPE;

/*
* 函数介绍：设置本地/通信主密钥
* 输入参数：strWK  主密钥BCD（8421）码；16位
* 输出参数：
* 返回值  ：见 CCommunicateWithAtmp::ERR_CODE
* 备注    ：必须再计算mac之前执行
*/
EXTERN_C RN_RESULT RN_API COMM_SetLMK(const char* strLMK);
EXTERN_C RN_RESULT RN_API COMM_SetLMK3(const char* strLMK);

/*
* 函数介绍：设置工作密钥
* 输入参数：strWK  工作密钥BCD（8421）码；16位
* 输出参数：
* 返回值  ：见 CCommunicateWithAtmp::ERR_CODE
* 备注    ：必须再计算mac之前执行
*/
EXTERN_C RN_RESULT RN_API COMM_SetWK(const char* strWK);
EXTERN_C RN_RESULT RN_API COMM_SetWK3(const char* strWK);

/*
* 函数介绍：计算MAC ANSI X9.9 单DES
* 输入参数：strSecurityMsg  需要计算MAC的数据，长度必须是偶数
* 输出参数：strMAC          计算结果
* 返回值  ：见 CCommunicateWithAtmp::ERR_CODE
* 备注    ：必须再设置完成LMK与WK之后计算
*/
EXTERN_C RN_RESULT RN_API COMM_ComputeMAC_X99(const char* strSecurityMsg, char* strMAC);
/*
* 函数介绍：计算MAC ANSI X9.19 3DES双倍长计算
* 输入参数：strSecurityMsg  需要计算MAC的数据，长度必须是偶数
* 输出参数：strMAC          计算结果
* 返回值  ：见 CCommunicateWithAtmp::ERR_CODE
* 备注    ：必须再设置完成LMK与WK之后计算
*/
EXTERN_C RN_RESULT RN_API COMM_ComputeMAC_X919(const char* strSecurityMsg, char* strMAC);
EXTERN_C RN_RESULT RN_API COMM_ComputeMACPOS(const char* strSecurityMsg, char* strMAC);

EXTERN_C RN_RESULT RN_API COMM_ComputePIN(const unsigned char* ppinkblock, char* strPIN, int nEncrypt); // 1 单倍长  2 双倍长  3 国密（略）

EXTERN_C RN_RESULT RN_API COMM_DES(const char* strKey, const char* strIn, char* strOut, ENCRYPT_TYPE bEncrypt); // bEncrypt 0加密 1解密
EXTERN_C RN_RESULT RN_API COMM_DES3(const char* strKey, const char* strIn, char* strOut, ENCRYPT_TYPE bEncrypt);// bEncrypt 0加密 1解密

/*
* 函数介绍：3DES ECB加解密
* 输入参数：strKey 24个字节的密钥
*           strPlain  明文，加密时为入参，解密时为出参
*           strCipher 密文，加密时为出参，解密时为入参；格式为base64
*           bEncrypt  enum ENCRYPT_TYPE 根据参数来加解密
* 输出参数：略
* 返回值  ：见 CCommunicateWithAtmp::ERR_CODE
*/
EXTERN_C RN_RESULT RN_API COMM_DES3_ECB(std::string strKey, std::string& strPlain, std::string& strCipher, ENCRYPT_TYPE bEncrypt);// bEncrypt 0加密 1解密

/*
* 函数介绍：将数据传送给
* 输入参数：strIP      服务器ip
*           uPort      服务器端口
*           strMsgpack 发送包
*           dwTimeout  超时时间
* 输出参数：strReceipt 回执,长度不得低于1024
* 返回值  ：见 CCommunicateWithAtmp::ERR_CODE
* 备注    ：仅仅负责传输
*/
EXTERN_C RN_RESULT RN_API COMM_CommunicateWithAtmp(const char* strIP, UINT uPort, const char* strMsgpack, char* strReceipt, DWORD dwTimeout);



#endif