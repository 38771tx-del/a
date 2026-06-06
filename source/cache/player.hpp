#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "sdk.hpp"
#include "../utils/flat_hash_map.hpp"

inline constexpr std::array<const char*, 6> r6parts = {
	"Head", "Torso", "Left Arm", "Right Arm", "Left Leg", "Right Leg"
};

struct Entity {
	std::uint64_t address{};
	std::string name;
	std::vector<std::uint64_t> parts;
	float hp{};
	float max_hp{};
	float distance{};
	std::uint64_t humanoidrootpart{};
	std::uint64_t character{};
	float walkspeed{};
	unsigned char direction{};
};

struct PlayerEntry {
	std::uint64_t model_instance{};
	std::uint64_t humanoid{};
	std::uint64_t humanoidrootpart{};
	std::string name{};
	std::vector<std::uint64_t> parts{};
};

struct PlayerObject {
	std::uint64_t humanoidrootpart{};
	std::uint64_t character{};
	float distance{};
	float walkspeed{};
	unsigned char direction{};
};

struct PartData {
	std::uint64_t primitive{};
	Engine::Vector3 size{};
	Engine::Matrix3x3 rot{};
	Engine::Vector3 position{};
};

namespace Player {
	void ESP(bool active);
	void AutoParry(bool active);
	std::vector<Entity> GetEntities();
	std::vector<PlayerObject> GetPlayers();
	ska::flat_hash_map<std::uint64_t, PartData> GetParts();
	void Reset();
	void Start();
}
