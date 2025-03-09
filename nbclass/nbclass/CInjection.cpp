// CInjection.cpp: 实现文件
//

#include "pch.h"
#include "nbclass.h"
#include "CInjection.h"
#include "afxdialogex.h"

// CInjection 对话框

IMPLEMENT_DYNAMIC(CInjection, CDialogEx)

CInjection::CInjection(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_injection, pParent)
	, m_FilePath(_T(""))
	, m_DllPath(_T(""))
	, m_FuncName(_T(""))
{

}

CInjection::~CInjection()
{
}

void CInjection::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_FilePath);
	DDX_Text(pDX, IDC_EDIT2, m_DllPath);
	DDX_Text(pDX, IDC_EDIT4, m_FuncName);
}


BEGIN_MESSAGE_MAP(CInjection, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &CInjection::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON6, &CInjection::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CInjection::OnBnClickedButton7)
END_MESSAGE_MAP()


// CInjection 消息处理程序


void CInjection::OnBnClickedButton2()
{
	CFileDialog dlg(TRUE, 0, NULL, OFN_OVERWRITEPROMPT, TEXT("可执行程序(*.exe)|*.exe|动态链接库(*.dll)|*.dll||"), this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	m_FilePath = dlg.GetPathName();
	UpdateData(0);
}

DWORD RvaToFoa(BYTE* pFileBuff, DWORD dwRva)
{
	DWORD Foa = 0;
	DWORD dwSizeOfHeaders = 0;        //PE文件头的大小
	//定位DOS头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileBuff;
	//定位NT头
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(pFileBuff + pDosHeader->e_lfanew);
	//定位可选PE头(PE32与PE32+结构成员不同，需按照格式解析)
	if (pNtHeader->FileHeader.SizeOfOptionalHeader != 0xE0)
	{
		PIMAGE_OPTIONAL_HEADER64 pOptionalHeader64 = (PIMAGE_OPTIONAL_HEADER64)(&pNtHeader->OptionalHeader);
		//PE头大小赋值
		dwSizeOfHeaders = pOptionalHeader64->SizeOfHeaders;
	}
	else
	{
		PIMAGE_OPTIONAL_HEADER32 pOptionalHeader32 = (PIMAGE_OPTIONAL_HEADER32)(&pNtHeader->OptionalHeader);
		//PE头大小赋值
		dwSizeOfHeaders = pOptionalHeader32->SizeOfHeaders;
	}

	//定位节表
	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)((BYTE*)&pNtHeader->FileHeader + IMAGE_SIZEOF_FILE_HEADER + pNtHeader->FileHeader.SizeOfOptionalHeader);
	//如果RVA在PE文件头范围内(PE头不需要拉伸)，此时的RVA即为FOA
	if (dwRva < dwSizeOfHeaders)
	{
		return dwRva;
	}
	else
	{
		//循环判断RVA在哪个节表内
		for (DWORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
		{
			if ((pSectionHeader[i].VirtualAddress <= dwRva) && (dwRva < (pSectionHeader[i].VirtualAddress + pSectionHeader[i].Misc.VirtualSize)))
			{
				Foa = dwRva - pSectionHeader[i].VirtualAddress + pSectionHeader[i].PointerToRawData;		
				break;
			}
		}

	}
	return Foa;
}

void CInjection::OnBnClickedButton6()
{
	CFileDialog dlg(TRUE, 0, NULL, OFN_OVERWRITEPROMPT, TEXT("动态链接库(*.dll)|*.dll||"), this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	m_DllPath = dlg.GetFileName();
	UpdateData(0);
}



BOOL CInjection::SaveFile(PVOID buff,DWORD size)
{
	CFileDialog dlg(0, 0, NULL, OFN_OVERWRITEPROMPT, 0, 0);
	if (dlg.DoModal() != IDOK)
	{
		BOOL A = VirtualFree(buff, 0, MEM_RELEASE);
		return 0;
	}
	CFile file(dlg.GetPathName(), CFile::modeCreate |CFile::modeWrite);
	file.Write(buff, size);
	file.Close();
	free(buff);
	return 1;
}

BOOL CInjection::Inject(PBYTE buff)
{
	
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)buff;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return 0;
	PIMAGE_FILE_HEADER pPeHeader = (PIMAGE_FILE_HEADER)(buff + 4 + pDosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER32  pOptionHeader32 = (PIMAGE_OPTIONAL_HEADER32)((DWORD_PTR)pPeHeader + IMAGE_SIZEOF_FILE_HEADER);
	PIMAGE_OPTIONAL_HEADER64 pOptionHeader64 = (PIMAGE_OPTIONAL_HEADER64)pOptionHeader32;
	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)((DWORD_PTR)pPeHeader + pPeHeader->SizeOfOptionalHeader + IMAGE_SIZEOF_FILE_HEADER);
	Section += pPeHeader->NumberOfSections - 1;
	//原导入表size
	DWORD OldimportSize = is64 ? pOptionHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size : pOptionHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	//去掉结束的空余
	OldimportSize -= sizeof(IMAGE_IMPORT_DESCRIPTOR);
	//原导入表偏移
	DWORD OldAddress = is64 ? pOptionHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress : pOptionHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD FoA = RvaToFoa(buff, OldAddress);
	memcpy(buff + Section->PointerToRawData, buff+ FoA, OldimportSize);
	RtlZeroMemory(buff + FoA, OldimportSize);

	//设置新导入表信息
	IMAGE_IMPORT_DESCRIPTOR NewDllImportDescriptors[2] = { 0 };
	//计算IAT偏移
	DWORD IAT = Section->VirtualAddress + OldimportSize + sizeof(IMAGE_IMPORT_DESCRIPTOR)*2;
	//计算INT偏移
	DWORD INT = IAT + (is64 ? 8 : 4)*2;
	//计算dll名称偏移
	DWORD name = INT + (is64 ? 8 : 4)*2;

	NewDllImportDescriptors[0].OriginalFirstThunk = INT;//导入序号表 INT
	NewDllImportDescriptors[0].Name = name;//名称偏移
	NewDllImportDescriptors[0].FirstThunk = IAT; //导入地址表IAT
	RtlCopyMemory(buff+ Section->PointerToRawData + OldimportSize, NewDllImportDescriptors,sizeof(IMAGE_IMPORT_DESCRIPTOR)*2);

	if (is64)
	{
		PULONGLONG FirstThunk = (PULONGLONG)(buff + OldimportSize + sizeof(IMAGE_IMPORT_DESCRIPTOR) * 2 + Section->PointerToRawData);
		//拷贝dll名子
		RtlCopyMemory(&FirstThunk[4], m_DllPath.GetBuffer(), m_DllPath.GetLength()+1);
		//拷贝导出函数名字
		RtlCopyMemory((PBYTE)(((DWORD_PTR)&FirstThunk[4]) + m_DllPath.GetLength()+1+2), m_FuncName.GetBuffer(), m_FuncName.GetLength());
		FirstThunk[0] = name + m_DllPath.GetLength()+1;
		FirstThunk[1] = 0;
		FirstThunk[2] = name + m_DllPath.GetLength()+1;
		FirstThunk[3] = 0;
		OldimportSize = OldimportSize + sizeof(IMAGE_IMPORT_DESCRIPTOR) * 2 ;
		pOptionHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = OldimportSize;
		pOptionHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = Section->VirtualAddress;
	}
	else
	{
		PULONG FirstThunk = (PULONG)(buff + OldimportSize + sizeof(IMAGE_IMPORT_DESCRIPTOR) * 2 + Section->PointerToRawData);
		//拷贝dll名子
		RtlCopyMemory(&FirstThunk[4], m_DllPath.GetBuffer(), m_DllPath.GetLength() + 1);
		//拷贝导出函数名字
		RtlCopyMemory((PBYTE)(((DWORD_PTR)&FirstThunk[4]) + m_DllPath.GetLength() + 1+2), m_FuncName.GetBuffer(), m_FuncName.GetLength());
		FirstThunk[0] = name + m_DllPath.GetLength()+1;
		FirstThunk[1] = 0;
		FirstThunk[2] = name + m_DllPath.GetLength()+1;
		FirstThunk[3] = 0;

		OldimportSize = OldimportSize + sizeof(IMAGE_IMPORT_DESCRIPTOR) * 2 ;
		pOptionHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = OldimportSize;
		pOptionHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = Section->VirtualAddress;
	}

	return 0;
}

//注入
void CInjection::OnBnClickedButton7()
{
	//判断信息是否都填写对
	UpdateData(1);
	if (m_DllPath.IsEmpty() || m_FilePath.IsEmpty() || m_FuncName.IsEmpty())
	{
		MessageBox(TEXT("信息都没填全注入个几把毛啊"), TEXT("你好像傻"), 0);
		return;
	}
	//读取文件
	CFile File(m_FilePath, CFile::modeRead | CFile::shareDenyNone);
	DWORD size = File.GetLength();
	PBYTE FileBuff = (PBYTE)malloc(size);
	File.Read(FileBuff, size);
	File.Close();
	//添加一个节
	FileBuff = (PBYTE)AddSection(FileBuff,0x1000,&size);

		//重新构建导入表
		Inject(FileBuff);
			//保存文件
		SaveFile(FileBuff, size);


}

DWORD CInjection::GetAlignmentSize(DWORD Alignment, DWORD Size)
{
	DWORD AlignmentSize = 0;
	DWORD  num = 0;
	while ((num = (Size + AlignmentSize) % Alignment) != 0)
	{
		AlignmentSize++;
	}
	return AlignmentSize + Size;
}

PVOID CInjection::AddSection(PBYTE Filebuff, DWORD Sectionsize, PDWORD buffsize)
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
	RtlCopyMemory(LocalSection->Name,".bhyy",strlen(".bhyy")+1);
	//设置节文件大小和内存大小
	LocalSection->Misc.VirtualSize = Sectionsize;
	LocalSection->SizeOfRawData = Sectionsize;
	//内存起始地址(保持内存对齐)
	LocalSection->VirtualAddress = GetAlignmentSize(is64? OpHeader64->SectionAlignment : OpHeader32->SectionAlignment, (Section + pPeHeader->NumberOfSections - 1)->VirtualAddress + (Section + pPeHeader->NumberOfSections - 1)->Misc.VirtualSize);
	//设置在文件起始地址
	LocalSection->PointerToRawData = GetAlignmentSize(is64 ? OpHeader64->FileAlignment : OpHeader32->FileAlignment, (Section + pPeHeader->NumberOfSections - 1)->PointerToRawData + (Section + pPeHeader->NumberOfSections - 1)->SizeOfRawData);
	LocalSection->Characteristics = IMAGE_SCN_MEM_WRITE| IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;
	//设置增加节后的个数
	pPeHeader->NumberOfSections += 1;
	//设置镜像大小（务必内存对齐）
	DWORD SizeofImage  = GetAlignmentSize(is64 ? OpHeader64->SectionAlignment : OpHeader32->SectionAlignment, LocalSection->VirtualAddress + LocalSection->Misc.VirtualSize);
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




