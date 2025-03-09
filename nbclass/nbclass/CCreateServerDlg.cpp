// CCreateServerDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CCreateServerDlg.h"
#include "afxdialogex.h"


// CCreateServerDlg 对话框

IMPLEMENT_DYNAMIC(CCreateServerDlg, CDialogEx)

CCreateServerDlg::CCreateServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CREATESERVER, pParent)
	, m_serName(_T(""))
	, m_serShowName(_T(""))
	, m_serPath(_T(""))
{

}

CCreateServerDlg::~CCreateServerDlg()
{

}

void CCreateServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_serName);
	DDX_Text(pDX, IDC_EDIT2, m_serShowName);
	DDX_Text(pDX, IDC_EDIT4, m_serPath);
}


BEGIN_MESSAGE_MAP(CCreateServerDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &CCreateServerDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CCreateServerDlg 消息处理程序


void CCreateServerDlg::OnBnClickedButton2()
{
	UpdateData(TRUE);
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


BOOL CCreateServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
