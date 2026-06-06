#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <array>

// pasted from @kitodoescode

extern "C" {
	uintptr_t ntreadvirtualmemory(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
	uintptr_t ntwritevirtualmemory(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
	uintptr_t ntallocatevirtualmemory(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
	uintptr_t ntfreevirtualmemory(HANDLE, PVOID*, PSIZE_T, ULONG);
}

struct pid_slot {
	std::string name;
	std::uint32_t pid = 0;
};

struct mem {
	static inline std::uint32_t process_id = 0;
	static inline uintptr_t base_address = 0;
	static inline HANDLE process_handle = nullptr;
	static inline std::array<HANDLE, 2> handle_slots{{nullptr, nullptr}};
	static inline std::array<pid_slot, 2> pid_slots;

	static std::uint32_t find_process_id(const std::string& process_name);
	static uintptr_t find_module_address(const std::string& module_name);
	static bool attach_to_process(const std::string& process_name);
	static void detach(std::size_t slot = 2);
	static void close_handle(HANDLE hwnd);
	static std::string read_string(uintptr_t addr);
	static bool read_bytes(uintptr_t addr, void* buffer, std::size_t size);
	static bool write_string(uintptr_t addr, const std::string& new_str);
	static bool write_bytes(uintptr_t addr, const void* buffer, std::size_t size, bool taskmgr = false);
	template <typename T> static T read(uintptr_t addr);
	template <typename T> static void write(uintptr_t addr, T value);
	static std::uint32_t get_process_id();
	static uintptr_t get_module_address();
	static HANDLE get_process_handle(std::size_t slot = 0);
};

union rbx_data {
	uint8_t raw[16];
	uintptr_t pointer;
};

struct rbx_string {
	rbx_data data;
	uintptr_t length;
	uintptr_t capacity;
};

inline std::uint32_t mem::find_process_id(const std::string& process_name) {
	std::uint32_t out = 0;
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE) return 0;
	PROCESSENTRY32 pe = {}; pe.dwSize = sizeof(pe);
	if (Process32First(snap, &pe)) {
		do {
			if (process_name == pe.szExeFile) {
				out = pe.th32ProcessID;
				if (process_name == "Taskmgr.exe") {
					pid_slots[1].name = process_name;
					pid_slots[1].pid = out;
				} else {
					pid_slots[0].name = process_name;
					pid_slots[0].pid = out;
				}
				break;
			}
		} while (Process32Next(snap, &pe));
	}
	::CloseHandle(snap);
	return out;
}

inline uintptr_t mem::find_module_address(const std::string& module_name) {
	uintptr_t out = 0;
	if (!process_handle) return 0;
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
	if (snap == INVALID_HANDLE_VALUE) return 0;
	MODULEENTRY32 me = {}; me.dwSize = sizeof(me);
	if (Module32First(snap, &me)) {
		do {
			if (module_name == me.szModule) {
				base_address = out = reinterpret_cast<uintptr_t>(me.modBaseAddr);
				break;
			}
		} while (Module32Next(snap, &me));
	}
	::CloseHandle(snap);
	return out;
}

inline bool mem::attach_to_process(const std::string& process_name) {
	std::uint32_t pid = find_process_id(process_name);
	if (!pid) return false;
	std::size_t i = (process_name == "Taskmgr.exe") ? 1 : 0;
	detach(i);
	HANDLE hwnd = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	handle_slots[i] = (hwnd != INVALID_HANDLE_VALUE) ? hwnd : nullptr;
	if (i == 0) { process_handle = handle_slots[0]; process_id = pid; }
	return (handle_slots[i] != nullptr);
}

inline void mem::close_handle(HANDLE hwnd) {
	if (hwnd != INVALID_HANDLE_VALUE) ::CloseHandle(hwnd);
}

inline void mem::detach(std::size_t slot) {
	if (slot >= 2) {
		close_handle(handle_slots[0]);
		handle_slots[0] = nullptr;
		close_handle(handle_slots[1]);
		handle_slots[1] = nullptr;
		process_handle = nullptr;
		process_id = 0;
		base_address = 0;
		pid_slots[0].pid = 0;
		pid_slots[1].pid = 0;
	} else {
		close_handle(handle_slots[slot]);
		handle_slots[slot] = nullptr;
		if (slot == 0) {
			process_handle = nullptr;
			process_id = 0;
			base_address = 0;
			pid_slots[0].pid = 0;
		}
	}
}

template <typename T>
__forceinline T mem::read(uintptr_t addr) {
	T buffer{};
	SIZE_T bytesRead = 0;
	ntreadvirtualmemory(process_handle, reinterpret_cast<void*>(addr), &buffer, sizeof(T), &bytesRead);
	return buffer;
}

inline bool mem::read_bytes(uintptr_t addr, void* buffer, std::size_t size) {
	SIZE_T bytesRead = 0;
	return ntreadvirtualmemory(process_handle, reinterpret_cast<void*>(addr), buffer, size, &bytesRead) == 0;
}

inline std::string mem::read_string(uintptr_t addr) {
	auto len = mem::read<uint64_t>(addr + 0x10);
	if (len > 15) addr = mem::read<uint64_t>(addr);
	std::string str; str.reserve((size_t)(len));
	for (size_t i = 0; i < (size_t)(len); ++i) { auto ch = mem::read<char>(addr + i); if (ch == '\0') break; str.push_back(ch); }
	return str;
}

template <typename T>
__forceinline void mem::write(uintptr_t addr, T value) {
	SIZE_T bytesWritten = 0;
	ntwritevirtualmemory(process_handle, reinterpret_cast<void*>(addr), &value, sizeof(T), &bytesWritten);
}

inline bool mem::write_bytes(uintptr_t addr, const void* buffer, std::size_t size, bool taskmgr) {
	HANDLE process = taskmgr ? handle_slots[1] : process_handle;
	if (!process) return false;
	SIZE_T bytesWritten = 0;
	return ntwritevirtualmemory(process, reinterpret_cast<void*>(addr), const_cast<void*>(buffer), size, &bytesWritten) == 0;
}

inline bool mem::write_string(uintptr_t addr, const std::string& value) {
	auto str = mem::read<rbx_string>(addr);
	uintptr_t len = (uintptr_t)value.size();
	if (len > str.capacity) {
		while (len > str.capacity) {
			str.capacity *= 2;
			str.capacity += 1;
		}
		void* base = nullptr;
		SIZE_T size = (SIZE_T)str.capacity;
		if (ntallocatevirtualmemory(process_handle, &base, 0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) != 0) return false;
		str.data.pointer = (uintptr_t)base;
	}
	str.length = len;
	if (str.length > 15) {
		mem::write<rbx_string>(addr, str);
		addr = str.data.pointer;
	} else {
		str.capacity = 15;
		mem::write<rbx_string>(addr, str);
	}
	mem::write_bytes(addr, value.c_str(), len);
	mem::write<char>(addr + len, 0);
	return true;
}

__forceinline std::uint32_t mem::get_process_id() { return process_id; }
__forceinline uintptr_t mem::get_module_address() { return base_address; }
__forceinline HANDLE mem::get_process_handle(std::size_t slot) { return slot < 2 ? handle_slots[slot] : nullptr; }
