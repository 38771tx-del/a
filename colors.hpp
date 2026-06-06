#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "source/imgui/imgui.h"
#include "source/imgui/imgui_internal.h"
#include "source/imgui/colors_widgets.h"
#include "source/utils/keybind.hpp"
#include <map>

namespace fonts {
	extern ImFont* inter_font;
	extern ImFont* inter_bold_font2;
	extern ImFont* inter_bold_font3;
}

struct KeyState { ImVec4 text, image; float slow; bool active; bool hovered; float alpha; };
inline std::map<ImGuiID, KeyState> g_keybind_anim{};
inline float g_ui_helper_scale = 1.0f;

inline void SetUIHelperScale(float scale) { g_ui_helper_scale = scale; }

inline bool Tab(const char* label, ImTextureID icon, const ImVec2& size_arg, bool active) {
	bool pressed = ImGui::InvisibleButton("##tab", size_arg);
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImVec2 rect_min = ImGui::GetItemRectMin();
	bool hovered = ImGui::IsItemHovered();
	float s = size_arg.y / 40.f;
	if (s < 0.6f) s = 0.6f;
	if (s > 1.0f) s = 1.0f;
	float alpha = active ? 1.f : (hovered ? 0.7f : 0.3f);
	ImU32 col = ImGui::GetColorU32(active ? colors::tabs::text_active : (hovered ? colors::tabs::text_hovered : colors::tabs::text_inactive));
	ImVec2 icon_min(rect_min + ImVec2(14.f * s, 9.f * s));
	ImVec2 icon_max(rect_min + ImVec2(33.f * s, 28.f * s));
	ImVec4 icon_col4 = active ? colors::tabs::text_active : (hovered ? colors::tabs::text_hovered : colors::tabs::text_inactive);
	icon_col4.w *= alpha;
	ImU32 icon_col = ImGui::GetColorU32(icon_col4);
	if (icon) window->DrawList->AddImage(icon, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), icon_col);
	else window->DrawList->AddRectFilled(icon_min, icon_max, icon_col, 4.f * s);
	if (fonts::inter_font) ImGui::PushFont(fonts::inter_font);
	window->DrawList->AddText(rect_min + ImVec2(37.f * s, 10.f * s), col, label);
	if (fonts::inter_font) ImGui::PopFont();
	return pressed;
}

inline bool BeginSection(ImTextureID icon, const char* name, const ImVec2& size_arg, const char* unique_id) {
	ImVec2 size = size_arg;
	const float header_h = 40.f;
	if (!ImGui::BeginChild(unique_id, size, false, ImGuiWindowFlags_NoScrollbar)) return false;
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImVec2 p = window->Pos;
	window->DrawList->AddRectFilled(p, ImVec2(p.x + size.x + 0.2f, p.y + size.y), ImGui::GetColorU32(colors::child::child_background), colors::child::child_rounding);
	window->DrawList->AddRectFilled(p, ImVec2(p.x + size.x + 0.2f, p.y + header_h), ImGui::GetColorU32(colors::child::child_top), colors::child::child_rounding, ImDrawFlags_RoundCornersTop);
	if (icon) window->DrawList->AddImage(icon, ImVec2(p.x + 11, p.y + 13), ImVec2(p.x + 27, p.y + 29), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(colors::accent_color));
	if (fonts::inter_bold_font3) ImGui::PushFont(fonts::inter_bold_font3);
	window->DrawList->AddText(ImVec2(p.x + 38, p.y + 12), ImGui::GetColorU32(colors::accent_color), name);
	if (fonts::inter_bold_font3) ImGui::PopFont();
	ImGui::SetCursorPos(ImVec2(20, header_h + 10));
	ImVec2 content_size(ImMax(1.f, size.x - 40), ImMax(1.f, size.y - header_h - 20));
	ImGui::Dummy(content_size);
	ImGui::SetCursorPos(ImVec2(20, header_h + 10));
	ImGui::PushID(unique_id);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 8));
	ImGui::Columns(2, nullptr, false);
	ImGui::SetColumnWidth(0, 140.f);
	return true;
}

inline void EndSection() {
	ImGui::Columns(1);
	ImGui::PopStyleVar();
	ImGui::PopID();
	ImGui::EndChild();
}

