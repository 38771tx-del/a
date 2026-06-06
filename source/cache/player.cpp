#include "player.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <mutex>
#include <thread>
#include "../utils/timer_manager.hpp"
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"

namespace Player {
	static std::mutex cache_mutex;
	static std::vector<Entity> entity_list;
	static std::vector<PlayerObject> player_list;
	static ska::flat_hash_map<std::uint64_t, PartData> parts_map;
	static ska::flat_hash_map<std::uint64_t, PlayerEntry> entry_map;
	static std::vector<std::uint64_t> player_instances;
	static std::atomic<bool> esp{ false };
	static std::atomic<bool> autoparry{ false };

	static void cache() {
		while (true) {
			if (!esp.load(std::memory_order_relaxed) && !autoparry.load(std::memory_order_relaxed)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(64));
				continue;
			}

			if (task.wait<2880>() || player_instances.empty()) {
				player_instances.clear();
				for (Instance& child : SDK::Players.GetChildren())
					player_instances.push_back(child.address);

				entry_map.clear();
				for (std::uint64_t player_instance : player_instances)
					entry_map[player_instance];
			}

			Engine::Vector3 local_position = mem::read<Engine::Vector3>(SDK::HumanoidRootPartPrimitive + Offsets::Primitive::Position);

			float local_x = local_position.x;
			float local_y = local_position.y;
			float local_z = local_position.z;

			std::vector<Entity> entities_out;
			std::vector<PlayerObject> players_out;
			ska::flat_hash_map<std::uint64_t, PartData> parts_out;

			entities_out.reserve(32);
			players_out.reserve(32);
			parts_out.reserve(180);
			std::uint64_t health_start = (std::min)(Offsets::Humanoid::Health, Offsets::Humanoid::MaxHealth);
			std::uint64_t health_end = (std::max)(Offsets::Humanoid::Health + sizeof(float), Offsets::Humanoid::MaxHealth + sizeof(float));
			std::uint64_t primitive_start = (std::min)(Offsets::Primitive::Size, (std::min)(Offsets::Primitive::Rotation, Offsets::Primitive::Position));
			std::uint64_t primitive_end = (std::max)(Offsets::Primitive::Size + sizeof(Engine::Vector3), (std::max)(Offsets::Primitive::Rotation + sizeof(Engine::Matrix3x3), Offsets::Primitive::Position + sizeof(Engine::Vector3)));
			alignas(16) char health_buffer[64];
			alignas(16) char primitive_buffer[256];

			for (std::uint64_t player_instance : player_instances) {
				if (!player_instance || player_instance == SDK::LocalPlayer.address)
					continue;

				PlayerEntry& entry = entry_map[player_instance];

				if (entry.name.empty())
					entry.name = Instance{ player_instance }.GetInstanceName();

				std::uint64_t model_instance = mem::read<std::uint64_t>(player_instance + Offsets::Player::ModelInstance);

				if (model_instance != entry.model_instance) {
					entry.model_instance = model_instance;
					entry.humanoid = 0;
					entry.humanoidrootpart = 0;
					entry.parts.clear();

					if (model_instance) {
						entry.humanoid = Instance{ model_instance }.FindFirstChildOfClass("Humanoid").address;
						entry.humanoidrootpart = mem::read<std::uint64_t>(model_instance + Offsets::Model::PrimaryPart);
						for (Instance& child : Instance{ model_instance }.GetChildren()) {
							std::string name = child.GetInstanceName();
							for (const char* index : r6parts) {
								if (name == index) {
									entry.parts.push_back(child.address);
									break;
								}
							}
						}
					}
				}

				Entity entity{};
				entity.address = player_instance;
				entity.name = entry.name;
				entity.parts = entry.parts;
				entity.humanoidrootpart = entry.humanoidrootpart;
				entity.character = model_instance;

				if (entry.humanoid && (health_end - health_start) <= 64) {
					if (mem::read_bytes(entry.humanoid + health_start, health_buffer, (std::size_t)(health_end - health_start))) {
						entity.hp = *reinterpret_cast<float*>(health_buffer + (Offsets::Humanoid::Health - health_start));
						entity.max_hp = *reinterpret_cast<float*>(health_buffer + (Offsets::Humanoid::MaxHealth - health_start));
					}
				}

				if (entity.hp <= 0.f || entity.parts.empty())
					continue;

				for (std::uint64_t part_instance : entity.parts) {
					std::uint64_t primitive = mem::read<std::uint64_t>(part_instance + Offsets::BasePart::Primitive);

					PartData part_data{};
					part_data.primitive = primitive;

					if ((primitive_end - primitive_start) <= 256) {
						if (mem::read_bytes(primitive + primitive_start, primitive_buffer, (std::size_t)(primitive_end - primitive_start))) {
							part_data.size = *reinterpret_cast<Engine::Vector3*>(primitive_buffer + (Offsets::Primitive::Size - primitive_start));
							part_data.rot = *reinterpret_cast<Engine::Matrix3x3*>(primitive_buffer + (Offsets::Primitive::Rotation - primitive_start));
							part_data.position = *reinterpret_cast<Engine::Vector3*>(primitive_buffer + (Offsets::Primitive::Position - primitive_start));
							parts_out[part_instance] = part_data;
						}
					}
				}

				if (entry.humanoidrootpart) {
					std::uint64_t humanoidrootpart_primitive = mem::read<std::uint64_t>(entry.humanoidrootpart + Offsets::BasePart::Primitive);
					if (humanoidrootpart_primitive) {
						Engine::Vector3 root_position = mem::read<Engine::Vector3>(humanoidrootpart_primitive + Offsets::Primitive::Position);
						float root_x = root_position.x;
						float root_y = root_position.y;
						float root_z = root_position.z;

						float delta_x = root_x - local_x;
						float delta_y = root_y - local_y;
						float delta_z = root_z - local_z;

						entity.distance = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

						float walk_speed = 0.f;
						float distance_2d = sqrtf(delta_x * delta_x + delta_z * delta_z);

						if (distance_2d >= 0.001f) {
							float velocity_buf[3];
							if (mem::read_bytes(humanoidrootpart_primitive + Offsets::Primitive::AssemblyLinearVelocity, velocity_buf, sizeof(velocity_buf))) {
								walk_speed = velocity_buf[0] * (-delta_x / distance_2d) + velocity_buf[2] * (-delta_z / distance_2d);
								if (walk_speed < 0.f) walk_speed = 0.f;
							}
						}
						entity.walkspeed = walk_speed;

						Engine::Matrix3x3 rotation_matrix = mem::read<Engine::Matrix3x3>(humanoidrootpart_primitive + Offsets::Primitive::Rotation);

						float to_local_x = local_x - root_x;
						float to_local_z = local_z - root_z;
						float to_local_len = sqrtf(to_local_x * to_local_x + to_local_z * to_local_z);

						float forward_x = -rotation_matrix.data[2];
						float forward_z = -rotation_matrix.data[0];
						float forward_mag = sqrtf(forward_x * forward_x + forward_z * forward_z);

						if (to_local_len >= 0.001f && forward_mag >= 0.001f) {
							to_local_x /= to_local_len;
							to_local_z /= to_local_len;
							forward_x /= forward_mag;
							forward_z /= forward_mag;

							float dot_forward = forward_x * to_local_x + forward_z * to_local_z;
							float right_x = forward_z;
							float right_z = -forward_x;
							float dot_right = right_x * to_local_x + right_z * to_local_z;

							entity.direction = fabsf(dot_forward) > fabsf(dot_right) ? (dot_forward > 0.f ? 1u : 2u) : (dot_right > 0.f ? 4u : 8u);
						}
					}
				}

				std::uint64_t out_humanoidrootpart = entity.humanoidrootpart;
				std::uint64_t out_character = entity.character;
				float out_distance = entity.distance;
				float out_walkspeed = entity.walkspeed;
				unsigned char out_direction = entity.direction;

				entities_out.push_back(std::move(entity));
				players_out.push_back({ out_humanoidrootpart, out_character, out_distance, out_walkspeed, out_direction });
			}

			{
				std::lock_guard<std::mutex> lock(cache_mutex);
				entity_list = std::move(entities_out);
				player_list = std::move(players_out);
				parts_map = std::move(parts_out);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(8));
		}
	}

	void ESP(bool active) { esp.store(active); }
	void AutoParry(bool active) { autoparry.store(active); }

	std::vector<Entity> GetEntities() {
		std::lock_guard<std::mutex> lock(cache_mutex);
		return entity_list;
	}

	std::vector<PlayerObject> GetPlayers() {
		std::lock_guard<std::mutex> lock(cache_mutex);
		return player_list;
	}

	ska::flat_hash_map<std::uint64_t, PartData> GetParts() {
		std::lock_guard<std::mutex> lock(cache_mutex);
		return parts_map;
	}

	void Reset() {
		{
			std::lock_guard<std::mutex> lock(cache_mutex);
			entity_list.clear();
			player_list.clear();
			parts_map.clear();
		}
		entry_map.clear();
		player_instances.clear();
	}

	void Start() {
		static std::atomic<bool> started{ false };
		if (started.exchange(true))
			return;
		std::thread(cache).detach();
	}
}
