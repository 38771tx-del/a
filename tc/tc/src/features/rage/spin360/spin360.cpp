#include "spin360.h"

#include <game/game.h>
#include <cache/cache.h>
#include <sdk/sdk.h>
#include <memory/memory.h>
#include <sdk/offsets.h>
#include <settings.h>
#include <sdk/math/math.h>
#include <menu/keybind/keybind.h>

#include <windows.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <cmath>

static math::matrix3 create_rotation_from_yaw_pitch(float yaw, float pitch)
{
	float cos_yaw = std::cos(yaw);
	float sin_yaw = std::sin(yaw);
	float cos_pitch = std::cos(pitch);
	float sin_pitch = std::sin(pitch);

	math::vector3 forward(
		-sin_yaw * cos_pitch,
		sin_pitch,
		-cos_yaw * cos_pitch
	);

	math::vector3 world_up(0.0f, 1.0f, 0.0f);
	math::vector3 right = world_up.cross(forward);
	float length = right.length();
	if (length < 0.0001f)
	{
		world_up = math::vector3(0.0f, 0.0f, 1.0f);
		right = world_up.cross(forward);
		length = right.length();
		if (length < 0.0001f)
		{
			return math::matrix3::identity();
		}
	}
	right = right * (1.0f / length);

	math::vector3 up = forward.cross(right);

	math::matrix3 result{};
	result.m[0][0] = -right.x;
	result.m[0][1] = up.x;
	result.m[0][2] = -forward.x;
	result.m[1][0] = -right.y;
	result.m[1][1] = up.y;
	result.m[1][2] = -forward.y;
	result.m[2][0] = -right.z;
	result.m[2][1] = up.z;
	result.m[2][2] = -forward.z;

	return result;
}

void rage::spin360::run()
{
	static float start_angle = 0.0f;
	static float start_pitch = 0.0f;
	static float current_angle = 0.0f;
	static bool is_spinning = false;
	static bool was_key_pressed = false;
	static math::matrix3 original_rotation{};
	const float TWO_PI = 6.28318530718f;

	while (true) {
		if (!settings::rage::spin360::enabled || !game::datamodel.address) {
			if (is_spinning) {
				is_spinning = false;
				current_angle = 0.0f;
				start_angle = 0.0f;
				start_pitch = 0.0f;
			}
			was_key_pressed = false;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		if (game::camera == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		if (!is_spinning && settings::rage::spin360::keybind != 0) {
			bool is_key_pressed = (GetAsyncKeyState(settings::rage::spin360::keybind) & 0x8000) != 0;
			
			if (is_key_pressed && !was_key_pressed) {
				original_rotation = memory->read<math::matrix3>(game::camera + Offsets::Camera::Rotation);
				math::vector3 current_forward = original_rotation.forward();
				start_angle = std::atan2(-current_forward.x, -current_forward.z);
				start_pitch = std::asin(current_forward.y);
				current_angle = 0.0f;
				is_spinning = true;
			}
			
			was_key_pressed = is_key_pressed;
		}

		if (!is_spinning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		float speed_rad_per_ms = (settings::rage::spin360::speed * TWO_PI) / 1000.0f;
		auto current_time = std::chrono::steady_clock::now();
		static auto last_time = current_time;

		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();
		last_time = current_time;

		if (elapsed > 0) {
			current_angle += speed_rad_per_ms * elapsed;

			if (current_angle >= TWO_PI) {
				memory->write<math::matrix3>(game::camera + Offsets::Camera::Rotation, original_rotation);
				current_angle = 0.0f;
				is_spinning = false;
				start_angle = 0.0f;
				start_pitch = 0.0f;
				was_key_pressed = false;
				continue;
			}

			float target_yaw = start_angle + current_angle;
			math::matrix3 new_rotation = create_rotation_from_yaw_pitch(target_yaw, start_pitch);
			memory->write<math::matrix3>(game::camera + Offsets::Camera::Rotation, new_rotation);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
