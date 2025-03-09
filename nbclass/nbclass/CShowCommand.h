#pragma once


// CShowCommand 对话框

class CShowCommand : public CDialog
{
	DECLARE_DYNAMIC(CShowCommand)

public:
	CShowCommand(CWnd* pParent = nullptr);   // 标准构造函数
	void Printf(CString data);
	virtual ~CShowCommand();
	CFont m_Editfont;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMMAND };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_command;
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
	virtual void PostNcDestroy();
	afx_msg void OnDestroy();
};
