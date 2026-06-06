#include "autoparry.hpp"
#include <cstdio>
#include <cmath>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <windows.h>
#include "timings.hpp"
#include "../utils/timer_manager.hpp"
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"
#include "../cache/sdk.hpp"
#include "../cache/player.hpp"
#include "../utils/flat_hash_map.hpp"

struct Enemy {
	std::uint64_t humanoidrootpart{ 0 };
	std::uint64_t character{ 0 };
	float distance{ 0 };
	float walkspeed{ 0 };
	unsigned char direction{ 0 };
};

struct State {
	Enemy enemy{};
	int has_target = 0;
	ska::flat_hash_set<std::size_t> active_entries;
	ska::flat_hash_set<std::size_t> triggered_entries;
	std::vector<std::uint64_t> anim_tracks;
	std::vector<std::string> anim_ids;
	std::string stored_m1;
	bool m1_armed = false;
};

static State state;

static std::vector<std::string> instances;
static std::vector<std::size_t> indexes;
static std::vector<std::uint64_t> children;
static ska::flat_hash_map<std::string_view, std::size_t> active_anim;
static ska::flat_hash_set<std::string_view> active_name;

static void Parry(void) {
	INPUT input[2] = {};
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wScan = 0x21;
	input[0].ki.dwFlags = KEYEVENTF_SCANCODE;
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wScan = 0x21;
	input[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(2, input, sizeof(INPUT));
}

void autoparry_run(bool enabled, std::uint32_t tag_mask) {
	Player::AutoParry(enabled);
	if (enabled) Player::Start();

	if (!enabled || !tag_mask || !SDK::Window().foreground || SDK::UIActive().chat || SDK::UIActive().menu) {
		state.active_entries.clear();
		state.triggered_entries.clear();
		state.stored_m1.clear();
		state.m1_armed = false;
		return;
	}

	if (!task.wait<16>())
		return;

	const auto players = Player::GetPlayers();

	const PlayerObject* closest = nullptr;
	for (const PlayerObject& player : players) {
		if (!closest || player.distance < closest->distance) closest = &player;
	}

	if (state.has_target && state.enemy.character) {
		const PlayerObject* tracked = nullptr;
		for (const PlayerObject& player : players) {
			if (player.character == state.enemy.character) {
				tracked = &player;
				break;
			}
		}
		if (!tracked)
			state.has_target = 0;
		else if (closest && closest->distance < tracked->distance) {
			state.enemy.humanoidrootpart = closest->humanoidrootpart;
			state.enemy.character = closest->character;
			state.enemy.distance = closest->distance;
			state.enemy.walkspeed = closest->walkspeed;
			state.enemy.direction = closest->direction;
		} else {
			state.enemy.humanoidrootpart = tracked->humanoidrootpart;
			state.enemy.character = tracked->character;
			state.enemy.distance = tracked->distance;
			state.enemy.walkspeed = tracked->walkspeed;
			state.enemy.direction = tracked->direction;
		}
	}

	if (!state.has_target && closest) {
		state.enemy.humanoidrootpart = closest->humanoidrootpart;
		state.enemy.character = closest->character;
		state.enemy.distance = closest->distance;
		state.enemy.walkspeed = closest->walkspeed;
		state.enemy.direction = closest->direction;
		state.has_target = 1;
	}

	if (!state.has_target) {
		state.active_entries.clear();
		state.triggered_entries.clear();
		return;
	}

	Instance enemy_hrp{ state.enemy.humanoidrootpart };
	Instance enemy_char{ state.enemy.character };

	instances.clear();
	children.clear();
	if (state.enemy.humanoidrootpart) {
		for (Instance& child : enemy_hrp.GetChildren()) {
			children.push_back(child.address);
			instances.push_back(child.GetInstanceName());
		}
	}

	active_name.clear();
	active_name.reserve(instances.size());
	for (const std::string& name : instances)
		active_name.emplace(std::string_view(name));

	state.anim_tracks.clear();
	state.anim_ids.clear();
	if (state.enemy.character) {
		std::uint64_t animator = enemy_char.FindFirstChildOfClass("Humanoid").FindFirstChildOfClass("Animator").address;
		std::uint64_t list_head = mem::read<std::uint64_t>(animator + Offsets::Animator::ActiveAnimations);
		std::uint64_t node = mem::read<std::uint64_t>(list_head);
		while (node && node != list_head) {
			std::uint64_t track = mem::read<std::uint64_t>(node + 0x10);
			std::uint64_t anim = mem::read<std::uint64_t>(track + Offsets::AnimationTrack::Animation);
			state.anim_tracks.push_back(track);
			state.anim_ids.push_back(mem::read_string(anim + Offsets::Misc::AnimationId));
			node = mem::read<std::uint64_t>(node);
		}
	}

	active_anim.clear();
	active_anim.reserve(state.anim_ids.size());
	for (std::size_t i = 0; i < state.anim_ids.size(); ++i)
		active_anim.emplace(std::string_view(state.anim_ids[i]), i);

	if (state.enemy.character) {
		std::string action_now = enemy_char.GetAttribute("StoredAction");
		if (state.stored_m1.empty()) {
			state.stored_m1 = action_now;
			state.m1_armed = false;
		} else if (action_now != state.stored_m1) {
			state.stored_m1 = action_now;
			state.m1_armed = true;
		} else {
			state.m1_armed = false;
		}
	}

	Timings::Search(instances, state.anim_ids, state.active_entries, indexes);
	if (indexes.empty()) {
		std::printf("no matching anim id found\n");
		for (const std::string& id : state.anim_ids)
			std::printf("%s\n", id.c_str());
		std::fflush(stdout);
	}

	for (std::size_t index : indexes) {
		const Timings::Entry* entry = &Timings::Entries[index];
		if (!entry->tag || entry->tag >= 32 || !(tag_mask & (1u << entry->tag))) continue;
		if (!(entry->flags & (Timings::Sound | Timings::Animation))) continue;

		bool was_active = state.active_entries.count(index) != 0;
		bool disappear = entry->sound_cue_dissapear != 0;

		bool stored_ok = true;
		switch (entry->tag) {
		case Timings::BasicAttack:
			stored_ok = state.m1_armed;
			break;
		default:
			if ((entry->flags & Timings::StoredAction) && entry->stored_action && state.enemy.character)
				stored_ok = enemy_char.GetAttribute("StoredAction") == entry->stored_action;
			break;
		}

		bool sound_active = false;
		if ((entry->flags & Timings::Sound) && entry->sound_parry_cue)
			sound_active = active_name.find(std::string_view(entry->sound_parry_cue)) != active_name.end();

		bool anim_active = false;
		if ((entry->flags & Timings::Animation) && entry->anim_parry_cue)
			anim_active = active_anim.find(std::string_view(entry->anim_parry_cue)) != active_anim.end();

		const int cue_type = ((entry->flags & Timings::Sound) && (entry->flags & Timings::Animation)) ? 2
			: ((entry->flags & Timings::Sound) ? 1 : 0);
		bool cue_active;
		switch (cue_type) {
		case 2:
			cue_active = sound_active && anim_active;
			break;
		case 1:
			cue_active = sound_active;
			break;
		default:
			cue_active = anim_active;
			break;
		}

		if (cue_active) {
			if (!was_active) state.active_entries.insert(index);
		}

		unsigned char allowed_dirs = entry->directions;
		if (!(state.enemy.direction & allowed_dirs)) {
			if (!cue_active) {
				state.active_entries.erase(index);
				state.triggered_entries.erase(index);
			}
			continue;
		}

		float parry_range = entry->range;
		if (state.enemy.direction == Timings::Front && state.enemy.walkspeed > 4.4f) parry_range = entry->range + (state.enemy.walkspeed * entry->walkspeed);
		{
			float div = 0.f;
			switch (state.enemy.direction) {
			case Timings::Front: div = entry->div_front; break;
			case Timings::Back: div = entry->div_back; break;
			case Timings::Left: div = entry->div_left; break;
			case Timings::Right: div = entry->div_right; break;
			default: break;
			}
			if (div > 0.f) parry_range /= div;
		}
		bool in_range = state.enemy.distance <= (double)parry_range;

		float track_time = 0.0f;
		std::size_t track_index = (std::size_t)-1;
		if ((entry->flags & Timings::TimePosition) && (entry->flags & Timings::Animation) && entry->anim_parry_cue) {
			auto trace = active_anim.find(std::string_view(entry->anim_parry_cue));
			if (trace != active_anim.end()) {
				track_index = trace->second;
				if (track_index < state.anim_tracks.size()) track_time = mem::read<float>(state.anim_tracks[track_index] + Offsets::AnimationTrack::TimePosition);
			}
		}
		bool time_ok = !(entry->flags & Timings::TimePosition) || (track_index != (std::size_t)-1 && track_time >= entry->time_position && track_time <= entry->time_position + 0.016f);

		if (cue_active && !disappear) {
			bool parry_sent = state.triggered_entries.count(index) != 0;
			if (!parry_sent && in_range && time_ok && stored_ok) {
				Parry();
				state.triggered_entries.insert(index);
			}
			continue;
		}

		const bool sound_and_anim = (entry->flags & Timings::Sound) && (entry->flags & Timings::Animation);
		if (!cue_active && disappear && was_active && in_range && time_ok && stored_ok) {
			switch (unsigned{ sound_and_anim }) {
			case 0:
				Parry();
				break;
			default:
				if (!sound_active && anim_active) Parry();
				break;
			}
		}
		if (!cue_active) {
			state.active_entries.erase(index);
			state.triggered_entries.erase(index);
		}
	}
}
