#pragma once

#include <string>
using namespace std;

class CBase64
{
public:
	CBase64(void);
	~CBase64(void);

	
	//pbData:��������, iDataByte:�������ݵĳ���
	string Encode(const BYTE* pbData, int iDataByte);

	//
	string Decode(const char* pData, int iDataLen, int& iOutLen);
};

