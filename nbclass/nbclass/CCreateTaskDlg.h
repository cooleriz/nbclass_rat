#pragma once


// CCreateTaskDlg 对话框

class CCreateTaskDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCreateTaskDlg)

public:
	CCreateTaskDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCreateTaskDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CREATETASK };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton2();
	CString m_TaskPath;
	CEdit m_TaskName;
	CString m_TaskNames;
	CString m_ExePath;
	CString m_ZhuoZhe;
	CString m_MiaoShu;
};
