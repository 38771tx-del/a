#include "fog.h"

#include <thread>
#include <chrono>

#include <memory/memory.h>
#include <sdk/offsets.h>
#include <sdk/sdk.h>
#include <game/game.h>
#include <settings.h>

namespace lighting
{
	namespace fog
	{
		struct OriginalFog {
			float fog_start;
			float fog_end;
			math::vector3 fog_color;
			bool saved = false;
		};

		static OriginalFog original;
		static bool was_enabled = false;

		static void fog_thread() {
			using namespace std::chrono_literals;

			while (true) {
				std::this_thread::sleep_for(std::chrono::milliseconds(250));

				try {
					if (game::datamodel.address == 0) {
						continue;
					}

					rbx::instance_t lighting = game::datamodel.find_first_child("Lighting");
					if (lighting.address == 0) {
						lighting = game::datamodel.find_first_child_by_class("Lighting");
					}
					if (lighting.address == 0) {
						continue;
					}

					if (!settings::lighting::fog::enabled) {
						if (was_enabled && original.saved) {
							memory->write<float>(lighting.address + Offsets::Lighting::FogStart, original.fog_start);
							memory->write<float>(lighting.address + Offsets::Lighting::FogEnd, original.fog_end);
							memory->write<math::vector3>(lighting.address + Offsets::Lighting::FogColor, original.fog_color);
							was_enabled = false;
						}
						continue;
					}

					if (!original.saved) {
						original.fog_start = memory->read<float>(lighting.address + Offsets::Lighting::FogStart);
						original.fog_end = memory->read<float>(lighting.address + Offsets::Lighting::FogEnd);
						original.fog_color = memory->read<math::vector3>(lighting.address + Offsets::Lighting::FogColor);
						original.saved = true;
					}

					memory->write<float>(lighting.address + Offsets::Lighting::FogStart, settings::lighting::fog::fog_start);
					memory->write<float>(lighting.address + Offsets::Lighting::FogEnd, settings::lighting::fog::fog_end);
					
					math::vector3 fog_color(settings::lighting::fog::fog_r, 
						settings::lighting::fog::fog_g, 
						settings::lighting::fog::fog_b);
					memory->write<math::vector3>(lighting.address + Offsets::Lighting::FogColor, fog_color);
					was_enabled = true;
				}
				catch (...) {
				}
			}
		}

		void run() {
			static bool initialized = false;
			if (!initialized) {
				std::thread(fog_thread).detach();
				initialized = true;
			}
		}
	}
}
