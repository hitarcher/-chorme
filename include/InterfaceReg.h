#ifndef __INTERFACE__REG__H__
#define __INTERFACE__REG__H__

#ifndef DLL
#define DLL __declspec(dllexport) 
#endif

extern "C" DLL const char* GetMAC(int a,int b,char* c,int d);
extern "C" DLL BOOL CheckLegal(char* strRegCode,int a,char* b,int c,char* d);
extern "C" DLL const char* GetLastErr();
















#endif