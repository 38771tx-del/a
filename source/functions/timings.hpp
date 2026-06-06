#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include "../utils/flat_hash_map.hpp"

struct Timings {
	struct Entry {
		std::uint16_t id;
		std::uint8_t tag;
		const char* anim_parry_cue;
		const char* sound_parry_cue;
		std::uint8_t sound_cue_dissapear;
		const char* stored_action;
		float range;
		float div_front;
		float div_left;
		float div_right;
		float div_back;
		float walkspeed;
		float time_position;
		std::uint16_t flags;
		std::uint8_t directions;
	};

	static constexpr std::uint16_t TimePosition = 1u;
	static constexpr std::uint16_t Animation = 2u;
	static constexpr std::uint16_t Sound = 4u;
	static constexpr std::uint16_t StoredAction = 8u;

	static constexpr std::uint8_t Front = 1u;
	static constexpr std::uint8_t Back = 2u;
	static constexpr std::uint8_t Left = 4u;
	static constexpr std::uint8_t Right = 8u;

	static constexpr std::uint8_t BasicAttack = 1u;
	static constexpr std::uint8_t Mantra = 2u;
	static constexpr std::uint8_t CriticalAttack = 3u;
	static constexpr std::uint8_t Vent = 4u;

	static Entry Entries[23];

	inline static ska::flat_hash_map<std::string_view, std::vector<std::size_t>> sound_index{};
	inline static ska::flat_hash_map<std::string_view, std::vector<std::size_t>> anim_index{};
	inline static std::vector<std::size_t> time_index{};

	static void Build();

	static void Search(
		const std::vector<std::string>& enemy_names,
		const std::vector<std::string>& anim_ids,
		const ska::flat_hash_set<std::size_t>& active_entries,
		std::vector<std::size_t>& out);
};

inline Timings::Entry Timings::Entries[23] = {
	{ 1, Timings::BasicAttack, "rbxassetid://7600224169", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 2, Timings::BasicAttack, "rbxassetid://6607519294", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 3, Timings::BasicAttack, "rbxassetid://114108670383026", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 4, Timings::BasicAttack, "rbxassetid://5064195992", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 5, Timings::BasicAttack, "rbxassetid://7600160919", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.600f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 6, Timings::BasicAttack, "rbxassetid://7626771915", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 7, Timings::BasicAttack, "rbxassetid://7627372304", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 8, Timings::BasicAttack, "rbxassetid://6675703249", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 9, Timings::BasicAttack, "rbxassetid://100190998225588", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 10, Timings::BasicAttack, "rbxassetid://6675698010", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 11, Timings::BasicAttack, "rbxassetid://122256431354649", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 12, Timings::BasicAttack, "rbxassetid://107653610777693", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 13, Timings::BasicAttack, "rbxassetid://6607538047", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 14, Timings::BasicAttack, "rbxassetid://138417702895229", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 15, Timings::BasicAttack, "rbxassetid://7627049402", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 16, Timings::BasicAttack, "rbxassetid://16372476897", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.500f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 17, Timings::BasicAttack, "rbxassetid://123456225328134", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 18, Timings::BasicAttack, "rbxassetid://5067090007", nullptr, 0, nullptr, 12.5f, 0.f, 0.f, 0.f, 0.f, 0.44f, 0.533f, (std::uint16_t)(Timings::Animation | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 19, Timings::Vent, "rbxassetid://9657469282", nullptr, 0, nullptr, 10.0f, 0.f, 0.f, 0.f, 0.f, 0.0f, 0.0f, Timings::Animation, (std::uint8_t)(Timings::Front | Timings::Back | Timings::Left | Timings::Right) },
	{ 20, Timings::Mantra, "rbxassetid://7861135817", "CLIENT_SOUND_110024038177151", 1, "Thunder Kick", 10.0f, 0.f, 0.f, 0.f, 0.f, 0.0f, 0.0f, (std::uint16_t)(Timings::Sound | Timings::Animation | Timings::StoredAction), (std::uint8_t)(Timings::Front | Timings::Left | Timings::Right) },
	{ 21, Timings::Mantra, nullptr, "REP_SOUND_4580495407", 0, "Punishment", 10.0f, 0.f, 0.f, 0.f, 0.f, 0.0f, 0.0f, (std::uint16_t)(Timings::Sound | Timings::StoredAction), (std::uint8_t)(Timings::Front | Timings::Back | Timings::Left | Timings::Right) },
	{ 22, Timings::Mantra, "rbxassetid://8857099063", nullptr, 0, "Storm Blades", 20.0f, 0.f, 0.f, 0.f, 0.f, 0.0f, 0.375f, (std::uint16_t)(Timings::Animation | Timings::StoredAction | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Back | Timings::Left | Timings::Right) },
	{ 23, Timings::Mantra, "rbxassetid://8198492537", nullptr, 0, "Exhaustion Strike", 15.0f, 0.f, 0.f, 0.f, 0.f, 0.0f, 0.350f, (std::uint16_t)(Timings::Animation | Timings::StoredAction | Timings::TimePosition), (std::uint8_t)(Timings::Front | Timings::Back | Timings::Left | Timings::Right) }
}; // (have yet to paste all the timings from lycoris)

inline void Timings::Build() {
	sound_index.clear();
	anim_index.clear();
	time_index.clear();
	for (std::size_t i = 0; i < sizeof(Entries) / sizeof(Entries[0]); ++i) {
		const Entry& e = Entries[i];
		if ((e.flags & Sound) && e.sound_parry_cue)
			sound_index[std::string_view(e.sound_parry_cue)].push_back(i);
		if ((e.flags & Animation) && e.anim_parry_cue)
			anim_index[std::string_view(e.anim_parry_cue)].push_back(i);
		if ((e.flags & TimePosition) && e.tag == Mantra && e.stored_action)
			time_index.push_back(i);
	}
}

inline void Timings::Search(
	const std::vector<std::string>& enemy_names,
	const std::vector<std::string>& anim_ids,
	const ska::flat_hash_set<std::size_t>& active_entries,
	std::vector<std::size_t>& out) {
	out.clear();
	out.reserve(enemy_names.size() + anim_ids.size() + active_entries.size() + 64u);
	for (const std::string& name : enemy_names) {
		auto find = sound_index.find(std::string_view(name));
		if (find != sound_index.end())
			out.insert(out.end(), find->second.begin(), find->second.end());
	}
	for (const std::string& id : anim_ids) {
		auto find = anim_index.find(std::string_view(id));
		if (find != anim_index.end())
			out.insert(out.end(), find->second.begin(), find->second.end());
	}
	out.insert(out.end(), active_entries.begin(), active_entries.end());
	std::sort(out.begin(), out.end());
	out.erase(std::unique(out.begin(), out.end()), out.end());
}
