#pragma once
#include"IOCPServer.h"
enum MYKEY
{
	MHKEY_CLASSES_ROOT,
	MHKEY_CURRENT_USER,
	MHKEY_LOCAL_MACHINE,
	MHKEY_USERS,
	MHKEY_CURRENT_CONFIG
};

// CRegDlg 对话框

class CRegDlg : public CDialog
{
	DECLARE_DYNAMIC(CRegDlg)

public:
	CRegDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL);   // standard constructor
	void OnReceiveComplete();
	CString GetFullPath(HTREEITEM hCurrent);
	char getFatherPath(CString& FullPath);
	DWORD atod(char* ch);
	void EnableCursor(bool b);
	void TestOK();
	void addKey(char* buf);
	void addPath(char* tmp);
	BYTE getType(int index);

	void initView();
	void CreatStatusBar();

	void OnRegEdit(BYTE types, CString title);


	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	virtual ~CRegDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REGDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	HICON       m_hIcon;
	int         size[3];
	CImageList	m_ImageList_tree;
	CImageList	m_HeadIcon;
	HTREEITEM	m_hRoot;
	HTREEITEM	HKLM;
	HTREEITEM	HKCR;
	HTREEITEM	HKCU;
	HTREEITEM	HKUS;
	HTREEITEM	HKCC;
	HTREEITEM	SelectNode;
	CString Path;
	CString Key;
	CString Value;
	int index;
	

	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_tree;
	CListCtrl m_list;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
private:
	BYTE  type;
	int how;       //执行了什么操作 1，删除项  2，新建项   3，删除键  4， 新建项  5，编辑项   
	bool isEnable;    //控件是否可用
	bool isEdit;       //是否正在编辑
	CStatusBar  m_wndStatusBar;
public:
	afx_msg void OnTvnSelchangedRegpath(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickRegpath(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickRegvalue(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRegDel();
	afx_msg void OnRegtCreat();
	afx_msg void OnEdit();
	afx_msg void Ondel();
	afx_msg void OnRegNew();
};
