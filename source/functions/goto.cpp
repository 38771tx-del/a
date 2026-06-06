#include "goto.hpp"
#include <cmath>
#include <thread>
#include <chrono>
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"

struct Target { const char* name; Engine::Vector3 pos; };

static const Target Location[] = {
	{ "Warspot", { -4430.f, 815.f, 4400.f } },
	{ "Etris", { 1500.f, 175.f, -4550.f } },
	{ "Crypt Of Unbroken", { 590.f, 985.f, -7560.f } },
	{ "Castle Light", { 4610.f, -1890.f, -500.f } },
	{ "Hell Mode", { 1850.f, -2220.f, -550.f } },
	{ "Depths Trial", { 2965.f, -2265.f, 1525.f } },
};

std::atomic<bool> moving{ false };
static Engine::Vector3 tp{};

static void goto_run(void) {
	while (moving) {
		HANDLE process = mem::get_process_handle(0);
		if (!process || process == INVALID_HANDLE_VALUE || WaitForSingleObject(process, 0) == WAIT_OBJECT_0 || mem::read<std::uint64_t>(SDK::GameLoaded) != 31) {
			moving = false;
			return;
		}
		Engine::Vector3 pos = mem::read<Engine::Vector3>(SDK::HumanoidRootPartPrimitive + Offsets::Primitive::Position);
		Engine::Vector3 diff = tp - pos;
		float distance = diff.Magnitude();
		Engine::Vector3 velocity{};
		Engine::Vector3 next = pos;
		if (distance < 3.f) {
			next = tp;
			moving = false;
		} else {
			Engine::Vector3 direction = diff.Normalize();
			next = pos + direction * 1.5f;
			velocity = direction * 100.f;
		}
		mem::write(SDK::HumanoidRootPartPrimitive + Offsets::Primitive::Position, next);
		mem::write(SDK::HumanoidRootPartPrimitive + Offsets::Primitive::AssemblyLinearVelocity, velocity);
		if (!moving) return;
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}
}

void goto_start(std::size_t index) {
	if (moving) return;
	tp = Location[index].pos;
	moving = true;
	std::thread(goto_run).detach();
}
