#pragma once
#include <cstdint>
#include <windows.h>
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"

inline ImGuiID kb_id = 0;
inline int* kb_ptr = nullptr;
inline bool kb_prev[256] = {};

struct KeyBind {
	int key = 0;

	inline bool IsPressed() const {
		return key > 0 && key < 256 && (GetAsyncKeyState(key) & 0x8000);
	}
};

inline const char* KeybindGetKeyName(int vk) {
	static char name[128] = {};
	if (vk == 0) return "None";
	switch (vk) {
	case VK_LBUTTON: return "M1";
	case VK_RBUTTON: return "M2";
	case VK_MBUTTON: return "M3";
	case VK_XBUTTON1: return "M4";
	case VK_XBUTTON2: return "M5";
	}
	UINT scanCode = MapVirtualKeyA((UINT)vk, MAPVK_VK_TO_VSC);
	if (scanCode == 0) return "Unknown";
	GetKeyNameTextA((LONG)(scanCode << 16), name, sizeof(name));
	return name;
}

inline void KeybindCaptureStart(ImGuiID id, int* key_ptr) {
	if (kb_id == id && kb_ptr == key_ptr) return;
	kb_id = id;
	kb_ptr = key_ptr;
	for (int vk = 0; vk < 256; vk++) kb_prev[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
}

inline void KeybindCaptureStop() {
	kb_id = 0;
	kb_ptr = nullptr;
	ImGui::ClearActiveID();
}

inline void KeybindCapture() {
	if (!kb_id || !kb_ptr) return;
	if (ImGui::GetActiveID() != kb_id) { kb_id = 0; kb_ptr = nullptr; return; }
	auto set_bind = [&](int vk) { *kb_ptr = vk; KeybindCaptureStop(); };
	ImGuiIO& io = ImGui::GetIO();
	if (io.MouseClicked[0] || io.MouseClicked[1] || ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) && !kb_prev[VK_ESCAPE])) {
		KeybindCaptureStop();
		return;
	}
	if (io.MouseClicked[2]) { set_bind(VK_MBUTTON); return; }
	if (io.MouseClicked[3]) { set_bind(VK_XBUTTON1); return; }
	if (io.MouseClicked[4]) { set_bind(VK_XBUTTON2); return; }
	for (int vk = 0x08; vk <= 0xA5; vk++) {
		if (vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON || vk == VK_XBUTTON1 || vk == VK_XBUTTON2 || vk == VK_ESCAPE) continue;
		bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
		if (down && !kb_prev[vk]) {
			set_bind(vk);
			return;
		}
	}
	for (int vk = 0; vk < 256; vk++) kb_prev[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
}
