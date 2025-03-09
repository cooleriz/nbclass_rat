#pragma once


// CBzDlg 对话框

class CBzDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBzDlg)

public:
	CBzDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CBzDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BZ };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_beizhu;
	afx_msg void OnBnClickedButton1();
};
