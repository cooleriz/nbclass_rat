#pragma once


// CInjection 对话框

class CInjection : public CDialogEx
{
	DECLARE_DYNAMIC(CInjection)

public:
	CInjection(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CInjection();
	BOOL Inject(PBYTE buff);
	BOOL SaveFile(PVOID buff,  DWORD size);
	DWORD GetAlignmentSize(DWORD Alignment, DWORD Size);
	PVOID AddSection(PBYTE Filebuff, DWORD Sectionsize ,PDWORD buffsize);

	BOOL is64 = TRUE;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_injection};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_FilePath;
	CString m_DllPath;
	CString m_FuncName;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
};
