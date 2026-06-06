#define IMGUI_DEFINE_MATH_OPERATORS
#include "source/imgui/imgui.h"
#include "source/imgui/imgui_internal.h"
#include "source/imgui/backends/imgui_impl_win32.h"
#include "source/imgui/backends/imgui_impl_dx11.h"
#include "source/imgui/colors_widgets.h"
#include "source/imgui/imgui_combo.hpp"
#include "source/imgui/fonts.h"
#include "source/imgui/images.h"
#include "source/cache/sdk.hpp"
#include "source/utils/offsets.hpp"
#include "source/utils/memory.hpp"
#include "source/functions/esp.hpp"
#include "source/functions/fly.hpp"
#include "source/functions/fog.hpp"
#include "source/functions/goto.hpp"
#include "source/functions/autoparry.hpp"
#include "source/functions/autowisp.hpp"
#include "source/utils/keybind.hpp"
#include "source/functions/timings.hpp"
#include "source/functions/bloxfinder.hpp"
#include "source/utils/timer_manager.hpp"
#include "config.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <tchar.h>
#include <map>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <atomic>
#include <thread>
#include <utility>
#include <charconv>
#include <comdef.h>
#include <dwmapi.h>
#include <shlwapi.h>
#include <windows.h>
#include "watermark.hpp"
#include "render.hpp"
#include "colors.hpp"
#include "taskmanager.hpp"

// ui taken off orthodox smh..

static void ClampToBounds(float& x, float& y, float w, float h, const RECT& b) { float minX = (float)b.left, minY = (float)b.top, maxX = (float)(b.right - (LONG)w), maxY = (float)(b.bottom - (LONG)h); if (maxX < minX) maxX = minX; if (maxY < minY) maxY = minY; if (x < minX) x = minX; if (y < minY) y = minY; if (x > maxX) x = maxX; if (y > maxY) y = maxY; }

static bool g_menu_drawn = false;
static std::atomic<bool> s_gameloaded{false};
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace misc {
    int tab_count = 0, active_tab_count = 0;
    float anim_tab = 0;
    int tab_width = 85;
    float alpha_child = 0;
}

namespace features {
    bool parry_basic_attacks = false;
    bool parry_critical_attacks = false;
    bool parry_mantras = false;
    bool parry_vents = false;
    bool roll_on_cd = false;
    bool feint_during_m1 = false;
    bool fly_enabled = false;
    bool hitbox_visual = false;
    bool health_visual = false;
    bool no_fog = false;
    bool no_clip = false;
    bool no_fall = false;
    bool desync = false;
    bool auto_wisp = false;
    int fly_speed = 1;
    int auto_wisp_speed = 1;
    int combobox_value = 0;
    int combobox_combo = 0;
    const char* backend_items[] = { "WinAPI", "(Soon!)" };
    const char* watermark_items[] = { "Top Left", "(Soon!)", "(Soon!)", "(Soon!)" };
    int key_toggle_menu = VK_INSERT, mind_toggle_menu = 1;
    char config_name_input[64] = { "" };
    bool watermark = true;
    bool stream_proof = false;
    bool task_manager_proof = false;
}

namespace pictures {
    ID3D11ShaderResourceView* aim_img = nullptr;
    ID3D11ShaderResourceView* misc_img = nullptr;
    ID3D11ShaderResourceView* visual_img = nullptr;
    ID3D11ShaderResourceView* keyboard_img = nullptr;
    ID3D11ShaderResourceView* wat_logo_img = nullptr;
    ID3D11ShaderResourceView* fps_img = nullptr;
    ID3D11ShaderResourceView* player_img = nullptr;
    ID3D11ShaderResourceView* time_img = nullptr;
}

namespace fonts {
    ImFont* inter_font = nullptr;
    ImFont* inter_bold_font = nullptr;
    ImFont* inter_bold_font2 = nullptr;
    ImFont* inter_bold_font3 = nullptr;
    ImFont* inter_bold_font4 = nullptr;
    ImFont* combo_icon_font = nullptr;
}

