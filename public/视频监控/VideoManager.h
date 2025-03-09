#pragma once
#include "../../PublicInclude/Manager.h"
#include "CaptureVideo.h"
#include "../../common/VideoCodec.h"

class CVideoManager : public CManager  
{
public:
	CVideoManager(CClientSocket* ClientObject) ;
	virtual ~CVideoManager();

	BOOL  m_bIsWorking;
	HANDLE m_hWorkThread;

	void SendBitMapInfor();
	BOOL SendNextScreen();
	static DWORD WorkThread(LPVOID lParam);

	CCaptureVideo  m_CapVideo;
	 void OnReceive(LPBYTE lpBuffer, UINT nSize);
	BOOL Initialize();

	DWORD	m_fccHandler;
	bool    m_bIsCompress;
	LPBYTE  lpBuffer; // ×¥Í¼»º´æÇø

	CVideoCodec	*m_pVideoCodec;   //Ñ¹ËõÀà
	void Destroy();
};

