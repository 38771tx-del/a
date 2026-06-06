#include "autowisp.hpp"
#include <cstdio>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"
#include "../cache/sdk.hpp"

static std::atomic<bool> autowisp_enabled{ false };
static std::atomic<std::uint32_t> click_speed{ 80 };

struct Order { std::int32_t x; std::uint64_t frame; };

void autowisp_run(bool enabled, std::uint32_t speed) {
	click_speed.store(speed);
	static std::once_flag once;
	std::call_once(once, []() {
		std::thread([]() {
			static bool active = false;
			static std::vector<std::uint64_t> kids;
			static std::vector<Order> order;
			for (;;) {
				std::this_thread::sleep_for(std::chrono::milliseconds(16));
				if (!autowisp_enabled.load()) {
					active = false;
					continue;
				}
				if (!SDK::Window().foreground || SDK::UIActive().chat || SDK::UIActive().menu)
					continue;
				if (mem::read<Engine::UDim2>(SDK::SpellFrame.address + Offsets::GuiObject::Size).Y.Offset < 80) {
					active = false;
					std::this_thread::sleep_for(std::chrono::milliseconds(64));
					continue;
				}
				kids.clear();
				order.clear();
				for (Instance& child : SDK::Symbols.GetChildren())
					kids.push_back(child.address);
				if (kids.empty()) {
					active = false;
					std::this_thread::sleep_for(std::chrono::milliseconds(64));
					continue;
				}
				if (active) {
					std::this_thread::sleep_for(std::chrono::milliseconds(64));
					continue;
				}
				active = true;
				for (std::uint64_t frame : kids) {
					if (Instance{ frame }.GetInstanceName() != "Frame") continue;
					Engine::UDim2 Position = mem::read<Engine::UDim2>(frame + Offsets::GuiObject::Position);
					order.push_back({ Position.X.Offset, frame });
				}
				std::sort(order.begin(), order.end(), [](const Order& a, const Order& b) { return a.x < b.x; });
				std::string sequence;
				sequence.reserve(order.size());
				for (const auto& sym : order) {
					std::uint64_t textlabel = Instance{ sym.frame }.FindFirstChildOfClass("TextLabel").address;
					std::string character = mem::read_string(textlabel + Offsets::GuiObject::Text);
					sequence += character;
				}
				if (sequence.empty()) {
					active = false;
					continue;
				}
				for (std::size_t i = 0; i < sequence.size(); i++) {
					char key = sequence[i];
					WORD code = 0;
					switch (key) {
						case 'A': code = 0x1E; break;
						case 'B': code = 0x30; break;
						case 'C': code = 0x2E; break;
						case 'D': code = 0x20; break;
						case 'E': code = 0x12; break;
						case 'F': code = 0x21; break;
						case 'G': code = 0x22; break;
						case 'H': code = 0x23; break;
						case 'I': code = 0x17; break;
						case 'J': code = 0x24; break;
						case 'K': code = 0x25; break;
						case 'L': code = 0x26; break;
						case 'M': code = 0x32; break;
						case 'N': code = 0x31; break;
						case 'O': code = 0x18; break;
						case 'P': code = 0x19; break;
						case 'Q': code = 0x10; break;
						case 'R': code = 0x13; break;
						case 'S': code = 0x1F; break;
						case 'T': code = 0x14; break;
						case 'U': code = 0x16; break;
						case 'V': code = 0x2F; break;
						case 'W': code = 0x11; break;
						case 'X': code = 0x2D; break;
						case 'Y': code = 0x15; break;
						case 'Z': code = 0x2C; break;
						default: continue;
					}
					while (!SDK::Window().foreground || SDK::UIActive().chat || SDK::UIActive().menu)
						std::this_thread::sleep_for(std::chrono::milliseconds(16));
					if (mem::read<Engine::UDim2>(SDK::SpellFrame.address + Offsets::GuiObject::Size).Y.Offset < 80) {
						active = false;
						break;
					}
					INPUT input[2] = {};
					input[0].type = INPUT_KEYBOARD;
					input[0].ki.wScan = code;
					input[0].ki.dwFlags = KEYEVENTF_SCANCODE;
					input[1].type = INPUT_KEYBOARD;
					input[1].ki.wScan = code;
					input[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
					SendInput(2, input, sizeof(INPUT));
					if (i + 1 < sequence.size()) {
						std::uint64_t nextframe = order[i + 1].frame;
						for (;;) {
							if (mem::read<Engine::UDim2>(SDK::SpellFrame.address + Offsets::GuiObject::Size).Y.Offset < 80) {
								active = false;
								break;
							}
							if (mem::read<Engine::UDim2>(nextframe + Offsets::GuiObject::Position).X.Offset <= (std::int32_t)click_speed.load())
								break;
							std::this_thread::sleep_for(std::chrono::milliseconds(16));
						}
					}
				}
			}
		}).detach();
	});
	autowisp_enabled.store(enabled);
}
