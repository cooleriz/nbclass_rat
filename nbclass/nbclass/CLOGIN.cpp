// CLOGIN.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CLOGIN.h"
#include "afxdialogex.h"


// CLOGIN 对话框

IMPLEMENT_DYNAMIC(CLOGIN, CDialogEx)

CLOGIN::CLOGIN(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LGOINDLG, pParent)
	, m_Mhwid(_T(""))
	, m_key(_T(""))
{

}

CLOGIN::~CLOGIN()
{
}

void CLOGIN::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Mhwid);
	DDX_Text(pDX, IDC_EDIT2, m_key);
}


BEGIN_MESSAGE_MAP(CLOGIN, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CLOGIN::OnBnClickedButton1)
END_MESSAGE_MAP()


// CLOGIN 消息处理程序


void CLOGIN::OnBnClickedButton1()
{
	UpdateData(1);
	CDialogEx::OnOK();
}
