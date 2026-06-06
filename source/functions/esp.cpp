#include "esp.hpp"
#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <cfloat>
#include <windows.h>
#include "../utils/flat_hash_map.hpp"
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"
#include "../cache/player.hpp"
#include "../utils/timer_manager.hpp"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"

static constexpr Engine::Vector3 corners[8] = {
	{-1.f, -1.f, -1.f}, {1.f, -1.f, -1.f}, {-1.f, 1.f, -1.f}, {1.f, 1.f, -1.f},
	{-1.f, -1.f,  1.f}, {1.f, -1.f,  1.f}, {-1.f, 1.f,  1.f}, {1.f, 1.f,  1.f}
};

static std::uint64_t cached_players = 0;
static std::uint64_t visual_engine = 0;
static float clip_origin_x = 0.f;
static float clip_origin_y = 0.f;
static int last_client_w = 0;
static int last_client_h = 0;
static int last_window_x = 0;
static int last_window_y = 0;

void esp_start(void) {
	Player::Start();
}

void esp_reset(void) {
	Player::Reset();

	cached_players = 0;
	visual_engine = 0;

	clip_origin_x = 0.f;
	clip_origin_y = 0.f;

	last_client_w = last_client_h = last_window_x = last_window_y = 0;
}

void esp_run(bool enabled, bool health_visual, const WindowState& window) {
	Player::ESP(enabled);
	if (!enabled) return;

	bool reload_visual = !visual_engine || (SDK::Players.address != cached_players);
	bool update_tick = reload_visual || task.wait<16>();
	if (update_tick) {
		if (reload_visual) visual_engine = mem::read<std::uint64_t>(SDK::ModuleBase + Offsets::VisualEngine::Pointer);
		cached_players = SDK::Players.address;
	}

	Engine::Matrix4x4 view_matrix = {};
	{
		float view_buffer[16];
		if (mem::read_bytes(visual_engine + Offsets::VisualEngine::ViewMatrix, view_buffer, sizeof(view_buffer))) std::memcpy(view_matrix.data, view_buffer, sizeof(view_buffer));
	}

	if (window.foreground) {
		int client_w = window.client_rect.right - window.client_rect.left;
		int client_h = window.client_rect.bottom - window.client_rect.top;
		if (client_w != last_client_w || client_h != last_client_h || window.position.x != last_window_x || window.position.y != last_window_y) {
			last_client_w = client_w;
			last_client_h = client_h;
			last_window_x = window.position.x;
			last_window_y = window.position.y;
			if (client_w > 0 && client_h > 0) {
				clip_origin_x = (float)window.position.x;
				clip_origin_y = (float)window.position.y;
			}
		}
	}

	const auto entity_list = Player::GetEntities();
	const auto part_lookup = Player::GetParts();
	ImDrawList* const draw_list = ImGui::GetBackgroundDrawList();
	draw_list->Flags |= ImDrawListFlags_AntiAliasedLines;
	if (last_client_w > 0 && last_client_h > 0) {
		draw_list->PushClipRect(ImVec2(clip_origin_x, clip_origin_y), ImVec2(clip_origin_x + (float)last_client_w, clip_origin_y + (float)last_client_h), true);
	}
	for (const Entity& entity : entity_list) {
		if (entity.address == 0 || entity.address == SDK::LocalPlayer.address) continue;
		if (entity.hp <= 0.f || entity.parts.empty()) continue;

		float min_x = FLT_MAX;
		float min_y = FLT_MAX;
		float max_x = -FLT_MAX;
		float max_y = -FLT_MAX;

		bool bounds_valid = false;
		for (std::uint64_t primitive : entity.parts) {
			auto part_it = part_lookup.find(primitive);
			if (part_it == part_lookup.end()) continue;
			const PartData& part_data = part_it->second;

			float half_x = part_data.size.x * 0.5f;
			float half_y = part_data.size.y * 0.5f;
			float half_z = part_data.size.z * 0.5f;

			for (const Engine::Vector3& corner : corners) {
				Engine::Vector3 world = part_data.position + part_data.rot * Engine::Vector3{ corner.x * half_x, corner.y * half_y, corner.z * half_z };
				Engine::Vector2 screen = Engine::WorldToScreen(world, Engine::Vector2{ (float)last_client_w, (float)last_client_h }, view_matrix);
				if (screen.x < 0.f || screen.y < 0.f) continue;

				float screen_x = clip_origin_x + screen.x;
				float screen_y = clip_origin_y + screen.y;

				bounds_valid = true;
				if (screen_x < min_x) min_x = screen_x;
				if (screen_y < min_y) min_y = screen_y;
				if (screen_x > max_x) max_x = screen_x;
				if (screen_y > max_y) max_y = screen_y;
			}
		}
		if (!bounds_valid) continue;

		float box_left = roundf(min_x);
		float box_top = roundf(min_y);
		float box_right = roundf(max_x);
		float box_bottom = roundf(max_y);

		ImU32 fill_color = IM_COL32(255, 255, 255, 255);
		ImU32 edge_color = IM_COL32(1, 1, 1, 255);

		draw_list->AddRect(ImVec2(box_left, box_top), ImVec2(box_right, box_bottom), edge_color);
		draw_list->AddRect(ImVec2(box_left - 1.f, box_top - 1.f), ImVec2(box_right + 1.f, box_bottom + 1.f), fill_color);
		draw_list->AddRect(ImVec2(box_left - 2.f, box_top - 2.f), ImVec2(box_right + 2.f, box_bottom + 2.f), edge_color);

		if (health_visual && entity.distance <= 5000.f) {
			float hp_ratio = entity.hp / entity.max_hp;
			float box_height = box_bottom - box_top;
			float bar_width = 3.f + (std::clamp(box_height / 25.f, 1.f, 3.f) - 3.f) * std::clamp((entity.distance - 750.f) / 250.f, 0.f, 1.f);
			float bar_right = box_left - 4.f;
			float bar_left = bar_right - bar_width;
			ImU32 health_color = (hp_ratio > 0.5f) ? IM_COL32((int)(255.f * (1.f - hp_ratio) * 2.f), 255, 0, 255) : IM_COL32(255, (int)(255.f * hp_ratio * 2.f), 0, 255);
			draw_list->AddRectFilled(ImVec2(bar_left, box_top), ImVec2(bar_right, box_bottom), edge_color);
			draw_list->AddRect(ImVec2(bar_left, box_top), ImVec2(bar_right, box_bottom), edge_color);
			draw_list->AddRect(ImVec2(bar_left - 1.f, box_top - 1.f), ImVec2(bar_right + 1.f, box_bottom + 1.f), fill_color);
			draw_list->AddRect(ImVec2(bar_left - 2.f, box_top - 2.f), ImVec2(bar_right + 2.f, box_bottom + 2.f), edge_color);
			draw_list->AddRectFilled(ImVec2(bar_left + (bar_width > 2.5f ? 1.f : 0.f), box_bottom - box_height * hp_ratio + (bar_width > 2.5f ? 1.f : 0.f)), ImVec2(bar_right - (bar_width > 2.5f ? 1.f : 0.f), box_bottom - (bar_width > 2.5f ? 1.f : 0.f)), health_color);
		}
		if (!entity.name.empty()) {
			ImVec2 text_size = ImGui::CalcTextSize(entity.name.c_str());
			ImVec2 text_pos((min_x + max_x) * 0.5f - text_size.x * 0.5f, min_y - text_size.y - 4.f);
			draw_list->AddText(ImVec2(text_pos.x - 1.f, text_pos.y - 1.f), IM_COL32(1, 1, 1, 255), entity.name.c_str());
			draw_list->AddText(ImVec2(text_pos.x + 1.f, text_pos.y - 1.f), IM_COL32(1, 1, 1, 255), entity.name.c_str());
			draw_list->AddText(ImVec2(text_pos.x - 1.f, text_pos.y + 1.f), IM_COL32(1, 1, 1, 255), entity.name.c_str());
			draw_list->AddText(ImVec2(text_pos.x + 1.f, text_pos.y + 1.f), IM_COL32(1, 1, 1, 255), entity.name.c_str());
			draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), entity.name.c_str());
		}
	}
	if (last_client_w > 0 && last_client_h > 0) {
		draw_list->PopClipRect();
	}
}
