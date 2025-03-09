#pragma once


// CGenerate 对话框

class CGenerate : public CDialog
{
	DECLARE_DYNAMIC(CGenerate)

public:
	CGenerate(CWnd* pParent = nullptr);   // 标准构造函数
	void BuildLogOut(CString log);				//输出编译信息
	static	DWORD WINAPI	TestMaster(LPVOID lparam);   //测试连接用的
	unsigned char* base64_encode(unsigned char* str, DWORD str_len);
	BOOL SetComd(char* buff, DWORD size);
	void GenerateDLL(char* name, unsigned char* data, DWORD size, char* sig);
	//PCHAR BuildShellcode(PCHAR buff,PDWORD size);
	void BuildServer(char* DNS, int port, char* Path);  //编译
	int CGenerate::memfind(const CHAR* mem, const TCHAR* str, int sizem, int sizes);  //查找要替换的数据
	virtual ~CGenerate();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GENERATE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_InsDir;
	CEdit m_Log;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CButton m_test;
	int m_portVaule;
	CString m_vipweb;
	afx_msg void OnBnClickedMove();
	CComboBox m_type;
	CButton m_mov;
	CButton m_del;
	CButton m_kill;
	CButton m_threatbook;
	CButton m_vm;
	CButton m_Antidebug;
	CButton m_puppet;
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnBnClickedButton6();

	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
};
