#pragma once
#include"IOCPServer.h"
#include"AudioRender.h"
#include"PlaybackAudioCapture.h"
// CSpeakerDlg 对话框

class CSpeakerDlg : public CDialog
{
	DECLARE_DYNAMIC(CSpeakerDlg)

public:
	CSpeakerDlg(CWnd* pParent = NULL, CIOCPServer* IOCPServer = NULL, ClientContext* ContextObject = NULL);   // 标准构造函数
	virtual ~CSpeakerDlg();
	ClientContext* m_ContextObject;
	CIOCPServer* m_iocpServer;
	HICON          m_hIcon;
	CString       m_strIPAddress;
	DWORD         m_nTotalRecvBytes;
	BOOL          m_bIsWorking;
	BOOL		  m_bThreadRun;
	HANDLE        m_hWorkThread;
	CAudioRenderImpl SetSpeakerDate;
	CPlaybackCaptureImpl GetSpeakerDate;
	BOOL m_brec; // 是否接收语音到本地
	static DWORD  CSpeakerDlg::WorkThread(LPVOID lParam);

	void CSpeakerDlg::OnReceiveComplete(void);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SPEAKER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	CButton m_startL;
	CButton m_stopL;
};
