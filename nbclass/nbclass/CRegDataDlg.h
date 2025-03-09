#pragma once


// CRegDataDlg 对话框

class CRegDataDlg : public CDialog
{
	DECLARE_DYNAMIC(CRegDataDlg)

public:
	CRegDataDlg(CWnd* pParent , CString title);   // 标准构造函数
	virtual ~CRegDataDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REGDATADLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	BYTE type = 100;
	CString mytitle;
	BOOL m_Enable = false;
	BOOL m_Radio = false;
	BOOL m_editname = false;
	void Enable();
	afx_msg void OnBnClickedButton1();
	CString m_name;
	CString m_data;
	CButton m_sz;
	CButton m_dword;
	CButton m_qword;
	CButton m_expsz;
	virtual BOOL OnInitDialog();
};
