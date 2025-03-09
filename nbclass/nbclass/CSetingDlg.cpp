// CSetingDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CSetingDlg.h"
#include "afxdialogex.h"
#include<playsoundapi.h>
#include"resource.h"
// CSetingDlg 对话框

IMPLEMENT_DYNAMIC(CSetingDlg, CDialogEx)

CSetingDlg::CSetingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SETINGCONFIG, pParent)
	, m_port(0)
	, m_Max(0)
	, ntou(0)
{

}

CSetingDlg::~CSetingDlg()
{
}

void CSetingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_LPort);
	DDX_Control(pDX, IDC_EDIT3, m_MaxConnect);
	DDX_Text(pDX, IDC_EDIT1, m_port);
	DDX_Text(pDX, IDC_EDIT3, m_Max);
	DDX_Control(pDX, IDC_EDIT2, m_tou);
	DDX_Text(pDX, IDC_EDIT2, ntou);
}


BEGIN_MESSAGE_MAP(CSetingDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CSetingDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CSetingDlg 消息处理程序

//确定配置
void CSetingDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	((CnbclassApp*)AfxGetApp())->m_IniFile.SetInt("Settings", "ListenPort", m_port);      //向ini文件中写入值
	((CnbclassApp*)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnection", m_Max);
	((CnbclassApp*)AfxGetApp())->m_IniFile.SetInt("Settings", "透明度", ntou);
	MessageBox("设置成功，重启本程序后生效！");
	CDialogEx::OnOK();
}


BOOL CSetingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CString	key;
	m_port = ((CnbclassApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort",8080);
	m_Max = ((CnbclassApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection",1000);
	ntou = ((CnbclassApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "透明度",200);

	UpdateData(FALSE);

	/*
	key.Format("%d", nPort);
	m_LPort.SetWindowTextA(key);

	key.Format("%d", nMaxConnection);
	m_MaxConnect.SetWindowTextA(key);
	*/

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
