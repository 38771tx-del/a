/*
/*
#pragma once
#include "../utils/memory.hpp"
#include "../cache/sdk.hpp"

// credits to @99tracheae 
new method moved to main.cpp, sdk.hpp
inline void run_nofall(bool nofall) {
	static ULONGLONG dts = 0;
	static bool old = false;
	ULONGLONG now = (ULONGLONG)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	const bool cached = Cached.load(std::memory_order_acquire);
	if (!cached && !old) {
		if (now - dts < 440) return;
		dts = now;
		return;
	} else if (cached && nofall == old) return;
	if (Offset.Protos.size() != Offset.Bytecode.size() ||
		Offset.Protos.size() != Offset.Constants.size() ||
		Offset.Protos.size() != Offset.Code.size() ||
		Offset.Protos.size() != Offset.Constant.size())
		return;
	std::uint64_t patch[] = {
		Offset.NoFall, 0x500000000ULL,
		Offset.GetEffectsHash, 0x500000000ULL
	};
	bool write = false;
	for (std::size_t i = 0; i < Offset.Protos.size(); ++i) {
		const bool restore = old && (!nofall || !cached);
		mem::write_bytes(Offset.Constant[i], restore ? Offset.Constants[i].data() : reinterpret_cast<const std::uint8_t*>(patch), constants_size);
		mem::write_bytes(Offset.Code[i], restore ? Offset.Bytecode[i].data() : bytecode, bytecode_size);
		write = true;
	}
	if (write) old = nofall && cached;
}
*/
