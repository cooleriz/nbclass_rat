#pragma once
#include <windows.h>
#include<mbstring.h>
#include "../../common/CursorInfo.h"
#include "../../libjpeg/jpeglib.h"
#pragma comment(lib, "../../libjpeg/turbojpeg-static.lib")

// �����㷨
#define ALGORITHM_HOME	1	// ���ð칫�㷨
#define ALGORITHM_XVID	2	// Ӱ�������㷨

class CScreenSpy  
{
public:
	CScreenSpy(int biBitCount = 32, bool bIsGray = false, UINT nMaxFrameRate = 25);
	virtual ~CScreenSpy();
	LPVOID getFirstScreen(LPDWORD lpdwBytes);
	LPVOID getNextScreen(LPDWORD lpdwBytes);

	LPBITMAPINFO getBitmapInfo();
	UINT getBitmapInfoSize();
	void setAlgorithm(UINT nAlgorithm);
	void setCaptureLayer(BOOL bIsCaptureLayer);
	void SendUALITY75();
	void SendUALITY30();
	void SendUALITY100();
private:
	int    m_biBitCount;
	DWORD  m_dwBitBltRop;
	UINT   m_bAlgorithm;
	UINT   m_nMaxFrameRate;
	bool   m_bIsGray;
	int    m_nFullWidth;
	int    m_nFullHeight;
	BYTE   m_nIncSize;
	int    m_nScanLine;

	LARGE_INTEGER m_liFreq;
	LARGE_INTEGER m_liLast;
	LARGE_INTEGER m_liCurr;

	HDC    m_hDeskTopDC;
	HDC    m_hLastMemDC;
	HDC    m_hCurrMemDC;
	HDC    m_hRectMemDC;
	LPVOID m_lpvLastBits;
	LPVOID m_lpvCurrBits;
	LPVOID m_lpvRectBits;

	LPBITMAPINFO m_lpbmi_full;
	LPBITMAPINFO m_lpbmi_rect;
	HBITMAP m_hLastBitmap;
	HBITMAP m_hCurrBitmap;
	HBITMAP m_hRectBitmap;

	LPBYTE m_changedBuffer;
	UINT   m_changedOffset;
	int    m_nPerLineDataSize;
    int m_SendUALITY;

	//CXvidEnc m_XvidEnc;
	CCursorInfo	m_CursorInfo;
	LPBITMAPINFO ConstructBitmapInfo(int biBitCount, int biWidth, int biHeight);
	void WriteChangedBuffer(LPBYTE lpData, int nCount);
	BOOL ScanChangedRect(BOOL bCopyChangedRect);
	void CopyChangedRect(LPRGNDATA lpRgnData, DWORD dwRgnSize);
	int  BMP_JPG(int width, int height, int cbit, int quality, void *input, void **output);
	BOOL SelectInputWinStation();
};


