// CCreateTaskDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CCreateTaskDlg.h"
#include "afxdialogex.h"


// CCreateTaskDlg 对话框

IMPLEMENT_DYNAMIC(CCreateTaskDlg, CDialogEx)

CCreateTaskDlg::CCreateTaskDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CREATETASK, pParent)
	, m_TaskPath(_T("\\"))
	, m_TaskNames(_T("bhyy"))
	, m_ExePath(_T("C:\\windows\\system32\\cmd.exe"))
	, m_ZhuoZhe(_T("Microsoft Corporation"))
	, m_MiaoShu(_T("此任务用于在需要时启动 Windows 更新服务以执行计划的操作(如扫描)"))
{

}

CCreateTaskDlg::~CCreateTaskDlg()
{

}

void CCreateTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_TaskPath);
	DDX_Control(pDX, IDC_EDIT2, m_TaskName);
	DDX_Text(pDX, IDC_EDIT2, m_TaskNames);
	DDX_Text(pDX, IDC_EDIT7, m_ExePath);
	DDX_Text(pDX, IDC_EDIT4, m_ZhuoZhe);
	DDX_Text(pDX, IDC_EDIT3, m_MiaoShu);
}


BEGIN_MESSAGE_MAP(CCreateTaskDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &CCreateTaskDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CCreateTaskDlg 消息处理程序


void CCreateTaskDlg::OnBnClickedButton2()
{
	UpdateData(TRUE);
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}
