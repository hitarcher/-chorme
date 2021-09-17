#pragma once
#ifndef _INTERFACE_TINY_ZIP_H__
#define _INTERFACE_TINY_ZIP_H__

#ifdef RN_API_EXPORTS
#define RN_API __declspec(dllexport) WINAPI  
#else
#define RN_API __declspec(dllimport) WINAPI 
#endif

#ifdef __cplusplus
extern "C" {
#endif 

	typedef DWORD RN_RESULT;
	typedef DWORD RN_STATUS;

// 	enum LOGLEVEL
// 	{
// 		LOGLEVEL_NULL = 0,
// 		LOGLEVEL_ERR = 1,
// 		LOGLEVEL_ENTER = 2,
// 		LOGLEVEL_DETAIL = 3
// 	};

	/************************************************************************/
	/*                             ERR    MESSAGE                           */
	/************************************************************************/
#define RN_ERR_PARAM		1000	/// 参数错误
#define RN_ERR_OPEN			1001	/// 打开压缩文件失败
#define RN_ERR_FILEINFO		1002	/// 读取压缩文件错误
#define RN_ERR_CREATFILE	1003	/// 创建文件失败
#define RN_ERR_OPENFILE		1004	/// 打开压缩文件中的文件失败

#define RN_ERR_NOSUPPORT	9999	/// 功能不支持

	/* example */
	//==================================================
	// 函数名：_expFunction
	// 入参  ：null
	// 出参  ：null
	// 返回值：RN_RESULT 
	//==================================================
	//EXTERN_C RN_RESULT RN_API _expFunction(void);

	//==================================================
	// 函数名：_version
	// 入参  ：detail			> ture:详细信息
	//							> false:仅显示版本号
	// 出参  ：null
	// 返回值：version 
	//==================================================
	LPCTSTR RN_API _version(bool bdetail);

	//==================================================
	// 函数名：_zip
	// 入参  ：null
	// 出参  ：null
	// 返回值：RN_RESULT 
	//==================================================
	RN_RESULT RN_API _zip(const char* _in_pFolderPath, int nType);

	//==================================================
	// 函数名：_unzip
	// 入参  ：_in_pFilePath       > 待解压文件全路径
	//         _in_pOutputFilePath > 解压后文件路径；如果为空，解压到当前路径
	//         _in_Passwd          > 待解压文件密码；如果为空，无需密码
	// 出参  ：null
	// 返回值：RN_RESULT           > 成功；其他失败
	//==================================================
	RN_RESULT RN_API _unzip(const char* _in_pFilePath, const char* _in_pOutputFilePath = NULL,const char* _in_Passwd = NULL);






#ifdef __cplusplus
}
#endif 
#endif