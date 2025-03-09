// CShowCommand.cpp: 实现文件
//


#include "pch.h"
#include "nbclass.h"
#include "CShowCommand.h"
#include "afxdialogex.h"


// CShowCommand 对话框

IMPLEMENT_DYNAMIC(CShowCommand, CDialogEx)

CShowCommand::CShowCommand(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_COMMAND, pParent)
{

}

CShowCommand::~CShowCommand()
{
	
}

void CShowCommand::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_command);
}


BEGIN_MESSAGE_MAP(CShowCommand, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CShowCommand 消息处理程序


BOOL CShowCommand::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_command.SetLimitText(MAXDWORD); // 设置最大长度

	//设置字体和大小
	m_Editfont.CreatePointFont(80, "微软雅黑");
	m_command.SetFont(&m_Editfont);

	Printf("###查询当前电脑保存的wifi");
	Printf("netsh wlan show profiles");
	Printf("");

	Printf("###查询指定wifi密码");
	Printf("netsh wlan show profiles FAST_0E1A key=clear");
	Printf("");

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CShowCommand::Printf(CString data)
{
	CString str;
	str = " " + data + "\r\n";
	m_command.ReplaceSel(str);
}

HBRUSH CShowCommand::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	if (pWnd->GetDlgCtrlID() == IDC_EDIT1)
	{
		COLORREF clr = RGB(0, 255, 0);
		pDC->SetTextColor(clr);   //设置白色的文本
		clr = RGB(0, 0, 0);
		pDC->SetBkColor(clr);     //设置黑色的背景
		return CreateSolidBrush(clr);  //作为约定，返回背景色对应的刷子句柄
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}


void CShowCommand::OnClose()
{
	DestroyWindow();
	
	CDialog::OnClose();
}


void CShowCommand::PostNcDestroy()
{
	//delete this;
	CDialog::PostNcDestroy();
}


void CShowCommand::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}
