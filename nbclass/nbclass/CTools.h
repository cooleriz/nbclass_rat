#pragma once
#include"IOCPServer.h"
class CTools
{
public:
	CTools(HWND pParent=NULL,CIOCPServer* pIOCPServer = NULL, ClientContext* pContext = NULL,DWORD Index =0);   // 标准构造函数
	void OnReceiveComplete();
	virtual ~CTools();

	void SaveSnapshot();
	void SendComm(DWORD Index);

	HWND m_pParent;
	CString m_IPAddress;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	HANDLE m_event;
};

