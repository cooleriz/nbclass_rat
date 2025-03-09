#pragma once


// CSetingDlg 对话框

class CSetingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetingDlg)

public:
	CSetingDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSetingDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETINGCONFIG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_LPort;
	CEdit m_key;
	CEdit m_MaxConnect;
	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
	int m_port;
	CString m_keyValue;
	int m_Max;
	CEdit m_tou;
	int ntou;
};
