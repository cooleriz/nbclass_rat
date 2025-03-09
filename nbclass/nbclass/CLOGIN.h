#pragma once


// CLOGIN 对话框

class CLOGIN : public CDialogEx
{
	DECLARE_DYNAMIC(CLOGIN)

public:
	CLOGIN(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CLOGIN();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LGOINDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_Mhwid;
	CString m_key;
	afx_msg void OnBnClickedButton1();
};
