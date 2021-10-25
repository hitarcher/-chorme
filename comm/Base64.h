#pragma once

#include <string>
using namespace std;

class CBase64
{
public:
	CBase64(void);
	~CBase64(void);

	
	//pbData:输入数据, iDataByte:输入数据的长度
	string Encode(const BYTE* pbData, int iDataByte);

	//
	string Decode(const char* pData, int iDataLen, int& iOutLen);
};

