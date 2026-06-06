#include "taskmanager.hpp"
#include <windows.h>
#include <shlwapi.h>
#include <string>
#include "source/utils/memory.hpp"

namespace features {
	extern bool task_manager_proof;
}

void TaskManagerHide(std::uint32_t tm_pid, bool exit) {
	static std::uint32_t last_pid = 0;
	static std::string dllPath;
	static HANDLE mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(DWORD), L"TaskManagerHide");
	static DWORD* shared = (DWORD*)MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD));
	if (!tm_pid || !features::task_manager_proof) {
		*shared = 0;
		if (!exit) Sleep(4320);
		mem::detach(1);
		last_pid = 0;
		return;
	}
	if (tm_pid != last_pid) {
		if (dllPath.empty()) {
			char path[MAX_PATH];
			GetModuleFileNameA(GetModuleHandle(nullptr), path, MAX_PATH);
			PathRemoveFileSpecA(path);
			PathCombineA(path, path, "taskmgr.dll");
			dllPath = path;
		}
		mem::attach_to_process("Taskmgr.exe");
		HANDLE hwnd = mem::get_process_handle(1);
		size_t len = dllPath.size() + 1;
		void* remote = nullptr;
		SIZE_T size = (SIZE_T)len;
		ntallocatevirtualmemory(hwnd, &remote, 0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		mem::write_bytes((std::uint64_t)(uintptr_t)remote, dllPath.c_str(), len, true);
		CreateRemoteThread(hwnd, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"), remote, 0, 0);
		SIZE_T freeSize = 0;
		ntfreevirtualmemory(hwnd, &remote, &freeSize, MEM_RELEASE);
		mem::detach(1);
		last_pid = tm_pid;
		Sleep(220);
	}
	*shared = GetCurrentProcessId();
}
