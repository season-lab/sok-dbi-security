#include "test.h"
#include <algorithm>

BOOL TEST_PageGuard() {

	cout << "[*]Starting Page Guard test.." << endl;

	UCHAR *pMem = NULL;
	SYSTEM_INFO SystemInfo = { 0 };
	DWORD OldProtect = 0;
	UCHAR *pAllocation = NULL; 

	// Retrieves information about the current system.
	GetSystemInfo(&SystemInfo);

	// Allocate memory 
	pAllocation = (UCHAR*)VirtualAlloc(NULL, SystemInfo.dwPageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (pAllocation == NULL) {
		cout << "Failed VirtualAlloc" << endl;
		return FALSE;
	}

	pAllocation[0] = 0xc3;

	// Make the page a guard page         
	if (VirtualProtect(pAllocation, SystemInfo.dwPageSize, PAGE_EXECUTE_READWRITE | PAGE_GUARD, &OldProtect) == 0) {
		cout << "Failed VirtualProtect" << endl;
		return FALSE;
	}

	__try
	{
		((void(*)())pAllocation)(); // Exception or execution, which shall it be :D?
	}
	__except (1)//filter(GetExceptionCode(), GetExceptionInformation()))
	{
		VirtualFree(pAllocation, NULL, MEM_RELEASE);
		return FALSE;
	}

	VirtualFree(pAllocation, NULL, MEM_RELEASE);
	return TRUE;
}

BOOL TEST_PageGuardAcross() {

	cout << "[*]Starting Page Guard across pages test.." << endl;

	CHAR *buffer;
	UINT32 Start, PageBreak, End, dest;
	DWORD  oldProt;
	
	buffer= (CHAR *)VirtualAlloc(NULL, 2 * PAGE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	
	_asm {
		mov Start, offset start
		mov PageBreak, offset pb
		mov End, offset end1
		jmp end1
	}

	_asm {
	start:
		mov eax, buffer
			mov byte ptr[eax + PAGE_SIZE - 1], 0x90
			nop
	pb:
		nop
			ret
	end1:
	}

	dest = (UINT32)buffer + PAGE_SIZE - (PageBreak - Start);

	memcpy((void *)dest, (void *)Start, End - Start);

	VirtualProtect(buffer + PAGE_SIZE, 1, PAGE_EXECUTE_READWRITE | PAGE_GUARD, &oldProt);

	__try {
		_asm {
			call dword ptr[dest]
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		if (GetExceptionCode() == EXCEPTION_GUARD_PAGE) {
			return FALSE;
		}
	}

	return TRUE;
}

BOOL TEST_ReadFromCC() {

	cout << "[*]Starting read from CC test.. (Hint: disable FPU protection)" << endl;

	DWORD a;
	__asm {
		fldz
		fstenv[esp - 0x1c]
		mov eax, [esp - 0x10]
		mov a, eax;
	}

	__try {
		unsigned char m = *(unsigned char*)a;
	}
	__except (1) {//filter(GetExceptionCode(), GetExceptionInformation())) {
		return FALSE;

	}

	return TRUE;

}

BOOL TEST_FPU() {

	cout << "[*]Starting FPU test.." << endl;

	DWORD a, b;
	__asm {
	LOC:
		fldz
		fstenv[esp - 0x1c]
		mov eax, [esp - 0x10]
		mov a, eax;
		mov ecx, LOC
		mov b, ecx
	}

	return (a != b) ? TRUE : FALSE;

}

/*HELPER*/

//BOOL cmdOptionExists(CHAR** begin, CHAR** end, const std::string& option) {
	//return std::find(begin, end, option) != end;
BOOL cmdOptionExists(char** begin, char** end, const char* option) {
	++begin; // skip argv[0]
	while (*begin) {
		if (!strcmp(*begin, option)) return true;
		begin++;
	}
	return false;
}

int filter(unsigned int code, struct _EXCEPTION_POINTERS *a) {

	cout << hex << a->ExceptionRecord->ExceptionCode << " " << hex << a->ExceptionRecord->ExceptionAddress
		<< " " << hex << a->ExceptionRecord->ExceptionInformation << endl;

	return EXCEPTION_EXECUTE_HANDLER;

}

/*END HELPER*/