#pragma once
#include "../../PublicInclude/Manager.h"
#include <taskschd.h>

class CTaskManager : public CManager
{
public:
	CTaskManager(CClientSocket* pClient);
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	~CTaskManager();
	PBYTE GetTaskAll(ITaskFolder* pFolder);							//获取目录下的所有任务
	PBYTE GetFolderAll(ITaskFolder* pFolder);							//获取目录下的子目录
	PBYTE GetRoot();																//获取根目录下的子文件夹

	BOOL CreateTask(LPBYTE lpBuffer);							//创建计划任务
	BOOL RunOrStopTask(LPBYTE lpBuffer, BOOL Action);  //执行或停止
	BOOL DelTask(LPBYTE lpBuffer);								//删除计划任务
	BOOL GetProgramPath(ITaskDefinition* iDefinition,BSTR* exepath);
	void SaveData(BSTR taskname, BSTR path, BSTR exepath, char* status, DATE LastTime, DATE NextTime);  //保存数据到缓冲区
private:
	ITaskService* pService = NULL;				//连接计划任务的
	PBYTE lpList = NULL;								//发送任务计划的数据
	DWORD offset = 0;								//偏移
	DWORD nBufferSize = 0;						//内存大小
};

