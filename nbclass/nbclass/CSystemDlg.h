#pragma once
#include"IOCPServer.h"
#include"SEU_QQwry.h"
// CSystemDlg 对话框

class CSystemDlg : public CDialog
{
	DECLARE_DYNAMIC(CSystemDlg)

public:
	CSystemDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	void OnReceiveComplete();

	void initView();								//初始化界面
	virtual ~CSystemDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYSTEM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_Env;
	CListCtrl m_pro;
	CListCtrl m_windows;
	CListCtrl m_net;
	CListCtrl m_install;
	CListCtrl m_run;
	CListCtrl m_HistoryFile;
	CListCtrl m_mstsc;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	SEU_QQwry m_SQQwry;          //ip位置

private:
	HICON m_hIcon;
	
	void AdjustList();
	void ShowSelectWindow(BOOL bInit = FALSE);
	void OnShowWindow(int Show);
	//点击列表框标题排序
	

	void KillAndDel(BYTE buff);
	/*显示类函数*/
	void ShowProcessList();
	void ShowStartupList();
	void ShowHostsInfo();
	void ShowWindowsList();
	void	ShowNetStateList();
	void ShowSoftWareList();
	void ShowHistoryFile();
	void ShowRemote();
	void ShowHistoryIE();

	/*获取类函数*/
	void GetStartupList();
	void GetHostsInfo();
	void GetProcessList();
	void GetWindowsList();
	void	GetNetStateList();
	void GetSoftWareList();
	void GetHistoryFile();
	void GetRemote();
	void GetHistoryIE();

public:
	virtual BOOL OnInitDialog();
	CTabCtrl m_tab;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CEdit m_hosts;
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	virtual void PostNcDestroy();
	CButton m_butGethost;
	CButton m_butSavehost;
	afx_msg void OnEnChangeHosts();
	afx_msg void OnBnClickedGethost();
	afx_msg void OnBnClickedSvaehost();
	afx_msg void OnNMRClickListPeocess(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillProcess();
	afx_msg void OnGetPs();
	afx_msg void OnDelPs();
	afx_msg void OnNMRClickListNet(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickListWindows(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnWindows();
	afx_msg void OnWindowsHide();
	afx_msg void OnWindowsShow();
	afx_msg void OnWindowsClose();
	afx_msg void OnWindowsMax();
	afx_msg void OnWindowsMin();
	afx_msg void OnUnistall();
	afx_msg void OnNMRClickListInstall(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnInistall32817();
	afx_msg void OnLvnColumnclickListPeocess(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnColumnclickListWindows(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnColumnclickListNet(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPsFind();
};
