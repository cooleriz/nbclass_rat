#pragma once
#include"IOCPServer.h"

// CTaskDlg 对话框

class CTaskDlg : public CDialog
{
	DECLARE_DYNAMIC(CTaskDlg)

public:
	CTaskDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	void OnReceiveComplete();
	virtual ~CTaskDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TASKDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
private:
	void AdjustList();
	void ShowTaskList();
	void GetTaskList();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMRClickTasklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTaskShuaxin();
	afx_msg void OnCreateTask();
	afx_msg void OnTaskRun();
	afx_msg void OnTaskStop();
	afx_msg void OnTaskDel();
	afx_msg void OnLvnColumnclickTasklist(NMHDR* pNMHDR, LRESULT* pResult);
};
