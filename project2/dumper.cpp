// dumper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include <WinBase.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <fstream>
#include <string>
    
typedef DWORD(*_NtQueryInformationThread)(HANDLE, LONG, PVOID, ULONG, PULONG);
HMODULE ntdll = GetModuleHandle(TEXT("ntdll.dll"));

// self-explanatory
BOOL SuspendProcess(DWORD ProcessId, bool Suspend)
{
	HANDLE snHandle = NULL;
	BOOL rvBool = FALSE;
	THREADENTRY32 te32 = { 0 };

	snHandle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (snHandle == INVALID_HANDLE_VALUE) return (FALSE);

	te32.dwSize = sizeof(THREADENTRY32);
	if (Thread32First(snHandle, &te32))
	{
		do
		{
			if (te32.th32OwnerProcessID == ProcessId)
			{
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
				if (Suspend == false)
				{
					ResumeThread(hThread);
				}
				else
				{
					SuspendThread(hThread);
				}

				CloseHandle(hThread);
			}
		} while (Thread32Next(snHandle, &te32));
		rvBool = TRUE;
	}
	else
		rvBool = FALSE;
	CloseHandle(snHandle);
	return (rvBool);
}

INT64 getThreadBaseAddr(HANDLE thread) {
	_NtQueryInformationThread NtQueryInformationThread = (_NtQueryInformationThread)GetProcAddress(ntdll, "NtQueryInformationThread");
	LONG ThreadQuerySetWin32StartAddress = 0x09; 
	INT64 addr = 0;
	PVOID out = &addr;
	ULONG bytesWritten = 0;
	NtQueryInformationThread(thread, ThreadQuerySetWin32StartAddress, out, sizeof(PVOID), &bytesWritten);
	return (INT64)out;
}


DWORD suspendThreadsByBaseAddr(INT64 baseAddr, HANDLE owner) {
	
	HANDLE snHandle = NULL;
	BOOL rvBool = FALSE;
	THREADENTRY32 te32 = { 0 };

	snHandle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (snHandle == INVALID_HANDLE_VALUE) return (FALSE);

	te32.dwSize = sizeof(THREADENTRY32);
	if (Thread32First(snHandle, &te32))
	{
		do
		{
			if (te32.th32OwnerProcessID == GetProcessId(owner));
			{
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
				if (getThreadBaseAddr(hThread) == baseAddr) {
					SuspendThread(hThread);
				}

			}
		} while (Thread32Next(snHandle, &te32));
	}
	
}

HANDLE FindProcessHandle(std::wstring processName)
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (processName.compare(entry.szExeFile) == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
				return hProcess;
			}
		}
	}


	CloseHandle(snapshot);
	return 0;
}

LPMODULEINFO findNeededModule(HANDLE target, std::wstring name) {
	HMODULE mArray[512];
	DWORD bytesRead;
	WCHAR mName[MAX_PATH];
	LPMODULEINFO mInfo = (LPMODULEINFO)malloc(sizeof(MODULEINFO));
	if (EnumProcessModules(target, mArray, sizeof(mArray), &bytesRead)) {
		int count = bytesRead / sizeof(HMODULE);
		for (int i = 0; i < count; i++) {
			if (GetModuleFileNameEx(target, mArray[i], mName, MAX_PATH)) {
				if (name.compare(mName) == 0) {
					if (K32GetModuleInformation(target, mArray[i], mInfo, sizeof(MODULEINFO))) {
						return mInfo;
					}
				}
			}
		}
	}
	return 0;
}

static void sus2() {

}

static void sus(INT64* oofsets) {
	char* charz = (char*)(oofsets);
	while (true) {
		if (charz[5] != 0) {
			INT64 jvmAddr = oofsets[0];
			LPVOID parse_constant_pool_entries = (LPVOID)(jvmAddr + 0x1d5580);
			INT64 newInsns = (INT64)jvmAddr + 0x85e000;
			INT64 vals = (INT64)jvmAddr + 0x85e1b8;
			char patch1[] = {
				0xE9, 0x7b, 0x8a, 0x68, 0x00
			};
			char insns[] = {
				0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, <insert value here>
				0x48, 0x89, 0x48, 0x01,										// mov [rax+01], rcx
				0x4c, 0x89, 0x40, 0x09,										// mov [rax+09], r8
				0x48, 0x89, 0x50, 0x11,										// mov [rax+11], rdx
				0x90,														// nop
				0xeb, 0xfe													// jmp [rip-02]
			};

			char* funnn = (char*)parse_constant_pool_entries;
			INT64* exists = (INT64*)(parse_constant_pool_entries);
			char* newInsnsP = (char*)newInsns;
			if (*exists != 0) {
				int i = 0;
				char* newCode = (char*)newInsns;
				INT64 p1 = 0x000000000000b848;
				INT64 p2 = 0x894c014889480000;
				INT64 p3 = 0xeb90115089480940;
				char p4 = 0xfd;
				for (int i = 0; i < 5; i++) {
					funnn[i] = patch1[i];
				}
				newInsnsP[24] = 0xfd;
				INT64* newCodePtr = (INT64*)(newInsns);
				newCodePtr[0] = p1;
				newCodePtr[1] = p2;
				newCodePtr[3] = p3;
				((char*)newCodePtr)[24] = p4;
				long long* addr = (long long*)&newCode[2];
				addr[0] = vals;
			}


		}
	}
}


