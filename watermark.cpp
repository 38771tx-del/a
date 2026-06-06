#include "watermark.hpp"
#include "source/imgui/imgui.h"
#include "source/imgui/colors_widgets.h"
#include "source/cache/sdk.hpp"
#include "source/utils/memory.hpp"
#include "source/utils/timer_manager.hpp"
#include <charconv>
#include <string>
#include <d3d11.h>

namespace pictures {
	extern ID3D11ShaderResourceView* wat_logo_img;
	extern ID3D11ShaderResourceView* fps_img;
	extern ID3D11ShaderResourceView* player_img;
	extern ID3D11ShaderResourceView* time_img;
}

namespace fonts {
	extern ImFont* inter_bold_font4;
}

void DrawWatermarkUI(float ui_scale, const ImVec2& watermark_size, bool menu_drawn) {
	auto S = [ui_scale](float v) { return v * ui_scale; };
	ImGui::SetNextWindowPos(ImVec2(S(10), S(10)));
	ImGui::SetNextWindowSize(watermark_size);
	ImGui::Begin("watermark", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
	ImVec2 pos = ImGui::GetWindowPos();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(ImVec2(pos.x + S(10), pos.y + S(10)), ImVec2(pos.x + S(125), pos.y + S(40)), ImGui::GetColorU32(colors::menu::watermark_filled), S(4.f));
	if (pictures::wat_logo_img) dl->AddImage(pictures::wat_logo_img, ImVec2(pos.x + S(20), pos.y + S(16)), ImVec2(pos.x + S(36), pos.y + S(32)), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(colors::accent_color));
	if (fonts::inter_bold_font4) ImGui::PushFont(fonts::inter_bold_font4);
	dl->AddText(ImVec2(pos.x + S(45), pos.y + S(17)), ImGui::GetColorU32(ImVec4(0.31f, 0.31f, 0.31f, 1)), "ABYSS V1");
	if (fonts::inter_bold_font4) ImGui::PopFont();
	dl->AddRectFilled(ImVec2(pos.x + S(135), pos.y + S(10)), ImVec2(pos.x + S(245), pos.y + S(40)), ImGui::GetColorU32(colors::menu::watermark_filled), S(4.f));
	if (pictures::player_img) dl->AddImage(pictures::player_img, ImVec2(pos.x + S(145), pos.y + S(17)), ImVec2(pos.x + S(161), pos.y + S(33)), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(colors::accent_color));
	if (fonts::inter_bold_font4) ImGui::PushFont(fonts::inter_bold_font4);
	static std::string s_display_username = "UNKNOWN";
	dl->AddText(ImVec2(pos.x + S(166), pos.y + S(17)), ImGui::GetColorU32(ImVec4(0.31f, 0.31f, 0.31f, 1)), s_display_username.c_str());
	if (fonts::inter_bold_font4) ImGui::PopFont();
	dl->AddRectFilled(ImVec2(pos.x + S(255), pos.y + S(10)), ImVec2(pos.x + S(365), pos.y + S(40)), ImGui::GetColorU32(colors::menu::watermark_filled), S(4.f));
	if (pictures::fps_img) dl->AddImage(pictures::fps_img, ImVec2(pos.x + S(265), pos.y + S(17)), ImVec2(pos.x + S(281), pos.y + S(33)), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(colors::accent_color));
	if (fonts::inter_bold_font4) ImGui::PushFont(fonts::inter_bold_font4);
	static float s_display_fps = 0.f;
	static int s_display_ping = 0;
	static bool s_stats_due = true;
	if (task.wait<1440>()) s_stats_due = true;
	if (menu_drawn && s_stats_due) {
		double dt = mem::read<double>(SDK::HeartbeatFPS);
		double fps = 1.0 / dt;
		double max_fps = 1.0 / mem::read<double>(SDK::TaskSchedulerMaxFPS);
		if (fps > max_fps) fps = max_fps;
		s_display_fps = (float)fps;
		s_display_ping = (int)(mem::read<double>(SDK::Ping) + 0.5);
		s_display_username = SDK::LocalPlayer.GetInstanceName();
		if (s_display_username.empty()) s_display_username = "UNKNOWN";
		if ((int)s_display_username.size() > 10) s_display_username = s_display_username.substr(0, 9) + "-";
		s_stats_due = false;
	}
	char fps_text[24];
	int fps_i = (int)(s_display_fps + (s_display_fps >= 0.f ? 0.5f : -0.5f));
	auto fps_res = std::to_chars(fps_text, fps_text + sizeof(fps_text) - 1, fps_i);
	*fps_res.ptr = '\0';
	dl->AddText(ImVec2(pos.x + S(295), pos.y + S(17)), ImGui::GetColorU32(ImVec4(0.31f, 0.31f, 0.31f, 1)), fps_text);
	if (fonts::inter_bold_font4) ImGui::PopFont();
	dl->AddRectFilled(ImVec2(pos.x + S(375), pos.y + S(10)), ImVec2(pos.x + S(475), pos.y + S(40)), ImGui::GetColorU32(colors::menu::watermark_filled), S(4.f));
	if (pictures::time_img) dl->AddImage(pictures::time_img, ImVec2(pos.x + S(385), pos.y + S(17)), ImVec2(pos.x + S(401), pos.y + S(33)), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(colors::accent_color));
	if (fonts::inter_bold_font4) ImGui::PushFont(fonts::inter_bold_font4);
	char ping_text[24];
	auto ping_res = std::to_chars(ping_text, ping_text + sizeof(ping_text) - 1, s_display_ping);
	*ping_res.ptr = '\0';
	dl->AddText(ImVec2(pos.x + S(419), pos.y + S(17)), ImGui::GetColorU32(ImVec4(0.31f, 0.31f, 0.31f, 1)), ping_text);
	if (fonts::inter_bold_font4) ImGui::PopFont();
	ImGui::End();
}
