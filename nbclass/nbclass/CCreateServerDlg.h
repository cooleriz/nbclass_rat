#pragma once


// CCreateServerDlg 对话框

class CCreateServerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCreateServerDlg)

public:
	CCreateServerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCreateServerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CREATESERVER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_serName;
	CString m_serShowName;
	CString m_serPath;
	afx_msg void OnBnClickedButton2();
	virtual BOOL OnInitDialog();
};
