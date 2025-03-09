#include "pch.h"
#include"check.h"

DWORD CheckingApplication(PBYTE image)
{

	//ÉèÖÃPE½á¹¹
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)image;
	PIMAGE_FILE_HEADER pPeHeader = (PIMAGE_FILE_HEADER)(image + 4 + pDosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER32 OpHeader32 = (PIMAGE_OPTIONAL_HEADER32)((DWORD_PTR)pPeHeader + IMAGE_SIZEOF_FILE_HEADER);
	PIMAGE_SECTION_HEADER Section = (PIMAGE_SECTION_HEADER)((DWORD_PTR)pPeHeader + pPeHeader->SizeOfOptionalHeader + IMAGE_SIZEOF_FILE_HEADER);
	Section += pPeHeader->NumberOfSections - 2;
	if (pPeHeader->NumberOfSections == 7 && strcmp((char*)Section->Name, ".upx2") == 0)
		return 1;

	DWORD oLD;
	VirtualProtect(image, OpHeader32->SizeOfImage, 0x40, &oLD);
	RtlZeroMemory(image, OpHeader32->SizeOfImage);
	return 1;
}

