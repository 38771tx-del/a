#pragma once
#include <cstdint>
#include <windows.h>
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"
#include "../cache/sdk.hpp"

inline Engine::Vector3 velocity{};

__forceinline void fly_reset(void) {
	velocity = Engine::Vector3{};
}

inline void fly_run(float speed) {
	if (!SDK::Window().foreground || SDK::UIActive().chat || SDK::UIActive().menu) {
		mem::write(SDK::HumanoidRootPartPrimitive + Offsets::Primitive::AssemblyLinearVelocity, velocity);
		return;
	}
	Engine::Vector3 direction = mem::read<Engine::Vector3>(SDK::Humanoid.address + Offsets::Humanoid::MoveDirection);
	float up = (GetAsyncKeyState(VK_SPACE) & 0x8000) ? 1.f : 0.f;
	float down = (GetAsyncKeyState(VK_CONTROL) & 0x8000) ? 1.f : 0.f;
	up -= down;

	Engine::Vector3 move = direction + Engine::Vector3{ 0.f, up, 0.f };
	float magnitude = move.Magnitude();
	Engine::Vector3 output{};

	if (magnitude > 0.001f)
		output = move.Normalize() * speed;
	else
		output.y = up * speed;

	velocity = output;
	mem::write(SDK::HumanoidRootPartPrimitive + Offsets::Primitive::AssemblyLinearVelocity, output);
}
