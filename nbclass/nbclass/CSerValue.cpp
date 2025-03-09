// CSerValue.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CSerValue.h"
#include "afxdialogex.h"


// CSerValue 对话框

IMPLEMENT_DYNAMIC(CSerValue, CDialog)

CSerValue::CSerValue(CWnd* pParent , CListCtrl* pService/*=nullptr*/)
	: CDialog(IDD_DIALOG1, pParent)
{
	m_list_service = pService;

}

CSerValue::~CSerValue()
{
}

void CSerValue::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_servername);
	DDX_Control(pDX, IDC_EDIT2, m_servertitle);
	DDX_Control(pDX, IDC_EDIT4, m_servervaule);
	DDX_Control(pDX, IDC_EDIT3, m_serverpath);
	DDX_Control(pDX, IDC_EDIT5, m_servertype);
	DDX_Control(pDX, IDC_EDIT6, m_serverstatus);
}


BEGIN_MESSAGE_MAP(CSerValue, CDialog)
END_MESSAGE_MAP()


// CSerValue 消息处理程序


BOOL CSerValue::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	int nItem = m_list_service->GetNextItem(-1, LVNI_SELECTED);
	CString strText = m_list_service->GetItemText(nItem, 0);
	strText += " 的属性";
	SetWindowText(strText);

	char szText[1024];
	m_list_service->GetItemText(nItem, 2, szText, 1024);
	m_servername.SetWindowTextA(szText);

	m_list_service->GetItemText(nItem, 0, szText, 1024);
	m_servertitle.SetWindowTextA(szText);

	m_list_service->GetItemText(nItem, 1, szText, 1024);
	m_servervaule.SetWindowTextA(szText);

	m_list_service->GetItemText(nItem, 5, szText, 1024);
	m_serverpath.SetWindowTextA(szText);

	m_list_service->GetItemText(nItem, 3, szText, 1024);
	m_servertype.SetWindowTextA(szText);

	m_list_service->GetItemText(nItem, 4, szText, 1024);
	if (strcmp(szText, "") == 0)
		m_serverstatus.SetWindowTextA("停止");
	else
		m_serverstatus.SetWindowTextA(szText);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
