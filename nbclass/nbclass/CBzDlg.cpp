// CBzDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CBzDlg.h"
#include "afxdialogex.h"


// CBzDlg 对话框

IMPLEMENT_DYNAMIC(CBzDlg, CDialogEx)

CBzDlg::CBzDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BZ, pParent)
	, m_beizhu(_T(""))
{

}

CBzDlg::~CBzDlg()
{
}

void CBzDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITBEIZHU, m_beizhu);
}


BEGIN_MESSAGE_MAP(CBzDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CBzDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CBzDlg 消息处理程序


void CBzDlg::OnBnClickedButton1()
{
	UpdateData(TRUE);
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}
