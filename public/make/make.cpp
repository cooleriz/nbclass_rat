#include"make.h"

int GetFileBuff(TCHAR* lpBuffer, TCHAR* lpFilename)
{
	DWORD BytesRead;

	HANDLE hDllFile = CreateFile(lpFilename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hDllFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDllFile);
		return NULL;
	}

	int FileSize = GetFileSize(hDllFile, 0);
	if (FileSize >= 200 *1024)
	{
		printf("文件过大\n");
		FileSize = 0;
		CloseHandle(hDllFile);
		return 0;
	}
	if (!ReadFile(hDllFile, lpBuffer, FileSize, &BytesRead, 0))
	{
		FileSize = 0;
	}
	CloseHandle(hDllFile);

	return FileSize;
}

int CreateFiled(TCHAR* szFilename, TCHAR* m_Shellcodes)
{
	DWORD dwwrsize = 0;
	HANDLE hbin = CreateFile(szFilename, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hbin != INVALID_HANDLE_VALUE)
	{
		BOOL rt = WriteFile(hbin, m_Shellcodes, sizeof(ShellcodeStr), &dwwrsize, NULL);
		if (rt)
		{
			
		}
		CloseHandle(hbin);
	}
	
	return dwwrsize;
}


int Make(TCHAR* szFilename, TCHAR* szLoader)
{
	ShellcodeStr m_Shellcodes;
	ZeroMemory(&m_Shellcodes, sizeof(ShellcodeStr));

	m_Shellcodes.dwFilelen = GetFileBuff((TCHAR*)m_Shellcodes.File, szFilename);
	m_Shellcodes.dwShellcodelen = GetFileBuff((TCHAR*)m_Shellcodes.Shellcode, szLoader);
	if (m_Shellcodes.dwFilelen && m_Shellcodes.dwShellcodelen)
	{
		TCHAR szSaveFile[MAX_PATH] = { 0, };
		_tcscat_s(szSaveFile, szFilename);
		_tcscat_s(szSaveFile, _T(".bin"));
		if (CreateFiled(szSaveFile, (TCHAR*)&m_Shellcodes))
		{
			printf("文件: %s 合成成功\n", szFilename);
			return 0;
		}
		else
		{
			printf("文件: %s 保存失败\n", szSaveFile);
		}
	}
	else
	{
		printf("文件: %s 合成失败\n", szFilename);
	}

	return 0;
}


BOOL  DirectoryList()
{
	DWORD																				FileCount = 0;													//记录文件个数
	WIN32_FIND_DATAA															FindData;															//文件结构
	CHAR																				    FilePathName[MAX_PATH] = "*.dll";			//要查找的文件
	TCHAR																					szLoader[MAX_PATH] = "loader.bin";				//shellcode文件名

	HANDLE hError = FindFirstFileA(FilePathName, &FindData);
	if (hError == INVALID_HANDLE_VALUE)
	{
		printf("搜索失败!\n");
		return 0;
	}

	//开始合并文件
	do
	{
		
		//合并文件
		if (Make(FindData.cFileName, szLoader))
		{
			FileCount++;
			DeleteFile(FindData.cFileName);
		}
		
	} while (::FindNextFile(hError, &FindData));

	printf("合并完成 总共生成%d个文件\n",FileCount);
	return 0;
}



int main()
{

	DirectoryList();
}


