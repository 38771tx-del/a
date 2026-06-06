#pragma once
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"
#include "../cache/sdk.hpp"

inline void run_nofog(bool no_fog) {
	if (no_fog) {
		for (const Instance& child : SDK::Atmosphere)
			mem::write<float>(child.address + Offsets::Atmosphere::Density, 0.f);
	} else {
		for (std::size_t index = 0; index < SDK::Atmosphere.size(); ++index)
		mem::write<float>(SDK::Atmosphere[index].address + Offsets::Atmosphere::Density, SDK::Density[index]);
		mem::write(SDK::FogEnd, 2000.f);
	}
}
