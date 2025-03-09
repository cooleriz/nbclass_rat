#pragma once
// CBundleDlg 对话框
typedef struct
{
	DWORD FileSize;					//文件大小
	DWORD RunType;					//运行方式
	BYTE name[MAX_PATH];			//释放路径
}MYFILEINFO;

typedef struct
{
	DWORD Conut;
	MYFILEINFO info;
}BUNDLEINFO;

class CBundleDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBundleDlg)

public:
	CBundleDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CBundleDlg();
	PVOID AddSection(PBYTE Filebuff, DWORD Sectionsize, PDWORD buffsize);
	PVOID GetFileToBuff(char* fileName, PDWORD size);																																								//将文件读取到内存并返回指针和大小
	DWORD GetAlignmentSize(DWORD Alignment, DWORD Size);												//获取对齐后的大小
	DWORD SetFileInfo(CArray<PBYTE>* bufflist, BUNDLEINFO** FileInfo);
	void SaveFile(PVOID buff, DWORD size);
	BOOL is64 = FALSE;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BUNDLEDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CListBox m_Filelist;
	afx_msg void OnBnClickedButton3();
	CEdit m_path;
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
