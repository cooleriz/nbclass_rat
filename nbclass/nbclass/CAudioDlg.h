#pragma once
#include"IOCPServer.h"
#include"Audio.h"
// CAudioDlg 对话框

class CAudioDlg : public CDialog
{
	DECLARE_DYNAMIC(CAudioDlg)

public:
	CAudioDlg(CWnd* pParent = NULL, CIOCPServer* IOCPServer = NULL, ClientContext* ContextObject = NULL);   // 标准构造函数
	virtual ~CAudioDlg();
	ClientContext* m_ContextObject;
	CIOCPServer* m_iocpServer;
	HICON          m_hIcon;
	CString       m_strIPAddress;
	DWORD         m_nTotalRecvBytes;
	BOOL          m_bIsWorking;
	BOOL		  m_bThreadRun;
	BOOL m_bSend = FALSE ; // 是否发送本地语音到远程
	HANDLE        m_hWorkThread;
	CAudio		  m_AudioObject;
	static DWORD  WorkThread(LPVOID lParam);
	void OnReceiveComplete(void);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUDIO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnBnClickedCheck1();
};
