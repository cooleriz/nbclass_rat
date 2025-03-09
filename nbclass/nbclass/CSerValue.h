#pragma once


// CSerValue 对话框

class CSerValue : public CDialog
{
	DECLARE_DYNAMIC(CSerValue)

public:
	CListCtrl* m_list_service;
	CSerValue(CWnd* pParent = nullptr, CListCtrl* pService = NULL);   // 标准构造函数
	virtual ~CSerValue();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_servername;
	CEdit m_servertitle;
	CEdit m_servervaule;
	CEdit m_serverpath;
	CEdit m_servertype;
	CEdit m_serverstatus;
};