int main()
{
	DWORD dataOffset;
	std::string addr;
	char* p;
	char* clazzData;
	char* fun[5];
	char* streamDataP[8];
	char* streamData[512];
	INT64 targAddr;
	HANDLE decibatH = 0;

	while (decibatH == 0) {
		decibatH = FindProcessHandle(L"decibat-cli.exe");
	}
	std::cout << "Found the target!\n";


	//INT64 ntdll = (INT64)findNeededModule(decibatH, L"C:\\Windows\\System32\\ntdll.dll")->lpBaseOfDll;


	DWORD prev = 0;


	char jmp[] = {
		0xeb, 0xfe, 0x90, 0x90, 0x90
	};
	char threadPatch1[] = {
		0x3d, 0x00, 0x00, 0x00, 0x00, 0x90,
		0x74, 0x02,
		0xeb, 0x74,
		0xeb, 0xfe
	};
	char threadPatch2[] = {
		0x48, 0x89, 0x5c, 0x24, 0x10,
		0xeb, 0x92
	};


	INT64 targPage = (INT64)VirtualAllocEx(decibatH, NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE); //ntdll + 0x3e8;


	int ccs = 0;
	int l = 0;
	for (int i = 0; i < 0x0fffffff; i++) {
		char* hui = (char*)sus;
		int* ccP = (int*)&hui[i];
		ccs = *ccP;
		if (ccs == 0xcccccccc) {
			l = i;
			break;
		}
	}

	// патчи в jvm
	char patch1[] = {
		/*0xe9, 0xb4, 0x84, 0x00, 0x00*/
		0xE9, 0xab, 0x82, 0x68, 0x00						// jmp 
	};
	char insns[] = {
		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x48, 0x89, 0x48, 0x01,
		0x48, 0x89, 0x60, 0x09,
		0x48, 0x89, 0x48, 0x11,
		0xEB, 0xFE												// jmp [rip-02]
	};


	INT64 argAddr = targPage + l + 0x10;

	/*SuspendProcess(GetProcessId(decibatH), true);
	WriteProcessMemory(decibatH, (LPVOID)(targPage), (LPCVOID)sus, l, NULL);

	HANDLE th = CreateRemoteThread(decibatH, NULL, NULL, (LPTHREAD_START_ROUTINE)targPage, (LPVOID)(targPage + 0x200), 0x4, 0);
	if (th == 0) {
		printf("fuck");
	};
	SuspendProcess(GetProcessId(decibatH), false);
	*/

	// очевидно.
	while (findNeededModule(decibatH, L"l") == 0) {

	}
	HMODULE mArray[512];
	DWORD bytesRead;
	WCHAR mName[MAX_PATH];
	INT64 decibat;
	LPMODULEINFO mInfo = (LPMODULEINFO)malloc(sizeof(MODULEINFO));
	if (EnumProcessModules(decibatH, mArray, sizeof(mArray), &bytesRead)) {
		K32GetModuleInformation(decibatH, mArray[0], mInfo, sizeof(MODULEINFO));
		decibat = (INT64)mInfo->lpBaseOfDll;
	}
	else {
		return 0;
	}


	INT64 jvm = (INT64)findNeededModule(decibatH, L"")->lpBaseOfDll;
	INT64 target = jvm + 0x1d5d50;
	INT64 newInsns = (INT64)jvm + 0x85e000;
	INT64 vals = (INT64)jvm + 0x85e0ff;
	INT64 decibatOff = decibat + 0x35c300;
	INT64 kernel32 = (INT64)findNeededModule(decibatH, L"C:\\windows\\System32\\KERNEL32.DLL")->lpBaseOfDll;
	INT64 kernelTarget = kernel32 + 0x144b0;
	INT64 kTarget2 = kernel32 + 0x14515;
	INT64 jvmJarRead = jvm + 0x1E4CC0;

	INT64 defOff = decibat + 0x34a295;
	int decibatOffCut = (int)decibatOff;
	INT64 argsPtr = 0;
	INT64 streamPtr = 0;
	INT64 streamRealPtr = 0;
	INT64 streamBeginPtr = 0;
	INT64 streamEndPtr = 0;
	INT64 readdd = 0;
	INT64 vtable = 0;
	INT64 neededVtable = jvm + 0x705908;

	char thread1[] = {
		0xeb, 0xe5, 0x90
	};

	char thread2[] = {
		0x49, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x4d, 0x39, 0xc8,
		0x75, 0x62,
		0xe9, 0x8d, 0x00, 0x00, 0x00
	};

	char thread3[] = {
		0x4c, 0x8b, 0xdc,
		0xeb, 0xa5
	};

	char thread4[] = {
		0x49, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xeb, 0xc4
	};

	char returnSequence[] = {
		0xEB, 0x02,
		0x90,
		0x90,
		0x90,
		0x44, 0x89, 0x4c, 0x24, 0x20,
		0xe9, 0x30, 0x7d, 0x97, 0xff
	};

	char Pizda[] = {
		0x48, 0x89, 0x44, 0x24, 0xA0,
		0x48, 0x89, 0x5C, 0x24, 0x98,
		0x48, 0x89, 0x4C, 0x24, 0x90, 
		0x48, 0x89, 0x5C, 0x24, 0x88,
		0x4C, 0x89, 0x44, 0x24, 0x80,
		0x4C, 0x89, 0x8C, 0x24, 0x78, 0xFF, 0xFF, 0xFF,
		0x4C, 0x89, 0x94, 0x24, 0x70, 0xFF, 0xFF, 0xFF,
		0x4C, 0x89, 0x9C, 0x24, 0x68, 0xFF, 0xFF, 0xFF,
		0x4C, 0x89, 0xA4, 0x24, 0x60, 0xFF, 0xFF, 0xFF,
		0x4C, 0x89, 0xAC, 0x24, 0x58, 0xFF, 0xFF, 0xFF,
		0x4C, 0x89, 0xB4, 0x24, 0x50, 0xFF, 0xFF, 0xFF,
		0x4C, 0x89, 0xBC, 0x24, 0x48, 0xFF, 0xFF, 0xFF
	};

	char Kli_Tor[] = {
		0x48, 0x9c, 0x6c, 0x24, 0x18,
		0x39, 0xb8, 0x54, 0x98, 0xff
	};

	char Pizda2[] = {
		0xe9, 0xeb, 0xaa, 0x67, 0x00
	};
	//char zipOriginal
	/*INT64 zip;
	INT64 zipTarget = zip + 0x5000;
	INT64 zipNewCode = zip + 0xd4b9;
	INT64 zipVals = zip + 0xd540;*/
	Sleep(800);
	VirtualProtectEx(decibatH, (LPVOID)(jvmJarRead), 0x800, PAGE_EXECUTE_READWRITE, &prev);
	VirtualProtectEx(decibatH, (LPVOID)(kernelTarget - 0x28), 0x800, PAGE_EXECUTE_READWRITE, &prev);
	VirtualProtectEx(decibatH, (LPVOID)(newInsns), 0x800, PAGE_EXECUTE_READWRITE, &prev);
	VirtualProtectEx(decibatH, (LPVOID)(decibat + 0x34a295), 0x800, PAGE_EXECUTE_READWRITE, &prev);
	VirtualProtectEx(decibatH, (LPVOID)(target), 0x800, PAGE_EXECUTE_READWRITE, &prev);

	WriteProcessMemory(decibatH, (LPVOID)(target), (LPCVOID)patch1, 5, NULL);
	//WriteProcessMemory(decibatH, (LPVOID)(jvmJarRead), (LPCVOID)jmp, 2, NULL);

	WriteProcessMemory(decibatH, (LPVOID)(newInsns), (LPCVOID)insns, 24, NULL);
	WriteProcessMemory(decibatH, (LPVOID)(newInsns + 2), (LPCVOID)&vals, 8, NULL);

	/*WriteProcessMemory(decibatH, (LPVOID)(zipTarget), (LPCVOID)patch1, 5, NULL);
	WriteProcessMemory(decibatH, (LPVOID)(zipNewCode), (LPCVOID)insns, 24, NULL);
	WriteProcessMemory(decibatH, (LPVOID)(zipNewCode + 2), (LPCVOID)&zipVals, 8, NULL);*/
	int clazzIndex = 0;
	bool wrote = false;

	//SuspendProcess(GetProcessId(decibatH), true);

	while (true) {
		ReadProcessMemory(decibatH, (LPVOID)(vals + 0x9), (LPVOID)&argsPtr, 8, (SIZE_T*)&readdd);
		if (clazzIndex > 1 && !wrote) {
			WriteProcessMemory(decibatH, (LPVOID)(kTarget2), (LPCVOID)jmp, 2, NULL);
			WriteProcessMemory(decibatH, (LPVOID)(kernelTarget - 0x19), (LPCVOID)thread2, 20, NULL);
			WriteProcessMemory(decibatH, (LPVOID)(kernelTarget + 0x58), (LPCVOID)thread3, 5, NULL);
			WriteProcessMemory(decibatH, (LPVOID)(kernelTarget + 0x88), (LPCVOID)thread4, 12, NULL);
			WriteProcessMemory(decibatH, (LPVOID)(kernelTarget + 0x8a), (LPCVOID)&kTarget2, 8, NULL);
			WriteProcessMemory(decibatH, (LPVOID)(kernelTarget - 0x17), (LPCVOID)&decibatOff, 8, NULL);
			WriteProcessMemory(decibatH, (LPVOID)(kernelTarget), (LPCVOID)thread1, 3, NULL);
			wrote = true;
		}
		//;
		if (argsPtr != 0) {
			ReadProcessMemory(decibatH, (LPVOID)(argsPtr + 0x8), (LPVOID)&streamPtr, 8, (SIZE_T*)&readdd);
			ReadProcessMemory(decibatH, (LPVOID)(streamPtr + 0x8), (LPVOID)&streamBeginPtr, 8, (SIZE_T*)&readdd);
			ReadProcessMemory(decibatH, (LPVOID)(streamPtr + 0x10), (LPVOID)&streamEndPtr, 8, (SIZE_T*)&readdd);
			ReadProcessMemory(decibatH, (LPVOID)(streamPtr), (LPVOID)&vtable, 8, (SIZE_T*)&readdd);
			/*if (vtable != neededVtable) {
				ReadProcessMemory(decibatH, (LPVOID)(streamPtr + 0x8), (LPVOID)&streamRealPtr, 8, (SIZE_T*)&readdd);
				ReadProcessMemory(decibatH, (LPVOID)(streamRealPtr + 0x8), (LPVOID)&streamBeginPtr, 8, (SIZE_T*)&readdd);
				ReadProcessMemory(decibatH, (LPVOID)(streamRealPtr + 0x10), (LPVOID)&streamEndPtr, 8, (SIZE_T*)&readdd);
			}*/
			INT64 length = streamEndPtr - streamBeginPtr;
			char* clazz = (char*)malloc(length);
			ReadProcessMemory(decibatH, (LPVOID)(streamBeginPtr), (LPVOID)clazz, length, (SIZE_T*)&readdd);
			LONG magic = *(LONG*)clazz;
			if (magic == 0xffffffff) {
				std::string folder = "C:\\" + std::to_string(clazzIndex);
				std::ofstream outClazz = std::ofstream(folder + ".class");
				outClazz.write(clazz, length);
				outClazz.close();
				std::cout << "Dumped ||| class " << clazzIndex << " of length " << length << " and with magic number of " << std::hex << magic << std::endl;
				clazzIndex++;
				streamPtr = 0;
				WriteProcessMemory(decibatH, (LPVOID)(vals + 0x9), (LPCVOID)&streamPtr, 8, NULL);
			}

		}
		else {
			streamPtr = 0;
			WriteProcessMemory(decibatH, (LPVOID)(vals + 0x9), (LPCVOID)&streamPtr, 8, NULL);
			WriteProcessMemory(decibatH, (LPVOID)(newInsns + 0x16), (LPCVOID)returnSequence, 15, NULL);
			/*for (int i = 0; i < 10000000; i++) {

			}*/
			WriteProcessMemory(decibatH, (LPVOID)(newInsns + 0x16), (LPCVOID)jmp, 5, NULL);
		}
	}
}

	
	/*while (true) {
		HANDLE snHandle = NULL;
		BOOL rvBool = FALSE;
		THREADENTRY32 te32 = { 0 };

		snHandle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (snHandle == INVALID_HANDLE_VALUE) return (FALSE);

		te32.dwSize = sizeof(THREADENTRY32);
		if (Thread32First(snHandle, &te32))
		{
			do
			{
				if (te32.th32OwnerProcessID == GetProcessId(decibatH));
				{
					HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
					INT64 bA = getThreadBaseAddr(hThread);
					//std::cout << bA << std::endl;
					if (bA == decibatOff) {
						SuspendThread(hThread);   
					}

				}
			} while (Thread32Next(snHandle, &te32));
		}
	}*/  