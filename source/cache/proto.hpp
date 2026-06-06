/*
#pragma once
#define SZ_USE_HASWELL 1
#define SZ_DEBUG 0
#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <atomic>
#include <windows.h>
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"
#include "../string/stringzilla.h"

// credits to @99tracheae // https://99offsetz.bot.nu/api/pull?version=latest&author=99tracheae&query=Blob/Deepwoken/NoFall&type=json
inline const std::uint8_t bytecode[] = {
	0x90, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x80,
	0xBE, 0x02, 0x01, 0x00, 0xA2, 0x02, 0x02, 0x00,
	0x53, 0x03, 0x00, 0x3A, 0x01, 0x00, 0x00, 0x00,
	0x7E, 0x03, 0x02, 0x02, 0xD6, 0x02, 0x03, 0x01,
	0xA2, 0x02, 0x02, 0x00, 0x12, 0x02, 0x00, 0x00,
	0x80, 0x80, 0x52, 0x86, 0x12, 0x02, 0x00, 0x00,
};

inline constexpr std::size_t bytecode_size = sizeof(bytecode);
inline constexpr std::size_t constants_size = sizeof(std::uint64_t) * 4;

inline std::atomic<bool> Cached{false};

struct HasEffect {
	std::vector<std::uint64_t> Protos;
	std::uint64_t NoFall = 0;
	std::uint64_t GetEffectsHash = 0;
	std::vector<std::vector<std::uint8_t>> Bytecode;
	std::vector<std::vector<std::uint8_t>> Constants;
	std::vector<std::uint64_t> Code;
	std::vector<std::uint64_t> Constant;
};

inline bool run_proto(HasEffect& out) {
	if (Cached.load(std::memory_order_acquire)) return false;
	out = {};
	out.Protos.reserve(1024);
	out.Bytecode.reserve(1024);
	out.Constants.reserve(1024);
	if (!Lua::StringData || !Lua::GCObjectType || !Lua::TypeString || !Lua::ProtoDebugName) return false;

	HANDLE process = mem::get_process_handle(0);
	if (!process || process == INVALID_HANDLE_VALUE) return false;

	std::unordered_set<std::uint64_t> haseffect;
	std::vector<std::pair<std::uint64_t, std::size_t>> regions;
	std::vector<std::uint64_t> bases;
	std::vector<std::uint8_t> bytes;
	std::vector<std::uint64_t> adresses;
	bytes.reserve(1024 * 1024 + 13);
	adresses.reserve((1024 * 1024) / sizeof(std::uint64_t));
	haseffect.reserve(1024);

	auto decrypt = [&](std::uint64_t cur, std::uint64_t val) {
		if (Lua::ProtoDebugNameObfuscation == "NONE") return val;
		if (Lua::ProtoDebugNameObfuscation == "CMV") return cur - val;
		if (Lua::ProtoDebugNameObfuscation == "CXV") return cur ^ val;
		if (Lua::ProtoDebugNameObfuscation == "VMC") return val - cur;
		return cur + val;
	};

	auto lua = [&](std::uint64_t chunk, const std::uint8_t* hay, std::size_t len, std::uint64_t abs) {
		std::uint64_t type = abs - Lua::StringData + Lua::GCObjectType;
		std::uint8_t byte = 0xFF;
		std::uint64_t rel = type - chunk;
		if (rel < len) byte = hay[static_cast<std::size_t>(rel)];
		else mem::read_bytes(type, &byte, 1);
		return byte == Lua::TypeString;
	};

	auto collect = [&](const std::uint8_t* hay, std::size_t len, const char* needle, std::size_t length, std::uint64_t chunk) {
		std::size_t i = 0;
		while (i < len) {
			sz_cptr_t f = sz_find_haswell(reinterpret_cast<sz_cptr_t>(hay + i), len - i, reinterpret_cast<sz_cptr_t>(needle), length);
			if (!f) break;
			std::size_t idx = static_cast<std::size_t>(reinterpret_cast<const std::uint8_t*>(f) - hay);
			std::uint64_t str = (chunk + idx) - Lua::StringData;
			haseffect.insert(str);
			i = idx + 1;
		}
	};

	auto find = [&](const std::uint8_t* hay, std::size_t len, const char* needle, std::size_t length, std::uint64_t chunk, std::uint64_t& val) {
		if (val) return;
		std::size_t i = 0;
		while (i < len) {
			sz_cptr_t f = sz_find_haswell(reinterpret_cast<sz_cptr_t>(hay + i), len - i, reinterpret_cast<sz_cptr_t>(needle), length);
			if (!f) break;
			std::size_t idx = static_cast<std::size_t>(reinterpret_cast<const std::uint8_t*>(f) - hay);
			std::uint64_t abs = chunk + idx;
			if (lua(chunk, hay, len, abs)) {
				val = abs - Lua::StringData;
				return;
			}
			i = idx + 1;
		}
	};

	MEMORY_BASIC_INFORMATION mbi{};
	std::uint64_t cursor = 0;
	while (VirtualQueryEx(process, reinterpret_cast<void*>(cursor), &mbi, sizeof(mbi)) == sizeof(mbi)) {
		std::uint64_t base = reinterpret_cast<std::uint64_t>(mbi.BaseAddress);
		std::size_t size = static_cast<std::size_t>(mbi.RegionSize);
		cursor = base + size;
		if (cursor == 0) break;

		if (mbi.State != MEM_COMMIT) continue;
		if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS | PAGE_EXECUTE)) continue;
		if (((mbi.Protect & 0xFF) != PAGE_READWRITE) && ((mbi.Protect & 0xFF) != PAGE_WRITECOPY)) continue;
		if (size < 6) continue;

		std::size_t effect = haseffect.size();
		std::uint64_t get = out.GetEffectsHash;
		std::uint64_t nofall = out.NoFall;

		std::size_t offset = 0;
		std::size_t carry = 0;
		while (offset < size) {
			std::size_t read_size = 1024 * 1024;
			std::size_t remain = size - offset;
			if (read_size > remain) read_size = remain;
			if (bytes.size() < carry + read_size) bytes.resize(carry + read_size);
			if (!mem::read_bytes(base + offset, bytes.data() + carry, read_size)) break;

			const std::uint8_t* hay = bytes.data();
			std::size_t len = carry + read_size;
			std::uint64_t chunk = base + offset - carry;

			collect(hay, len, "HasEffect", 9, chunk);
			find(hay, len, "NoFall", 6, chunk, out.NoFall);
			find(hay, len, "GetEffectsHash", 14, chunk, out.GetEffectsHash);

			carry = (read_size < 13) ? read_size : 13;
			std::memmove(bytes.data(), bytes.data() + len - carry, carry);
			offset += read_size;
		}

		bool result = (haseffect.size() != effect) || (out.NoFall != nofall) || (out.GetEffectsHash != get);
		if (result && std::find(bases.begin(), bases.end(), base) == bases.end()) {
			bases.push_back(base);
			regions.push_back({base, size});
		}
	}

	if (out.NoFall == 0 || haseffect.empty()) return false;

	auto scan = [&](std::uint64_t base, std::size_t size) {
		if (size < sizeof(std::uint64_t)) return;
		std::size_t off = 0;
		while (off < size) {
			std::size_t read = 1024 * 1024;
			std::size_t remain = size - off;
			if (read > remain) read = remain;
			read -= (read % sizeof(std::uint64_t));
			if (!read) break;
			std::size_t count = read / sizeof(std::uint64_t);
			if (adresses.size() < count) adresses.resize(count);
			if (!mem::read_bytes(base + off, adresses.data(), read)) break;
			std::uint64_t current = base + off;
			for (std::size_t i = 0; i < count; ++i, current += sizeof(std::uint64_t)) {
				if (!haseffect.count(decrypt(current, adresses[i]))) continue;
				std::uint64_t proto = current - Lua::ProtoDebugName;
				if (!out.Protos.empty() && out.Protos.back() == proto) return;
				std::uint64_t code = mem::read<std::uint64_t>(proto + Lua::ProtoCode);
				std::uint64_t constants = mem::read<std::uint64_t>(proto + Lua::ProtoConstant);
				if (!code || !constants) return;
				std::vector<std::uint8_t> buffer(bytecode_size);
				std::vector<std::uint8_t> temp(constants_size);
				if (!mem::read_bytes(code, buffer.data(), buffer.size()) || !mem::read_bytes(constants, temp.data(), temp.size())) return;
				out.Protos.push_back(proto);
				out.Bytecode.push_back(std::move(buffer));
				out.Constants.push_back(std::move(temp));
				out.Code.push_back(code);
				out.Constant.push_back(constants);
				return;
			}
			off += read;
		}
	};

	for (const auto& r : regions) scan(r.first, r.second);
	bool ok = !out.Protos.empty();
	if (ok && out.NoFall && out.GetEffectsHash)
		Cached.store(true, std::memory_order_release);
	return ok;
}
*/


