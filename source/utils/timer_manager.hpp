#pragma once
#include <cstdint>
#include <windows.h>
#include <intrin.h>
#include "flat_hash_map.hpp"
#pragma intrinsic(_ReturnAddress)

class timer {
public:
	timer() = default;
	timer(const timer&) = delete;
	timer& operator=(const timer&) = delete;

	template<std::uint64_t ms>
	__declspec(noinline) bool wait() {
		const std::uintptr_t caller = caller_id();
		ULONGLONG& last = state<ms>()[caller];
		const ULONGLONG now = GetTickCount64();
		if (last != 0) {
			if (now - last < ms) return false;
			last = now;
			return true;
		}
		last = now;
		return false;
	}

private:
	static std::uintptr_t caller_id() {
		return reinterpret_cast<std::uintptr_t>(_ReturnAddress());
	}

	template<std::uint64_t ms>
	static ska::flat_hash_map<std::uintptr_t, ULONGLONG>& state() {
		static ska::flat_hash_map<std::uintptr_t, ULONGLONG> value;
		return value;
	}

};

inline timer task{};
