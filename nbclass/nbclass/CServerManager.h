#pragma once
#include"IOCPServer.h"

// CServerManager 对话框

class CServerManager : public CDialog
{
	DECLARE_DYNAMIC(CServerManager)

public:
	CServerManager(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	void OnReceiveComplete();
	virtual ~CServerManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVERDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CListCtrl m_server;
	afx_msg void OnNMRClickServerlist(NMHDR* pNMHDR, LRESULT* pResult);
private:
	void AdjustList();
	void ShowServiceList();
	void GetServiceList();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnNMDblclkServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnColumnclickServerlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnServerStart();
	afx_msg void OnServerStop();
	afx_msg void OnServerDel();
	afx_msg void OnServerGet();
	afx_msg void OnServerSetShoudong();
	afx_msg void OnServerAuto();
	afx_msg void OnServerClose();
	afx_msg void OnServerCreate();
};