/*
static void ParseLua(std::string& json_str) {
	static simdjson::ondemand::parser parser;
	simdjson::padded_string_view padded = simdjson::pad(json_str);
	simdjson::ondemand::document doc;
	parser.iterate(padded).get(doc);
	simdjson::ondemand::object root;
	doc.get_object().get(root);

	auto get = [&](const char* key, std::uint64_t& out) {
		std::string_view digits{};
		root[key].get_string().get(digits);
		out = 0;
		std::from_chars(digits.data(), digits.data() + digits.size(), out, 16);
	};
	get("Lua/String/Data", Lua::StringData);
	get("Lua/GCObject/Type", Lua::GCObjectType);
	get("Lua/Type/String", Lua::TypeString);
	get("Lua/Proto/DebugName", Lua::ProtoDebugName);
	get("Lua/Proto/Constant", Lua::ProtoConstant);
	get("Lua/Proto/Code", Lua::ProtoCode);
	{ std::string_view sv{}; if (root["Lua/Proto/DebugNameObfuscation"].get_string().get(sv)) Lua::ProtoDebugNameObfuscation.assign(sv.data(), sv.size()); }
}

static std::string GetLua(void) {
	CURL* curl = curl_easy_init();
	if (!curl) return {};
	std::string response;
	auto write = +[](void* content, size_t size, size_t mem, void* out) -> size_t {
		size_t callback = size * mem;
		static_cast<std::string*>(out)->append(static_cast<char*>(content), callback);
		return callback;
	};
	std::string post = std::string("{\"version\":\"") + Offsets::ClientVersion + "\",\"author\":[\"*\"],\"query\":[\"Lua/String/Data\",\"Lua/GCObject/Type\",\"Lua/Type/String\",\"Lua/Proto/DebugName\",\"Lua/Proto/Constant\",\"Lua/Proto/DebugNameObfuscation\",\"Lua/Proto/Code\"],\"type\":\"json\"}"; // credits to @99tracheae
	curl_slist* handle = curl_slist_append(nullptr, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_URL, "https://99offsetz.bot.nu/api/pull");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)post.size());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, handle);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_perform(curl);
	long code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	curl_slist_free_all(handle);
	curl_easy_cleanup(curl);
	if (code != 200) response.clear();
	return response;
}

...

inline void Offsets::Fetch(void) {
	if (offsets_initialized) return;
	std::string body[2];
	const char* url[2] = { "https://imtheo.lol/Offsets/Offsets.json", "https://imtheo.lol/Offsets/FFlags.json" };
	CURL* handle[2] = { curl_easy_init(), curl_easy_init() };
	CURLM* multi = curl_multi_init();
	if (!multi) return;
	auto write = +[](void* content, size_t size, size_t mem, void* out) -> size_t {
		size_t callback = size * mem;
		static_cast<std::string*>(out)->append(static_cast<char*>(content), callback);
		return callback;
	};
	for (int i = 0; i < 2; ++i) {
		if (!handle[i]) continue;
		curl_easy_setopt(handle[i], CURLOPT_URL, url[i]);
		curl_easy_setopt(handle[i], CURLOPT_WRITEFUNCTION, write);
		curl_easy_setopt(handle[i], CURLOPT_WRITEDATA, &body[i]);
		curl_easy_setopt(handle[i], CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(handle[i], CURLOPT_SSL_VERIFYHOST, 0L);
		curl_multi_add_handle(multi, handle[i]);
	}
	int running;
	do {
		curl_multi_wait(multi, nullptr, 0, 440, nullptr);
		curl_multi_perform(multi, &running);
	} while (running);
	for (int i = 0; i < 2; ++i) {
		if (!handle[i]) continue;
		long code = 0;
		curl_easy_getinfo(handle[i], CURLINFO_RESPONSE_CODE, &code);
		if (code != 200) body[i].clear();
		curl_multi_remove_handle(multi, handle[i]);
		curl_easy_cleanup(handle[i]);
	}
	curl_multi_cleanup(multi);
	if (!body[0].empty()) { ParseOffsets(body[0]); offsets_initialized = true; }
	if (!body[1].empty()) ParseFFlags(body[1]);
	if (std::string body2 = GetLua(); !body2.empty()) ParseLua(body2);
}

struct Lua {
	static inline std::uint64_t StringData = 0;
	static inline std::uint64_t GCObjectType = 0;
	static inline std::uint64_t TypeString = 0;
	static inline std::uint64_t ProtoDebugName = 0;
	static inline std::uint64_t ProtoConstant = 0;
	static inline std::uint64_t ProtoCode = 0;
	static inline std::string ProtoDebugNameObfuscation{};
};
*/