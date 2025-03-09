#pragma once
#include"IOCPServer.h"

// CShellDlg 对话框

class CShellDlg : public CDialog
{
	DECLARE_DYNAMIC(CShellDlg)

public:
	void OnReceiveComplete();
	CShellDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // standard constructor
	virtual ~CShellDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHELL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CEdit m_Edit;
private:
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	UINT m_nCurSel;
	UINT m_nReceiveLength;
	void AddKeyBoardData();
	int is = 0;
	void ResizeEdit();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnChangeEditshell();
};