inline bool Keybind(ImTextureID icon, const char* label, int* key, bool label_active, float label_offset_x = 0.f) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return false;
	ImGuiContext& g = *ImGui::GetCurrentContext();
	ImGuiIO& io = ImGui::GetIO();
	const ImGuiID id = window->GetID(label);
	float s = g_ui_helper_scale;
	if (s < 0.6f) s = 0.6f;
	if (s > 1.0f) s = 1.0f;
	const float width = ImGui::GetContentRegionAvail().x;
	const float row_h = 35.f * s;
	const ImRect rect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(width, row_h));
	ImGui::ItemSize(rect);
	if (!ImGui::ItemAdd(rect, id)) return false;
	const char* buf_display = KeybindGetKeyName(*key);
	if (g.ActiveId == id) buf_display = "-";
	ImVec2 label_size = ImGui::CalcTextSize(buf_display);
	ImRect clickable(ImVec2(rect.Max.x - 19.f * s - label_size.x, rect.Min.y), rect.Max);
	bool hovered = ImGui::ItemHoverable(clickable, id, 0);
	KeyState& state = g_keybind_anim[id];
	state.text = ImLerp(state.text, g.ActiveId == id ? colors::binder::text_active : hovered ? colors::binder::text_hovered : colors::binder::text_inactive, io.DeltaTime * 6.f);
	state.image = ImLerp(state.image, g.ActiveId == id ? colors::accent_color : hovered ? colors::binder::image_hovered : colors::binder::image_inactive, io.DeltaTime * 6.f);
	state.slow = ImLerp(state.slow, clickable.Min.x - rect.Min.x - 40.f * s, io.DeltaTime * 15.f);
	window->DrawList->AddRectFilled(ImVec2(state.slow + rect.Min.x, clickable.Min.y), clickable.Max - ImVec2(0, 5.f * s), ImGui::GetColorU32(colors::binder::binder_bg), 4.f * s);
	window->DrawList->AddRect(ImVec2(state.slow + rect.Min.x, clickable.Min.y), clickable.Max - ImVec2(0, 5.f * s), ImGui::GetColorU32(colors::binder::binder_bg), 4.f * s);
	window->DrawList->AddRectFilled(clickable.Min - ImVec2(4.f * s, -5.f * s), clickable.Min - ImVec2(3.f * s, -25.f * s), ImGui::GetColorU32(colors::binder::line), 100.f * s);
	if (icon) window->DrawList->AddImage(icon, clickable.Min - ImVec2(30.f * s, -5.f * s), clickable.Min - ImVec2(14.f * s, -21.f * s), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(state.image));
	if (label_active && fonts::inter_bold_font2) { ImGui::PushFont(fonts::inter_bold_font2); window->DrawList->AddText(rect.Min + ImVec2(-8.f * s + label_offset_x, 8.f * s), ImGui::GetColorU32(state.text), label); ImGui::PopFont(); }
	ImGui::PushClipRect(ImVec2(state.slow + rect.Min.x, clickable.Min.y), clickable.Max, true);
	ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(state.text));
	ImGui::RenderTextClipped(clickable.Min + ImVec2(0, 2.f * s), clickable.Max - ImVec2(0, 7.f * s), buf_display, nullptr, &label_size, ImVec2(0.5f, 0.5f));
	ImGui::PopStyleColor();
	ImGui::PopClipRect();
	if (hovered && io.MouseClicked[0]) {
		ImGui::SetActiveID(id, window);
		ImGui::FocusWindow(window);
	} else if (io.MouseClicked[0] && g.ActiveId == id) ImGui::ClearActiveID();
	if (g.ActiveId == id) KeybindCaptureStart(id, key);
	return false;
}

inline void StyleAbyss() {
	ImGuiStyle& s = ImGui::GetStyle();
	s.WindowPadding = ImVec2(0, 0);
	s.FramePadding = ImVec2(8, 4);
	s.ItemSpacing = ImVec2(0, 5);
	s.WindowRounding = 8.f;
	s.WindowShadowSize = 0.f;
	s.FrameRounding = 4.f;
	s.GrabRounding = 4.f;
	s.ScrollbarRounding = 4.f;
	s.ChildRounding = 7.f;
	s.Colors[ImGuiCol_WindowBg] = colors::menu::window_bg;
	s.Colors[ImGuiCol_Border] = colors::menu::border;
	s.Colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
	s.Colors[ImGuiCol_FrameBg] = colors::combo::combo_bg;
	s.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.37f, 1.f);
	s.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.37f, 0.37f, 0.39f, 1.f);
	s.Colors[ImGuiCol_CheckMark] = colors::accent_color;
	s.Colors[ImGuiCol_SliderGrab] = colors::accent_color;
	s.Colors[ImGuiCol_SliderGrabActive] = colors::accent_color;
	s.Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);
	s.Colors[ImGuiCol_TextDisabled] = colors::tabs::text_inactive;
}