int main(int, char**)
{
    RECT screen_rect;
    GetWindowRect(GetDesktopWindow(), &screen_rect);
    int screen_w = screen_rect.right - screen_rect.left;
    int screen_h = screen_rect.bottom - screen_rect.top;
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST, wc.lpszClassName, L"ABYSS", WS_POPUP, 0, 0, screen_w, screen_h, nullptr, nullptr, wc.hInstance, nullptr);
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    ::ShowWindow(hwnd, SW_HIDE);
    ::UpdateWindow(hwnd);

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    StyleAbyss();
    ImGuiStyle base_style = ImGui::GetStyle();
    std::thread([]() {
        for (;;) {
            try {
                if (features::task_manager_proof) TaskManagerHide(mem::find_process_id("Taskmgr.exe"), false);
                else TaskManagerHide(0, false);
            } catch (...) {}
            Sleep(2880);
        }
    }).detach();
    std::thread([]() {
        for (;;) {
            try {
                if (!mem::get_process_handle(0)) {
                    if (!mem::attach_to_process("RobloxPlayerBeta.exe")) {
                        Sleep(2880);
                        continue;
                    }
                    mem::find_module_address("RobloxPlayerBeta.exe");
                    if (!mem::get_module_address()) {
                        SDK::Unload();
                        mem::detach(0);
                        Sleep(2880);
                        continue;
                    }
                    Offsets::Fetch();
                    SDK::Load();
                    continue;
                }
                HANDLE h = mem::get_process_handle(0);
                if (!h || h == INVALID_HANDLE_VALUE) {
                    SDK::Unload();
                    Offsets::Reset();
                    mem::detach(0);
                    Sleep(2880);
                    continue;
                }
                DWORD wait_r = WaitForSingleObject(h, 2880);
                if (wait_r == WAIT_OBJECT_0 || wait_r == WAIT_FAILED) {
                    SDK::Unload();
                    Offsets::Reset();
                    mem::detach(0);
                    Sleep(2880);
                } else if (wait_r == WAIT_TIMEOUT) {
                    std::uint64_t gl = mem::read<std::uint64_t>(SDK::GameLoaded);
                    if (gl != 31) {
                        SDK::Unload();
                        SDK::Load();
                        s_gameloaded.store(true);
                    }
                    else {
                        s_gameloaded.store(false);
                        std::uint64_t lp = mem::read<std::uint64_t>(SDK::Players.address + Offsets::Player::LocalPlayer);
                        std::uint64_t ch = mem::read<std::uint64_t>(lp + Offsets::Player::ModelInstance);
                        if (lp != SDK::LocalPlayer.address || ch != SDK::Character.address) SDK::ReloadCharacter();
                    }
                }
            } catch (...) {
                SDK::Unload();
                Offsets::Reset();
                mem::detach(0);
                Sleep(2880);
            }
        }
    }).detach();
    fonts::inter_font = io.Fonts->AddFontFromMemoryTTF(inter, sizeof(inter), 17, NULL, io.Fonts->GetGlyphRangesCyrillic());
    fonts::inter_bold_font = io.Fonts->AddFontFromMemoryTTF(inter_bold, sizeof(inter_bold), 20, NULL, io.Fonts->GetGlyphRangesCyrillic());
    fonts::inter_bold_font2 = io.Fonts->AddFontFromMemoryTTF(inter_bold, sizeof(inter_bold), 17, NULL, io.Fonts->GetGlyphRangesCyrillic());
    fonts::inter_bold_font3 = io.Fonts->AddFontFromMemoryTTF(inter_bold, sizeof(inter_bold), 18, NULL, io.Fonts->GetGlyphRangesCyrillic());
    fonts::inter_bold_font4 = io.Fonts->AddFontFromMemoryTTF(inter_bold, sizeof(inter_bold), 16, NULL, io.Fonts->GetGlyphRangesCyrillic());
    fonts::combo_icon_font = fonts::inter_bold_font2;
    if (load_config()) {
        features::parry_basic_attacks = saved.parry_basic_attacks;
        features::parry_critical_attacks = saved.parry_critical_attacks;
        features::parry_mantras = saved.parry_mantras;
        features::parry_vents = saved.parry_vents;
        features::roll_on_cd = saved.roll_on_cd;
        features::feint_during_m1 = saved.feint_during_m1;
        features::fly_enabled = saved.fly_enabled;
        features::hitbox_visual = saved.hitbox_visual;
        features::health_visual = saved.health_visual;
        features::no_fog = saved.no_fog;
        features::no_clip = saved.no_clip;
        features::no_fall = saved.no_fall;
        features::desync = saved.desync;
        features::auto_wisp = saved.auto_wisp;
        features::fly_speed = saved.fly_speed;
        features::auto_wisp_speed = saved.auto_wisp_speed;
        features::combobox_value = saved.combobox_value;
        features::combobox_combo = saved.combobox_combo;
        features::key_toggle_menu = saved.key_toggle_menu;
        features::watermark = saved.watermark;
        features::stream_proof = saved.stream_proof;
        features::task_manager_proof = saved.task_manager_proof;
        load_timings();
    }
    Timings::Build();

    if (!CreateOverlay(screen_w, screen_h, io.Fonts, wc.lpszClassName, wc.hInstance)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ImGuiContext* main_ctx = ImGui::GetCurrentContext();
    ImVec4 clear_color = ImVec4(17.f/255.f, 17.f/255.f, 17.f/255.f, 0.5f);
    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }
        RECT ui_rect = { 0, 0, screen_w, screen_h };
        auto window = SDK::Window();
        if (window.foreground) { int cw = window.client_rect.right - window.client_rect.left, ch = window.client_rect.bottom - window.client_rect.top; if (cw > 0 && ch > 0) ui_rect = { window.position.x, window.position.y, window.position.x + cw, window.position.y + ch }; }
        int ui_w = ui_rect.right - ui_rect.left, ui_h = ui_rect.bottom - ui_rect.top;
        if (ui_w <= 0 || ui_h <= 0) ui_rect = { 0, 0, screen_w, screen_h }, ui_w = screen_w, ui_h = screen_h;
        static RECT s_last_ui_rect = { -1, -1, -1, -1 };
        if (ui_rect.left != s_last_ui_rect.left || ui_rect.top != s_last_ui_rect.top || ui_w != (s_last_ui_rect.right - s_last_ui_rect.left) || ui_h != (s_last_ui_rect.bottom - s_last_ui_rect.top)) s_last_ui_rect = ui_rect, ::SetWindowPos(hwnd, HWND_TOPMOST, ui_rect.left, ui_rect.top, ui_w, ui_h, SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        float ui_scale = (float)ui_w / (float)screen_w, sy = (float)ui_h / (float)screen_h;
        if (sy < ui_scale) ui_scale = sy; if (ui_scale < 0.6f) ui_scale = 0.6f; if (ui_scale > 1.0f) ui_scale = 1.0f;
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        KeybindCapture();
        ImGui_ImplWin32_FeedNoActivateInputText();
        ImGuiStyle& scaled_style = ImGui::GetStyle();
        scaled_style = base_style;
        scaled_style.ScaleAllSizes(ui_scale);
        SetUIHelperScale(ui_scale);
        io.FontGlobalScale = ui_scale;
        auto S = [ui_scale](float v) { return v * ui_scale; };
        ImVec2 menu_size = ImVec2(settings::size_menu.x * ui_scale, settings::size_menu.y * ui_scale);
        ImVec2 watermark_size = ImVec2(settings::size_watermark.x * ui_scale, settings::size_watermark.y * ui_scale);

        if (!pictures::aim_img) CreateTextureFromPNGInMemory(g_pd3dDevice, aim, sizeof(aim), &pictures::aim_img);
        if (!pictures::misc_img) CreateTextureFromPNGInMemory(g_pd3dDevice, other, sizeof(other), &pictures::misc_img);
        if (!pictures::visual_img) CreateTextureFromPNGInMemory(g_pd3dDevice, visual, sizeof(visual), &pictures::visual_img);
        if (!pictures::keyboard_img) CreateTextureFromPNGInMemory(g_pd3dDevice, keyboard, sizeof(keyboard), &pictures::keyboard_img);
        if (!pictures::wat_logo_img) CreateTextureFromPNGInMemory(g_pd3dDevice, wat, sizeof(wat), &pictures::wat_logo_img);
        if (!pictures::fps_img) CreateTextureFromPNGInMemory(g_pd3dDevice, fps, sizeof(fps), &pictures::fps_img);
        if (!pictures::player_img) CreateTextureFromPNGInMemory(g_pd3dDevice, player, sizeof(player), &pictures::player_img);
        if (!pictures::time_img) CreateTextureFromPNGInMemory(g_pd3dDevice, timse, sizeof(timse), &pictures::time_img);

        if (features::watermark) DrawWatermarkUI(ui_scale, watermark_size, g_menu_drawn);

        static ImVec2 s_menu_pos(0.f, 0.f);
        static ImVec2 s_menu_norm(0.5f, 0.5f);
        static bool s_menu_first = true;
        static float s_prev_avail_x = -1.f, s_prev_avail_y = -1.f;
        float avail_x = ui_w - menu_size.x, avail_y = ui_h - menu_size.y;
        if (avail_x < 1.f) avail_x = 1.f;
        if (avail_y < 1.f) avail_y = 1.f;
        if (s_menu_first) {
            s_menu_pos = ImVec2(avail_x * 0.5f, avail_y * 0.5f);
            s_menu_norm = ImVec2(0.5f, 0.5f);
            s_prev_avail_x = avail_x;
            s_prev_avail_y = avail_y;
            s_menu_first = false;
        } else if (avail_x != s_prev_avail_x || avail_y != s_prev_avail_y) {
            s_menu_pos = ImVec2(s_menu_norm.x * avail_x, s_menu_norm.y * avail_y);
            s_prev_avail_x = avail_x;
            s_prev_avail_y = avail_y;
        }
        RECT sr = { 0, 0, ui_w, ui_h };
        ClampToBounds(s_menu_pos.x, s_menu_pos.y, menu_size.x, menu_size.y, sr);
        const float menu_gap = S(10.f);
        const float wm_l = S(10.f), wm_t = S(10.f), wm_r = S(10.f) + watermark_size.x, wm_b = S(10.f) + watermark_size.y;
        const float forbid_l = wm_l - menu_gap, forbid_t = wm_t - menu_gap, forbid_r = wm_r + menu_gap, forbid_b = wm_b + menu_gap;
        bool overlap_x = (s_menu_pos.x < forbid_r && s_menu_pos.x + menu_size.x > forbid_l);
        bool overlap_y = (s_menu_pos.y < forbid_b && s_menu_pos.y + menu_size.y > forbid_t);
        if (overlap_x && overlap_y) {
            float push_x = forbid_r - s_menu_pos.x;
            float push_y = forbid_b - s_menu_pos.y;
            if (push_x <= push_y)
                s_menu_pos.x = forbid_r;
            else
                s_menu_pos.y = forbid_b;
        }
        ClampToBounds(s_menu_pos.x, s_menu_pos.y, menu_size.x, menu_size.y, sr);
        s_menu_norm.x = s_menu_pos.x / avail_x;
        s_menu_norm.y = s_menu_pos.y / avail_y;
        if (s_menu_norm.x < 0.f) s_menu_norm.x = 0.f; else if (s_menu_norm.x > 1.f) s_menu_norm.x = 1.f;
        if (s_menu_norm.y < 0.f) s_menu_norm.y = 0.f; else if (s_menu_norm.y > 1.f) s_menu_norm.y = 1.f;
        ImGui::SetNextWindowPos(s_menu_pos);
        ImGui::SetNextWindowSize(menu_size);
        ImGui::Begin("ABYSS", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove);
        ImGuiStyle* style = &ImGui::GetStyle();
        style->Colors[ImGuiCol_WindowBg] = colors::menu::window_bg;
        style->Colors[ImGuiCol_Border] = colors::menu::border;
        style->ItemSpacing = ImVec2(0, S(5.f));
        style->WindowRounding = S(8.f);

        ImGui::SetCursorPos(ImVec2(S(10), S(10)));
        ImGui::BeginChild("General Tabs", ImVec2(S(620), S(60)), true, ImGuiWindowFlags_NoBackground);
        ImVec2 pos = ImGui::GetWindowPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + S(620), pos.y + S(60)), ImGui::GetColorU32(menu::general_child), S(10.f));
        if (pictures::wat_logo_img) draw_list->AddImage((ImTextureID)pictures::wat_logo_img, ImVec2(pos.x + S(24), pos.y + S(22)), ImVec2(pos.x + S(40), pos.y + S(38)), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
        if (fonts::inter_bold_font4) ImGui::PushFont(fonts::inter_bold_font4);
        draw_list->AddText(ImVec2(pos.x + S(56), pos.y + S(22)), ImColor(255, 255, 255), "ABYSS V1");
        if (fonts::inter_bold_font4) ImGui::PopFont();
        draw_list->AddRectFilledMultiColor(ImVec2(pos.x + S(144), pos.y + S(12)), ImVec2(pos.x + S(145.5f), pos.y + S(32)), ImGui::GetColorU32(ImVec4(1, 1, 1, 0)), ImGui::GetColorU32(ImVec4(1, 1, 1, 0)), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
        draw_list->AddRectFilledMultiColor(ImVec2(pos.x + S(144), pos.y + S(32)), ImVec2(pos.x + S(145.5f), pos.y + S(52)), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)), ImGui::GetColorU32(ImVec4(1, 1, 1, 0)), ImGui::GetColorU32(ImVec4(1, 1, 1, 0)));

        ImGui::SetCursorPos(ImVec2(S(155), S(12)));
        ImGui::PushID("tab_general");
        if (Tab("General", (ImTextureID)pictures::aim_img, ImVec2(S(95), S(40)), misc::tab_count == 0)) misc::tab_count = 0;
        ImVec2 r0_min = ImGui::GetItemRectMin(), r0_max = ImGui::GetItemRectMax();
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::PushID("tab_visuals");
        if (Tab("Visuals", (ImTextureID)pictures::visual_img, ImVec2(S(86), S(40)), misc::tab_count == 1)) misc::tab_count = 1;
        ImVec2 r1_min = ImGui::GetItemRectMin(), r1_max = ImGui::GetItemRectMax();
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::PushID("tab_config");
        if (Tab("Config", (ImTextureID)pictures::misc_img, ImVec2(S(95), S(40)), misc::tab_count == 2)) misc::tab_count = 2;
        ImVec2 r2_min = ImGui::GetItemRectMin(), r2_max = ImGui::GetItemRectMax();
        ImGui::PopID();

        ImVec2 inner_min = ImGui::GetCurrentWindow()->InnerRect.Min;
        ImVec2 r0_min_l(r0_min.x - inner_min.x, r0_min.y - inner_min.y);
        ImVec2 r0_max_l(r0_max.x - inner_min.x, r0_max.y - inner_min.y);
        ImVec2 r1_min_l(r1_min.x - inner_min.x, r1_min.y - inner_min.y);
        ImVec2 r1_max_l(r1_max.x - inner_min.x, r1_max.y - inner_min.y);
        ImVec2 r2_min_l(r2_min.x - inner_min.x, r2_min.y - inner_min.y);
        ImVec2 r2_max_l(r2_max.x - inner_min.x, r2_max.y - inner_min.y);

        bool drag_active = false;
        const float bar_h = S(60.f);
        const float bar_w = S(620.f);
        if (r0_min_l.x > 0) {
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::InvisibleButton("##drag_left", ImVec2(r0_min_l.x, bar_h));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) drag_active = true;
        }
        float gap01 = r1_min_l.x - r0_max_l.x;
        if (gap01 > 0) {
            ImGui::SetCursorPos(ImVec2(r0_max_l.x, 0));
            ImGui::InvisibleButton("##drag_gap01", ImVec2(gap01, bar_h));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) drag_active = true;
        }
        float gap12 = r2_min_l.x - r1_max_l.x;
        if (gap12 > 0) {
            ImGui::SetCursorPos(ImVec2(r1_max_l.x, 0));
            ImGui::InvisibleButton("##drag_gap12", ImVec2(gap12, bar_h));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) drag_active = true;
        }
        float right_w = bar_w - r2_max_l.x;
        if (right_w > 0) {
            ImGui::SetCursorPos(ImVec2(r2_max_l.x, 0));
            ImGui::InvisibleButton("##drag_right", ImVec2(right_w, bar_h));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) drag_active = true;
        }
        static ImVec2 s_drag_offset(0.f, 0.f);
        static bool s_drag_offset_valid = false;
        if (drag_active) {
            ImVec2 root_pos = ImGui::GetCurrentWindow()->RootWindow->Pos;
            ImVec2 mouse_pos = ImGui::GetIO().MousePos;
            if (!s_drag_offset_valid) {
                s_drag_offset = ImVec2(mouse_pos.x - root_pos.x, mouse_pos.y - root_pos.y);
                s_drag_offset_valid = true;
            }
            s_menu_pos = ImVec2(mouse_pos.x - s_drag_offset.x, mouse_pos.y - s_drag_offset.y);
        } else {
            s_drag_offset_valid = false;
            s_menu_pos = ImGui::GetCurrentWindow()->RootWindow->Pos;
        }
        ClampToBounds(s_menu_pos.x, s_menu_pos.y, menu_size.x, menu_size.y, sr);
        s_menu_norm.x = s_menu_pos.x / avail_x;
        s_menu_norm.y = s_menu_pos.y / avail_y;
        if (s_menu_norm.x < 0.f) s_menu_norm.x = 0.f; else if (s_menu_norm.x > 1.f) s_menu_norm.x = 1.f;
        if (s_menu_norm.y < 0.f) s_menu_norm.y = 0.f; else if (s_menu_norm.y > 1.f) s_menu_norm.y = 1.f;

        ImVec2 r_min[] = { r0_min, r1_min, r2_min }, r_max[] = { r0_max, r1_max, r2_max };
        int sel = misc::tab_count;
        const float ind_half = S(30.f);
        const float icon_w = S(19.f);
        if (sel >= 0 && sel <= 2) {
            float target_center = (r_min[sel].x + r_max[sel].x) * 0.5f - pos.x;
            if (sel == 0 || sel == 1) target_center += icon_w * 0.5f;
            misc::anim_tab = ImLerp(misc::anim_tab, target_center, io.DeltaTime * 15.f);
        }
        draw_list->AddRectFilled(ImVec2(pos.x + misc::anim_tab - ind_half, pos.y + S(57)), ImVec2(pos.x + misc::anim_tab + ind_half, pos.y + S(60)), ImGui::GetColorU32(colors::accent_color), S(10.f), ImDrawFlags_RoundCornersTop);

        ImGui::EndChild();

        misc::alpha_child = ImLerp(misc::alpha_child, (misc::tab_count == misc::active_tab_count) ? 1.f : 0.f, 15.f * io.DeltaTime);
        if (misc::alpha_child < 0.01f) misc::active_tab_count = misc::tab_count;

        ImGui::SetCursorPos(ImVec2(S(10), S(80)));
        ImGui::BeginChild("Main", ImVec2(S(725), S(440)), true, ImGuiWindowFlags_NoBackground);
        ImGui::SetCursorPos(ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, misc::alpha_child * style->Alpha);
        
        switch (misc::active_tab_count) {
        case 0: {
            ImGui::BeginGroup();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::aim_img, "Auto-Parry", ImVec2(S(304), S(270)), false, ImGuiWindowFlags_NoScrollWithMouse)) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, S(26.f)));
                ImGui::Checkbox("Parry Basic Attacks", &features::parry_basic_attacks);
                ImGui::Checkbox("Parry Critical Attacks", &features::parry_critical_attacks);
                ImGui::Checkbox("Parry Mantras", &features::parry_mantras);
                ImGui::Checkbox("Parry Vents", &features::parry_vents);
                ImGui::Checkbox("Roll on CD", &features::roll_on_cd);
                ImGui::Checkbox("Feint during M1", &features::feint_during_m1);
                ImGui::PopStyleVar();
            }
            ImGui::EndChildCustom();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::aim_img, "Misc", ImVec2(S(304), S(150)), false, ImGuiWindowFlags_NoScrollWithMouse)) {
                combo::Combo("Backend", &features::combobox_value, features::backend_items, IM_ARRAYSIZE(features::backend_items), 2);
                combo::Combo("Watermark", &features::combobox_combo, features::watermark_items, IM_ARRAYSIZE(features::watermark_items), 2);
            }
            ImGui::EndChildCustom();
            ImGui::EndGroup();
            ImGui::SameLine(0, S(10.f));
            ImGui::BeginGroup();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::aim_img, "Cheats", ImVec2(S(304), S(220)), false, 0)) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, S(6.f)));
                ImGui::Checkbox("Fly Hack:", &features::fly_enabled);
                ImGui::Dummy(ImVec2(0, S(6.f)));
                ImGui::SliderInt(" Fly Speed", &features::fly_speed, 1, 10);
                ImGui::Dummy(ImVec2(0, S(15.f)));
                ImGui::Checkbox("Auto Wisp:", &features::auto_wisp);
                ImGui::Dummy(ImVec2(0, S(6.f)));
                ImGui::SliderInt(" Click Speed", &features::auto_wisp_speed, 1, 10);
                ImGui::Dummy(ImVec2(0, S(15.f)));
                ImGui::Checkbox("No Clip", &features::no_clip);
                ImGui::Dummy(ImVec2(0, S(10.f)));
                ImGui::Checkbox("No Fall", &features::no_fall);
                ImGui::Dummy(ImVec2(0, S(10.f)));
                ImGui::Checkbox("Desync", &features::desync);
                ImGui::PopStyleVar();
            }
            ImGui::EndChildCustom();
            const ImVec2 exploits_padding(S(12.f), S(50.f));
            if (ImGui::BeginChildCustom((ImTextureID)pictures::aim_img, "Keybinds", ImVec2(S(304), S(200)), false, ImGuiWindowFlags_NoScrollWithMouse, &exploits_padding)) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, S(10)));
                Keybind((ImTextureID)pictures::keyboard_img, "Toggle Menu", &features::key_toggle_menu, true, S(16.f));
                ImGui::PopStyleVar();
            }
            ImGui::EndChildCustom();
            ImGui::EndGroup();
        } break;
        case 1:
            ImGui::BeginGroup();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::visual_img, "ESP", ImVec2(S(304), S(240)), false, ImGuiWindowFlags_NoScrollWithMouse)) {
                ImGui::Checkbox("Hitbox Visual", &features::hitbox_visual);
                ImGui::Checkbox("Health Visual", &features::health_visual);
            }
            ImGui::EndChildCustom();
            ImGui::EndGroup();
            ImGui::SameLine(0, S(10.f));
            ImGui::BeginGroup();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::visual_img, "Quality", ImVec2(S(304), S(200)), false, ImGuiWindowFlags_NoScrollWithMouse)) {
                ImGui::Checkbox("No Fog", &features::no_fog);
            }
            ImGui::EndChildCustom();
            ImGui::EndGroup();
            break;
        case 2:
            ImGui::BeginGroup();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::misc_img, "General", ImVec2(S(304), S(300)), false, ImGuiWindowFlags_NoScrollWithMouse)) {
                ImGui::Checkbox("Stream Proof", &features::stream_proof);
                ImGui::Checkbox("Task Manager Proof", &features::task_manager_proof);
            }
            ImGui::EndChildCustom();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::misc_img, "Configs", ImVec2(S(304), S(300)), false, ImGuiWindowFlags_NoScrollWithMouse)) {
                if (ImGui::Button("Load Config", ImVec2(S(126), S(30)))) {
                    if (load_config()) {
                        features::parry_basic_attacks = saved.parry_basic_attacks;
                        features::parry_critical_attacks = saved.parry_critical_attacks;
                        features::parry_mantras = saved.parry_mantras;
                        features::parry_vents = saved.parry_vents;
                        features::roll_on_cd = saved.roll_on_cd;
                        features::feint_during_m1 = saved.feint_during_m1;
                        features::fly_enabled = saved.fly_enabled;
                        features::hitbox_visual = saved.hitbox_visual;
                        features::health_visual = saved.health_visual;
                        features::no_fog = saved.no_fog;
                        features::no_clip = saved.no_clip;
                        features::no_fall = saved.no_fall;
                        features::desync = saved.desync;
                        features::auto_wisp = saved.auto_wisp;
                        features::fly_speed = saved.fly_speed;
                        features::auto_wisp_speed = saved.auto_wisp_speed;
                        features::combobox_value = saved.combobox_value;
                        features::combobox_combo = saved.combobox_combo;
                        features::key_toggle_menu = saved.key_toggle_menu;
                        features::watermark = saved.watermark;
                        features::stream_proof = saved.stream_proof;
                        features::task_manager_proof = saved.task_manager_proof;
                        load_timings();
                    }
                }
                ImGui::SameLine(0, S(10.f));
                if (ImGui::Button("Save Config", ImVec2(S(126), S(30)))) {
                    saved.parry_basic_attacks = features::parry_basic_attacks;
                    saved.parry_critical_attacks = features::parry_critical_attacks;
                    saved.parry_mantras = features::parry_mantras;
                    saved.parry_vents = features::parry_vents;
                    saved.roll_on_cd = features::roll_on_cd;
                    saved.feint_during_m1 = features::feint_during_m1;
                    saved.fly_enabled = features::fly_enabled;
                    saved.hitbox_visual = features::hitbox_visual;
                    saved.health_visual = features::health_visual;
                    saved.no_fog = features::no_fog;
                    saved.no_clip = features::no_clip;
                    saved.no_fall = features::no_fall;
                    saved.desync = features::desync;
                    saved.auto_wisp = features::auto_wisp;
                    saved.fly_speed = features::fly_speed;
                    saved.auto_wisp_speed = features::auto_wisp_speed;
                    saved.combobox_value = features::combobox_value;
                    saved.combobox_combo = features::combobox_combo;
                    saved.key_toggle_menu = features::key_toggle_menu;
                    saved.watermark = features::watermark;
                    saved.stream_proof = features::stream_proof;
                    saved.task_manager_proof = features::task_manager_proof;
                    save_config();
                }
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + S(4.f));
                if (ImGui::Button("Reset Config", ImVec2(S(262), S(30)))) {
                    int result = MessageBoxA(nullptr, "Program will exit and delete config file. Next launch will use defaults.", "Abyss", MB_OKCANCEL | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
                    if (result == IDOK) {
                        std::filesystem::remove(get_path());
                        ExitProcess(0);
                    }
                }
            }
            ImGui::EndChildCustom();
            ImGui::EndGroup();
            ImGui::SameLine(0, S(10.f));
            ImGui::BeginGroup();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::misc_img, "Timings", ImVec2(S(304), S(200)), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(S(6.f), S(14.f)));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(S(4.f), S(1.f)));
                for (std::size_t ti : Timings::time_index) {
                    Timings::Entry& te = Timings::Entries[ti];
                    ImGui::PushID((int)te.id);
                    const float row_y = ImGui::GetCursorPosY();
                    const float frame_h = ImGui::GetFrameHeight();
                    const float text_h = ImGui::GetTextLineHeight();
                    ImGui::SetCursorPosY(row_y + (frame_h - text_h) * 0.5f);
                    ImGui::TextUnformatted(te.stored_action);
                    ImGui::SameLine(S(152));
                    ImGui::SetCursorPosY(row_y);
                    ImGui::SetNextItemWidth(S(20));
                    ImGui::InputFloat("##timeposition", &te.time_position, 0.0f, 0.0f, "%.4f");
                    ImGui::PopID();
                }
                ImGui::PopStyleVar(2);
            }
            ImGui::EndChildCustom();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::misc_img, "GoTo", ImVec2(S(304), S(220)), false, ImGuiWindowFlags_NoScrollWithMouse)) {
                ImGui::BeginGroup();
                if (ImGui::Button("Warspot", ImVec2(S(126), S(30)))) goto_start(0);
                if (ImGui::Button("Crypt Of Unbroken", ImVec2(S(126), S(30)))) goto_start(2);
                if (ImGui::Button("Castle Light", ImVec2(S(126), S(30)))) goto_start(3);
                if (ImGui::Button("Hell Mode", ImVec2(S(126), S(30)))) goto_start(4);
                ImGui::EndGroup();
                ImGui::SameLine(0, S(10.f));
                ImGui::BeginGroup();
                if (ImGui::Button("Etris", ImVec2(S(126), S(30)))) goto_start(1);
                if (ImGui::Button("Depths Trial", ImVec2(S(126), S(30)))) goto_start(5);
                ImGui::EndGroup();
            }
            ImGui::EndChildCustom();
            if (ImGui::BeginChildCustom((ImTextureID)pictures::misc_img, "Bloxfinder", ImVec2(S(304), S(168)), false, ImGuiWindowFlags_NoScrollWithMouse)) {
                static char bloxfinder_user_id[64] = "";
                static std::string bloxfinder_status;
                const float x = ImGui::GetCursorPosX();
                const float label_x = x;
                const float input_x = x + S(80.f);
                const float input_w = S(127.f);
                const float button_w = S(194.f);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(S(6.f), S(3.f)));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(S(3.f), 0.f));
                const float frame_h = ImGui::GetFrameHeight();
                const float text_h = ImGui::GetTextLineHeight();
                float y = ImGui::GetCursorPosY() + S(2.f);
                const float label_dy = (frame_h - text_h) * 0.5f;
                ImGui::SetCursorPos(ImVec2(label_x, y + label_dy));
                ImGui::TextUnformatted("User ID:");
                ImGui::SetCursorPos(ImVec2(input_x, y));
                ImGui::SetNextItemWidth(input_w);
                ImGui::InputText("##bloxfinder_user_id", bloxfinder_user_id, sizeof(bloxfinder_user_id));
                y += frame_h + S(6.f);
                ImGui::SetCursorPos(ImVec2(input_x, y));
                if (ImGui::Button("Search", ImVec2(button_w, frame_h + S(4.f)))) {
                    bloxfinder_status = "searching...";
                    std::string lum, srv;
                    if (bloxfinder_user_id[0] == 0)
                        bloxfinder_status = "search failed";
                    else if (bloxfinder_run(bloxfinder_user_id, lum, srv) != 0)
                        bloxfinder_status = "search failed";
                    else
                        bloxfinder_status = lum + " - " + srv;
                }
                y += frame_h + S(10.f);
                ImGui::SetCursorPos(ImVec2(input_x, y));
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + button_w);
                ImGui::TextUnformatted(bloxfinder_status.c_str());
                ImGui::PopTextWrapPos();
                ImGui::PopStyleVar(2);
            }
            ImGui::EndChildCustom();
            ImGui::EndGroup();
            break;
        default:
            break;
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();

        static bool esp_started = false;
        if (!esp_started) { esp_start(); esp_started = true; }
        bool roblox_fg = window.foreground;
        static bool s_menu_vis = false, s_menu_toggle_down = false, s_prev_menu_vis = false, s_prev_menu_drawn = false;
        static bool s_menu_wait_release = false;
        static int s_prev_menu_key = VK_INSERT;
        static KeyBind menu_toggle_bind{};
        menu_toggle_bind.key = features::key_toggle_menu;
        if (features::key_toggle_menu != s_prev_menu_key) {
            s_prev_menu_key = features::key_toggle_menu;
            s_menu_wait_release = true;
        }
        bool menu_toggle_pressed = menu_toggle_bind.IsPressed();
        if (s_menu_wait_release) {
            if (!menu_toggle_pressed) {
                s_menu_wait_release = false;
                s_menu_toggle_down = false;
            } else {
                s_menu_toggle_down = true;
            }
        } else {
            if (menu_toggle_pressed && !s_menu_toggle_down && (s_menu_vis || roblox_fg)) s_menu_vis = !s_menu_vis;
            s_menu_toggle_down = menu_toggle_pressed;
        }
        bool menu_drawn = s_menu_vis && roblox_fg;
        ::ShowWindow(hwnd, menu_drawn ? SW_SHOWNOACTIVATE : SW_HIDE);
        s_prev_menu_drawn = menu_drawn; s_prev_menu_vis = s_menu_vis; g_menu_drawn = menu_drawn;
        bool draw_overlay = features::hitbox_visual && roblox_fg && !SDK::UIActive().menu;
        ::ShowWindow(g_overlay_hwnd, draw_overlay ? SW_SHOWNOACTIVATE : SW_HIDE);
        if (draw_overlay) {
            ImGui::SetCurrentContext(g_overlay_ctx);
            ImGui::GetIO().DisplaySize = ImVec2((float)screen_w, (float)screen_h);
            ImGui::GetIO().DeltaTime = io.DeltaTime;
            ImGui::NewFrame();
            esp_run(true, features::health_visual, window);
            ImGui::Render();
            ImDrawData* dd = ImGui::GetDrawData();
            ImGui::SetCurrentContext(main_ctx);
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_overlayRTV, nullptr);
            float clear_overlay[4] = { 0, 0, 0, 0 };
            g_pd3dDeviceContext->ClearRenderTargetView(g_overlayRTV, clear_overlay);
            if (dd && dd->CmdListsCount) ImGui_ImplDX11_RenderDrawData(dd);
            g_pOverlaySwapChain->Present(1, 0);
        } else
            esp_run(false, false, window);
        static bool s_last_no_fog = false;
        static bool s_last_no_clip = false;
        if (s_last_no_fog != features::no_fog)
            run_nofog(features::no_fog);
        if (features::no_fog && task.wait<3000>())
            mem::write(SDK::FogEnd, 9e9f);
        if (s_last_no_clip != features::no_clip)
            mem::write<std::uint8_t>(SDK::CanCollide, features::no_clip ? (SDK::CanCollideMask & ~0x08) : (SDK::CanCollideMask | 0x08));
        s_last_no_fog = features::no_fog;
        s_last_no_clip = features::no_clip;
        static bool s_desync = false;
        bool s_refresh = s_gameloaded.exchange(false);
        if (features::no_fall && SDK::GroundSensor.address && task.wait<16>())
            SDK::GroundSensor.GetAttribute("FloorMaterial");
        if (s_refresh || s_desync != features::desync) {
            mem::write<bool>(SDK::ModuleBase + FFlagOffsets::FFlags::NextGenReplicatorEnabledWrite4, features::desync);
            s_desync = features::desync;
        }
        if (features::fly_enabled && !moving)
            fly_run(25.f + 75.f * std::sqrt((float)(features::fly_speed - 1) / 9.f));
        {
            std::uint32_t parry_tags = 0;
            if (features::parry_basic_attacks) parry_tags |= (1u << Timings::BasicAttack);
            if (features::parry_critical_attacks) parry_tags |= (1u << Timings::CriticalAttack);
            if (features::parry_mantras) parry_tags |= (1u << Timings::Mantra);
            if (features::parry_vents) parry_tags |= (1u << Timings::Vent);
            autoparry_run(parry_tags != 0, parry_tags);
        }
        autowisp_run(features::auto_wisp, 80u + ((features::auto_wisp_speed - 1) * 70u) / 9u);
        static bool s_last_stream_proof = false;
        if (s_last_stream_proof != features::stream_proof) {
            s_last_stream_proof = features::stream_proof;
            DWORD affinity = features::stream_proof ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE;
            SetWindowDisplayAffinity(hwnd, affinity);
            SetWindowDisplayAffinity(g_overlay_hwnd, affinity);
        }
        if (g_menu_drawn) {
            ImGui::Render();
            const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            g_pSwapChain->Present(1, 0);
        } else {
            ImGui::EndFrame();
            g_pSwapChain->Present(1, 0);
        }
    }

    mem::write<std::uint8_t>(SDK::CanCollide, (SDK::CanCollideMask | 0x08));
    run_nofog(false);
    mem::detach(2);

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    if (g_overlay_ctx) {
        ImGui::SetCurrentContext(g_overlay_ctx);
        ImGui::DestroyContext();
        g_overlay_ctx = nullptr;
    }
    ImGui::SetCurrentContext(main_ctx);
    ImGui::DestroyContext();
    CleanupOverlay();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (hWnd == g_overlay_hwnd) {
        if (msg == WM_DESTROY) return 0;
        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        TaskManagerHide(0, true);
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
