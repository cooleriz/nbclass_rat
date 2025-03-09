// CRegDataDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CRegDataDlg.h"
#include "afxdialogex.h"


// CRegDataDlg 对话框

IMPLEMENT_DYNAMIC(CRegDataDlg, CDialog)

CRegDataDlg::CRegDataDlg(CWnd* pParent /*=nullptr*/,CString title)
	: CDialog(IDD_REGDATADLG, pParent)
	, m_name(_T(""))
	, m_data(_T(""))
{
	mytitle = title;
}

CRegDataDlg::~CRegDataDlg()
{
}

void CRegDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_name);
	DDX_Text(pDX, IDC_EDIT3, m_data);
	DDX_Control(pDX, IDC_RADIO1, m_sz);
	DDX_Control(pDX, IDC_RADIO2, m_dword);
	DDX_Control(pDX, IDC_RADIO3, m_qword);
	DDX_Control(pDX, IDC_RADIO4, m_expsz);
}


BEGIN_MESSAGE_MAP(CRegDataDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CRegDataDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CRegDataDlg 消息处理程序
enum KEYVALUE
{
	MREG_SZ,
	MREG_DWORD,
	MREG_QWORD,
	MREG_BINARY,
	MREG_EXPAND_SZ,
	MREG_MULTI_SZ
};



void CRegDataDlg::OnBnClickedButton1()
{

	if (m_sz.GetCheck())
	{
		this->type = MREG_SZ;
	}
	else if (m_dword.GetCheck())
	{
		this->type = MREG_DWORD;
	}
	else if (m_qword.GetCheck())
	{
		this->type = MREG_QWORD;
	}
	else if (m_expsz.GetCheck())
	{
		this->type = MREG_EXPAND_SZ;
	}

	CDialog::OnOK();
	// TODO: 在此添加控件通知处理程序代码
}


BOOL CRegDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//设置标题
	SetWindowTextA(mytitle);
	// TODO:  在此添加额外的初始化

	switch (type)
	{
	case MREG_SZ:
		m_sz.SetCheck(1);
		break;
	case MREG_DWORD:
		m_dword.SetCheck(1);
		break;
	case MREG_QWORD:
		m_qword.SetCheck(1);
		break;
	case MREG_EXPAND_SZ:
		m_expsz.SetCheck(1);
		break;
	default:
		break;
	}

	Enable();



	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



void CRegDataDlg::Enable()
{
	//禁用单选框
	if (m_Radio)
	{
		m_dword.EnableWindow(0);
		m_sz.EnableWindow(0);
		m_qword.EnableWindow(0);
		m_expsz.EnableWindow(0);
	}

	//禁用值编辑框
	if (m_Enable)
	{
		GetDlgItem(IDC_EDIT3)->EnableWindow(false);
	}

	//禁用名称编辑框
	if (m_editname)
	{
		GetDlgItem(IDC_EDIT1)->EnableWindow(false);
	}

	
	UpdateData(false);
}