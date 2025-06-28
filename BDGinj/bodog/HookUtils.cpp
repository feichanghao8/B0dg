
#include <Windows.h>

// void FindSignature() {
// 	// Get .text region
// 	HMODULE base = GetModuleHandleA(NULL);
// 	IMAGE_DOS_HEADER* pDOSHeader = (IMAGE_DOS_HEADER*)base;
// 	IMAGE_NT_HEADERS* pNTHeaders = (IMAGE_NT_HEADERS*)((BYTE*)pDOSHeader + pDOSHeader->e_lfanew);
// 	
// 	uintptr_t codeStartP = reinterpret_cast<uintptr_t>(base);
// 	uintptr_t codeEndP = reinterpret_cast<uintptr_t>(base + pNTHeaders->OptionalHeader.SizeOfImage);
// }
