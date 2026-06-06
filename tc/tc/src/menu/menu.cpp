//============ Copyright ImMagic, All rights reserved ============//
//
// Purpose: 
//
//================================================================//

#include "menu.h"

// Dear ImGui
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

// Dear ImGui - Backends
#include <imgui/backends/imgui_impl_dx11.h>
#include <imgui/backends/imgui_impl_win32.h>

// Dear ImGui - Misc
#include <imgui/misc/imgui_freetype.h>

// Texture
#include "../../render/textures/texture.h"

// ImGui Addons
#include "../../../ext/imgui/addons/imgui_addons.h"

// Project includes
#include <settings.h>
#include <menu/keybind/keybind.h>
#include <features/explorer/explorer.h>
#include <features/config/config.h>
#include <cache/custom_entities/custom_entities.h>
#include <memory/memory.h>
#include <game/game.h>
#include <sdk/offsets.h>
#include <features/aimbot/aimbot.h>
#include <features/avatarmanager/avatarmanager.h>
#include <features/lighting/skybox/skybox.h>
#include <render/render.h>
#include "../../render/3d/main_api.hpp"
#include "../../render/3d/texture/texture_cache.hpp"
#include "../../ext/font/config/font_config.h"
#include "../../ext/font/visitor.h"
#include "../../ext/font/sp7.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <unordered_map>
#include <unordered_set>

extern ImFont* esp_font;
extern ImFont* esp_font_visitor;
extern ImFont* esp_font_sp7;
extern ImFont* esp_font_arial;

class AvatarManager;
extern std::unique_ptr<AvatarManager> g_avatar_manager;

namespace helper
{
    void draw_text_outlined(ImDrawList* draw, ImFont* font, float font_size, ImVec2 pos, ImU32 col, const char* text_begin, const char* text_end = nullptr);
    void corner_box(ImDrawList* draw, ImVec2 min, ImVec2 max, ImU32 col, float thickness = 1.f);
}

#define PROJECT_NAME    "Turtle.Club"

bool Menu::Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    bool result = true;

    IMGUI_CHECKVERSION();
    if (!ImGui::GetCurrentContext())
    {
        ImGui::CreateContext();
    }

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.IniFilename = nullptr;   // Disable INI File

    // Setup Dear ImGui default style
    ImGui::StyleColorsDark();
    
    style.WindowRounding    = 0;
    style.ChildRounding     = 0;
    style.FrameRounding     = 0;
    style.PopupRounding     = 0;
    style.GrabRounding      = 0;
    style.ScrollbarRounding = 0;

    style.WindowBorderSize  = 1;
    style.FrameBorderSize   = 1;
    style.PopupBorderSize   = 1;

    style.WindowPadding     = ImVec2(9, 9);
    style.ChildPadding      = ImVec2(7, 7);
    style.FramePadding      = ImVec2(5.0f, 4.5f);
    style.CellPadding       = ImVec2(2, 2); // checkbox padding
    style.ItemSpacing       = ImVec2(7, 7);
    style.ItemInnerSpacing  = ImVec2(5, 6);
    style.WindowMinSize     = ImVec2(0, 0);

    style.ScrollbarSize     = 6.0f;

    style.Colors[ImGuiCol_WindowBg]             = ImAdd::HexToColorVec4(0x1c1c1c);
    style.Colors[ImGuiCol_ChildBg]              = ImAdd::HexToColorVec4(0x1c1c1c);
    style.Colors[ImGuiCol_PopupBg]              = ImAdd::HexToColorVec4(0x181818);

    style.Colors[ImGuiCol_Text]                 = ImAdd::HexToColorVec4(0xffffff);
    style.Colors[ImGuiCol_TextDisabled]         = ImAdd::HexToColorVec4(0x848484); // idk if this is accurate, coz in the ss there was no disabled text

    style.Colors[ImGuiCol_Border]               = ImAdd::HexToColorVec4(0x000000);
    style.Colors[ImGuiCol_Separator]            = style.Colors[ImGuiCol_Border];

    style.Colors[ImGuiCol_Header]               = ImAdd::HexToColorVec4(0xE8FFE8);
    style.Colors[ImGuiCol_HeaderHovered]        = ImAdd::HexToColorVec4(0xD8FFD8);
    style.Colors[ImGuiCol_HeaderActive]         = ImAdd::HexToColorVec4(0xC8FFC8);

    style.Colors[ImGuiCol_SliderGrab]           = style.Colors[ImGuiCol_Header];
    style.Colors[ImGuiCol_SliderGrabActive]     = style.Colors[ImGuiCol_HeaderActive];

    style.Colors[ImGuiCol_Button]               = ImAdd::HexToColorVec4(0x232323);
    style.Colors[ImGuiCol_ButtonHovered]        = ImAdd::HexToColorVec4(0x252525);
    style.Colors[ImGuiCol_ButtonActive]         = ImAdd::HexToColorVec4(0x212121);

    style.Colors[ImGuiCol_FrameBg]              = ImAdd::HexToColorVec4(0x232323);
    style.Colors[ImGuiCol_FrameBgHovered]       = ImAdd::HexToColorVec4(0x252525);
    style.Colors[ImGuiCol_FrameBgActive]        = ImAdd::HexToColorVec4(0x212121);

    style.Colors[ImGuiCol_Tab]                  = ImAdd::HexToColorVec4(0x1b1b1b);
    style.Colors[ImGuiCol_TabHovered]           = ImAdd::HexToColorVec4(0x1c1c1c);
    style.Colors[ImGuiCol_TabActive]            = ImAdd::HexToColorVec4(0x1a1a1a);

    style.Colors[ImGuiCol_FrameBgShadow]        = ImAdd::HexToColorVec4(0x000000, 0.5f);
    style.Colors[ImGuiCol_ButtonShadow]         = ImAdd::HexToColorVec4(0xffffff, 0.035f);

    // Setup Font
    ImFontConfig font_cfg_main;
    font_cfg_main.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_ForceAutoHint;
    font_cfg_main.GlyphOffset = ImVec2(0, 1);
    font_cfg_main.SizePixels = 12.0f;
	font_cfg_main.GlyphExtraAdvanceX = 1.0f;
	io.Fonts->AddFontDefault(&font_cfg_main);

    // Initialize Dear ImGui - WIN32
    result = ImGui_ImplWin32_Init(hWnd);
    if (!result) return false;

    // Initialize Dear ImGui - DX11
    result = ImGui_ImplDX11_Init(pDevice, pDeviceContext);
    if (!result) return false;

    m_bInitialized = true;

    return true;
}

void Menu::Render()
{
    if (!m_bInitialized) return;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    {
        DrawMenu();
    }
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Menu::DrawWatermark()
{
    if (!settings::menu::watermark)
    {
        return;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    
    char watermark_text[64];
    int fps = static_cast<int>(ImGui::GetIO().Framerate);
    std::snprintf(watermark_text, sizeof(watermark_text), "##Turtle.Club | Exclusives Only | %d", fps);
    
    float text_width = ImGui::CalcTextSize(watermark_text).x;
    float watermark_width = text_width + style.FramePadding.x * 2.0f + style.CellPadding.x * 2.0f;

    ImGui::SetNextWindowSize(ImVec2(watermark_width, ImGui::GetFrameHeight() + style.CellPadding.y * 2.0f + style.WindowBorderSize * 5.0f), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(20, 100), ImGuiCond_Once);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    bool watermark_window = ImGui::Begin("ImMagic - Watermark", (bool*)0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar(2);

    if (watermark_window)
    {
        ImRect window_bb(ImGui::GetCurrentWindow()->Rect());

        if (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_NoBackground)
        {
            ImGuiWindow* window = ImGui::GetCurrentWindow();

            window->DrawList->AddRectFilled(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_WindowBg));

            if (style.WindowBorderSize > 0.0f)
            {
                window->DrawList->AddRect(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, ImDrawFlags_None, style.WindowBorderSize);

                window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize, 0.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Min.y), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
                window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize, style.WindowBorderSize * 1.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Min.y + style.WindowBorderSize * 1.0f), ImGui::GetColorU32(ImGuiCol_Header), style.WindowBorderSize);
                window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize, style.WindowBorderSize * 2.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Min.y + style.WindowBorderSize * 2.0f), ImGui::GetColorU32(ImGuiCol_HeaderActive), style.WindowBorderSize);
                window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize, style.WindowBorderSize * 3.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Min.y + style.WindowBorderSize * 3.0f), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
                window->DrawList->AddLine(ImVec2(window_bb.Min.x + style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
            }
        }

        ImGui::SetCursorScreenPos(window_bb.Min + ImVec2(0.0f, style.ChildBorderSize * 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.CellPadding);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, style.CellPadding);
        if (ImGui::BeginChild("watermark_body", ImVec2(window_bb.GetWidth(), ImGui::GetFrameHeight() + style.CellPadding.y * 2.0f), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.FramePadding.y));
            if (ImGui::BeginChild("logo", ImVec2(ImGui::GetWindowWidth(), ImGui::GetFrameHeight()), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground));
            {
                ImAdd::RenderText(ImGui::GetCursorScreenPos(), watermark_text, NULL, false, true);
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
}

void Menu::DrawMenu()
{
    static bool m_bMainWindowOpen = true;
    static bool m_bExplorerWindowOpen = false;
    static bool m_bClientWindowOpen = false;
    
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, ImGui::GetFrameHeight() + style.CellPadding.y * 2.0f + style.WindowBorderSize * 5.0f), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    bool menubar_window = ImGui::Begin("ImMagic - MenuBar", (bool*)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize);
    ImGui::PopStyleVar(2);

    if (menubar_window)
    {
        ImRect window_bb(ImGui::GetCurrentWindow()->Rect());

        if (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_NoBackground)
        {
            ImGuiWindow* window = ImGui::GetCurrentWindow();

            window->DrawList->AddRectFilled(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_WindowBg));

            if (style.WindowBorderSize > 0.0f)
            {
                // Window outer border
                window->DrawList->AddRect(window_bb.Min - ImVec2(style.WindowBorderSize, 0.0f), window_bb.Max + ImVec2(style.WindowBorderSize, 0.0f), ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, ImDrawFlags_None, style.WindowBorderSize);

                window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize, 0.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Min.y), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
                window->DrawList->AddLine(ImVec2(window_bb.Min.x + style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize * 4.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize * 4.0f), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
                window->DrawList->AddLine(ImVec2(window_bb.Min.x + style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize * 3.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize * 3.0f), ImGui::GetColorU32(ImGuiCol_Header), style.WindowBorderSize);
                window->DrawList->AddLine(ImVec2(window_bb.Min.x + style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize * 2.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize * 2.0f), ImGui::GetColorU32(ImGuiCol_HeaderActive), style.WindowBorderSize);
                window->DrawList->AddLine(ImVec2(window_bb.Min.x + style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Max.y - style.WindowBorderSize), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
            }
        }

        ImGui::SetCursorScreenPos(window_bb.Min + ImVec2(0.0f, style.ChildBorderSize));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.CellPadding);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, style.CellPadding);
        if (ImGui::BeginChild("body", ImVec2(window_bb.GetWidth(), ImGui::GetFrameHeight() + style.CellPadding.y * 2.0f), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground))
        {
            // YEAP, you have to make a child like this to align the text correctly each time u wanna use text in this menubar...
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, style.FramePadding.y));
            if (ImGui::BeginChild("logo", ImVec2(ImGui::CalcTextSize(PROJECT_NAME).x, ImGui::GetFrameHeight()), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground));
            {
                ImAdd::RenderText(ImGui::GetCursorScreenPos(), PROJECT_NAME, NULL, false, true);
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();

            ImGui::SameLine();
            ImAdd::VSeparator(style.CellPadding.y);
            ImGui::SameLine();

            if (ImAdd::ButtonAccent("Main"))
                m_bMainWindowOpen = !m_bMainWindowOpen;
            ImGui::SameLine();
            if (ImAdd::ButtonAccent("Explorer"))
                m_bExplorerWindowOpen = !m_bExplorerWindowOpen;
            ImGui::SameLine();
            if (ImAdd::ButtonAccent("Client"))
                m_bClientWindowOpen = !m_bClientWindowOpen;
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
    }
    ImGui::End();

    if (m_bMainWindowOpen)
    {
        ImGui::SetNextWindowSize(ImVec2(487, 588), ImGuiCond_Once);
        ImGui::SetNextWindowPos(io.DisplaySize / 2, ImGuiCond_Once, ImVec2(0.5f, 0.5f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        bool main_window = ImGui::Begin("ImMagic - Menu", &m_bMainWindowOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize);
        ImGui::PopStyleVar(2);

        if (main_window)
    {
        ImRect window_bb(ImGui::GetCurrentWindow()->Rect());

        if (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_NoBackground)
        {
            ImGuiWindow* window = ImGui::GetCurrentWindow();

            window->DrawList->AddRectFilled(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_WindowBg));

            if (style.WindowBorderSize > 0.0f)
            {
                // Window outer border
				window->DrawList->AddRect(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, ImDrawFlags_None, style.WindowBorderSize);
                window->DrawList->AddRect(window_bb.Min + ImVec2(style.WindowBorderSize, style.WindowBorderSize), window_bb.Max - ImVec2(style.WindowBorderSize, style.WindowBorderSize), ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, ImDrawFlags_None, style.WindowBorderSize);
            
                // Window top decoration
                window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize * 2.0f, style.WindowBorderSize * 2.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize * 2.0f, window_bb.Min.y + style.WindowBorderSize * 2.0f), ImGui::GetColorU32(ImGuiCol_Header), style.WindowBorderSize);
                window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize * 2.0f, style.WindowBorderSize * 3.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize * 2.0f, window_bb.Min.y + style.WindowBorderSize * 3.0f), ImGui::GetColorU32(ImGuiCol_HeaderActive), style.WindowBorderSize);
                window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize * 2.0f, style.WindowBorderSize * 4.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize * 2.0f, window_bb.Min.y + style.WindowBorderSize * 4.0f), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
            }

            ImAdd::RenderText(window_bb.Min + ImVec2(style.FramePadding.x + style.WindowBorderSize * 3.0f, style.FramePadding.y + style.WindowBorderSize * 4.0f), PROJECT_NAME, NULL, false, true);
        }

		static int tab_index = 0;

        ImGui::SetCursorScreenPos(window_bb.Min + ImVec2(style.WindowPadding.x, ImGui::GetFrameHeight() + style.WindowBorderSize * 3.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.WindowPadding);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildPadding, style.WindowPadding);
        bool main_content_child = ImAdd::BeginChild("body", { "Aimbot", "Silent Aim", "Visuals", "Rage", "Settings" }, &tab_index, ImGui::GetContentRegionAvail() - style.WindowPadding);
        ImGui::PopStyleVar(2);

        if (main_content_child)
        {
                float group_width = ImTrunc((ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2);
                float group_height = ImTrunc((ImGui::GetContentRegionAvail().y - style.ItemSpacing.y) / 2);

                if (tab_index == 0)
                {
                    ImGui::BeginGroup();
                    static int aimbot_subtab = 0;
                    if (ImAdd::BeginChild("Aimbot", { "Main", "Settings" }, &aimbot_subtab, ImVec2(group_width, ImGui::GetContentRegionAvail().y)))
                    {
                        if (aimbot_subtab == 0)
                        {
                            ImAdd::CheckBox("Enabled", &settings::aimbot::enabled);
                            ImGui::SameLine();
                            static ImGuiKey aimbot_key = ImGuiKey_None;
                            aimbot_key = keybind::vk_to_imgui_key(settings::aimbot::keybind);
                            if (ImAdd::KeyBind("## Aimbot Keybind", &aimbot_key, ImVec2(0, 0), &settings::aimbot::activation_mode))
                            {
                                settings::aimbot::keybind = keybind::imgui_key_to_vk(aimbot_key);
                            }

                            ImAdd::CheckBox("Sticky Aim", &settings::aimbot::sticky_aim);

                            ImAdd::CheckBox("Draw FOV", &settings::aimbot::draw_fov);
                            if (settings::aimbot::draw_fov)
                            {
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() * 2.0f - style.ItemSpacing.x + style.ChildPadding.x);
                                ImAdd::ColorEdit4("## fov circle color", settings::aimbot::fov_circle_colour);
                                
                                ImGui::SameLine();
                                ImAdd::ColorEdit4("## fov outline color", settings::aimbot::fov_outline_colour);
                            }

                            ImAdd::SliderFloat("FOV", &settings::aimbot::fov, 0.0f, 1000.0f, "%.0f");

                            ImGui::Text("Mode");
                            ImAdd::Combo("## Aimbot Mode", &settings::aimbot::mode, { "Mouse", "Camera" });
                        }
                        else if (aimbot_subtab == 1)
                        {
                            ImGui::Text("Target Part");
                            ImAdd::Combo("## Target Part", &settings::aimbot::target_part, { "Closest", "Head", "HumanoidRootPart", "LeftArm", "RightArm", "LeftLeg", "RightLeg" });

                            ImAdd::CheckBox("Air Part", &settings::aimbot::air_part_enabled);
                            if (settings::aimbot::air_part_enabled)
                            {
                                ImGui::Text("Air Part");
                                ImAdd::Combo("## Air Part", &settings::aimbot::air_part, { "Closest", "Head", "HumanoidRootPart", "LeftArm", "RightArm", "LeftLeg", "RightLeg" });
                            }

                            ImAdd::CheckBox("Enable Smoothing", &settings::aimbot::smoothing);
                            if (settings::aimbot::smoothing)
                            {
                                ImAdd::SliderFloat("Smoothing X", &settings::aimbot::smoothingx, 1.0f, 100.0f, "%.1f");
                                
                                ImAdd::SliderFloat("Smoothing Y", &settings::aimbot::smoothingy, 1.0f, 100.0f, "%.1f");

                                ImGui::Text("Smoothing Style");
                                ImAdd::Combo("## Smoothing Style", &settings::aimbot::smoothing_style, { "None", "Linear", "EaseInQuad", "EaseOutQuad", "EaseInOutQuad", "EaseInCubic", "EaseOutCubic", "EaseInOutCubic", "EaseInSine", "EaseOutSine", "EaseInOutSine" });
                            }

                            ImAdd::CheckBox("Enable Prediction", &settings::aimbot::enable_prediction);
                            if (settings::aimbot::enable_prediction)
                            {
                                ImAdd::SliderFloat("Prediction X", &settings::aimbot::prediction_x, 0.0f, 20.0f, "%.1f");
                                
                                ImAdd::SliderFloat("Prediction Y", &settings::aimbot::prediction_y, 0.0f, 20.0f, "%.1f");
                            }

                            ImAdd::CheckBox("Air Prediction", &settings::aimbot::air_prediction_enabled);
                            if (settings::aimbot::air_prediction_enabled)
                            {
                                ImAdd::SliderFloat("Air Prediction X", &settings::aimbot::air_prediction_x, 0.0f, 20.0f, "%.1f");
                                
                                ImAdd::SliderFloat("Air Prediction Y", &settings::aimbot::air_prediction_y, 0.0f, 20.0f, "%.1f");
                            }

                            ImAdd::CheckBox("Offset", &settings::aimbot::offset_enabled);
                            if (settings::aimbot::offset_enabled)
                            {
                                ImAdd::SliderFloat("Offset X", &settings::aimbot::offset_x, -100.0f, 100.0f, "%.1f");
                                
                                ImAdd::SliderFloat("Offset Y", &settings::aimbot::offset_y, -100.0f, 100.0f, "%.1f");
                            }
                        }

                        ImAdd::EndChild();
                    }
                    ImGui::EndGroup();

                    ImGui::SameLine(0, style.ItemSpacing.x);

                    ImGui::BeginGroup();
                    if (ImAdd::BeginChild("Aimbot Settings", ImVec2(group_width, ImGui::GetContentRegionAvail().y)))
                    {
                        ImAdd::CheckBox("Use FOV", &settings::aimbot::use_fov);
                        ImAdd::CheckBox("Team Check", &settings::aimbot::teamcheck);
                        ImAdd::CheckBox("Knock Check", &settings::aimbot::knock_check);
                        ImAdd::CheckBox("Health Check", &settings::aimbot::health_check_enabled);
                        
                        if (settings::aimbot::health_check_enabled)
                        {
                            ImAdd::SliderFloat("Min Health", &settings::aimbot::min_health, 0.0f, 100.0f, "%.1f");
                        }

                        ImAdd::EndChild();
                    }
                    ImGui::EndGroup();
                }
                else if (tab_index == 1)
                {
                    ImGui::BeginGroup();
                    static int silentaim_subtab = 0;
                    if (ImAdd::BeginChild("Silent Aim", { "Main", "Settings" }, &silentaim_subtab, ImVec2(group_width, ImGui::GetContentRegionAvail().y)))
                    {
                        if (silentaim_subtab == 0)
                        {
                            ImAdd::CheckBox("Enabled", &settings::silentaim::enabled);
                            ImGui::SameLine();
                            static ImGuiKey silentaim_key = ImGuiKey_None;
                            silentaim_key = keybind::vk_to_imgui_key(settings::silentaim::keybind);
                            if (ImAdd::KeyBind("## Silent Aim Keybind", &silentaim_key, ImVec2(0, 0), &settings::silentaim::activation_mode))
                            {
                                settings::silentaim::keybind = keybind::imgui_key_to_vk(silentaim_key);
                            }

                            ImAdd::CheckBox("Sticky Aim", &settings::silentaim::sticky_aim);

                            ImAdd::CheckBox("Draw FOV", &settings::silentaim::draw_fov);
                            if (settings::silentaim::draw_fov)
                            {
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() * 2.0f - style.ItemSpacing.x + style.ChildPadding.x);
                                ImAdd::ColorEdit4("## fov circle color", settings::silentaim::fov_circle_colour);
                                
                                ImGui::SameLine();
                                ImAdd::ColorEdit4("## fov outline color", settings::silentaim::fov_outline_colour);
                                
                                ImAdd::CheckBox("Attach FOV to Target", &settings::silentaim::attach_fov_to_target);
                            }

                            ImAdd::SliderFloat("FOV", &settings::silentaim::fov, 0.0f, 1000.0f, "%.0f");

                        }
                        else if (silentaim_subtab == 1)
                        {
                            ImGui::Text("Target Part");
                            ImAdd::Combo("## Target Part", &settings::silentaim::target_part, { "Nearest Point", "Closest", "Head", "HumanoidRootPart", "LeftArm", "RightArm", "LeftLeg", "RightLeg" });

                            ImAdd::CheckBox("Enable Prediction", &settings::silentaim::enable_prediction);
                            if (settings::silentaim::enable_prediction)
                            {
                                ImAdd::SliderFloat("Prediction X", &settings::silentaim::prediction_x, 0.0f, 20.0f, "%.1f");
                                
                                ImAdd::SliderFloat("Prediction Y", &settings::silentaim::prediction_y, 0.0f, 20.0f, "%.1f");
                            }
                        }

                        ImAdd::EndChild();
                    }
                    ImGui::EndGroup();

                    ImGui::SameLine(0, style.ItemSpacing.x);

                    ImGui::BeginGroup();
                    if (ImAdd::BeginChild("Silent Aim Settings", ImVec2(group_width, ImGui::GetContentRegionAvail().y)))
                    {
                        ImAdd::CheckBox("Use Aimbot Target", &settings::silentaim::use_aimbot_target);
                        ImAdd::CheckBox("Use FOV", &settings::silentaim::use_fov);
                        ImAdd::CheckBox("Team Check", &settings::silentaim::teamcheck);
                        ImAdd::CheckBox("Gun Check", &settings::silentaim::guncheck);
                        ImAdd::CheckBox("Knock Check", &settings::silentaim::knock_check);
                        ImAdd::CheckBox("Health Check", &settings::silentaim::health_check_enabled);
                        
                        if (settings::silentaim::health_check_enabled)
                        {
                            ImAdd::SliderFloat("Min Health", &settings::silentaim::min_health, 0.0f, 100.0f, "%.1f");
                        }

                        ImAdd::CheckBox("Draw Target Dot", &settings::silentaim::draw_target_dot);
                        if (settings::silentaim::draw_target_dot)
                        {
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() - style.ItemSpacing.x + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## target dot color", settings::silentaim::target_dot_color);
                        }

                        ImAdd::EndChild();
                    }
                    ImGui::EndGroup();
                }
                else if (tab_index == 2)
                {
                    float group_width = ImTrunc((ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2);

                    static int enemy_client_settings_tab = 0;

                    ImGui::BeginGroup();

                    if (ImAdd::BeginChild("group1", { "Enemy", "Client", "Settings" }, &enemy_client_settings_tab, ImVec2(group_width, 0.0f)))
                    {
                        if (enemy_client_settings_tab == 0)
                        {
                            ImAdd::CheckBox("Enable Enemies", &settings::visuals::enable_enemies);
                            
                            ImAdd::CheckBox("Box", &settings::visuals::box);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## box color", settings::visuals::box_color);

                            ImAdd::CheckBox("Box Fill", &settings::visuals::box_fill);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## box fill color", settings::visuals::box_fill_color);

                            ImAdd::CheckBox("Name", &settings::visuals::name);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## name color", settings::visuals::name_color);

                            ImAdd::CheckBox("Avatar", &settings::visuals::avatar);

                            ImAdd::CheckBox("Healthbar", &settings::visuals::healthbar);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## healthbar color", settings::visuals::healthbar_color);

                            ImAdd::CheckBox("Health Percent", &settings::visuals::health_percent);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## health percent color", settings::visuals::health_percent_color);

                            ImAdd::CheckBox("Armor Bar", &settings::visuals::armorbar);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## armorbar color", settings::visuals::armorbar_color);

                            ImAdd::CheckBox("Distance", &settings::visuals::distance);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## distance color", settings::visuals::distance_color);

                            ImAdd::CheckBox("Tool", &settings::visuals::tool);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## tool color", settings::visuals::tool_color);

                            ImAdd::CheckBox("Flags", &settings::visuals::flags);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## flags state color", settings::visuals::flags_state_colour);


                            ImAdd::CheckBox("Chams", &settings::visuals::chams);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() * 2 - style.ItemSpacing.x + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## chams fill color", settings::visuals::chams_fill_color);
                            ImGui::SameLine();
                            ImAdd::ColorEdit4("Outline Color", settings::visuals::chams_outline_color);

                            ImAdd::CheckBox("Target Warning Icon", &settings::visuals::target_warning_icon);
                        }
                        else if (enemy_client_settings_tab == 1)
                        {
                            ImAdd::CheckBox("Enable Client", &settings::visuals::enable_client);
                            
                            ImAdd::CheckBox("Box", &settings::visuals::client_box);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local box color", settings::visuals::client_box_color);

                            ImAdd::CheckBox("Box Fill", &settings::visuals::client_box_fill);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local box fill color", settings::visuals::client_box_fill_color);

                            ImAdd::CheckBox("Name", &settings::visuals::client_name);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local name color", settings::visuals::client_name_color);

                            ImAdd::CheckBox("Avatar", &settings::visuals::client_avatar);

                            ImAdd::CheckBox("Healthbar", &settings::visuals::client_healthbar);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local healthbar color", settings::visuals::client_healthbar_color);

                            ImAdd::CheckBox("Health Percent", &settings::visuals::client_health_percent);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local health percent color", settings::visuals::client_health_percent_color);

                            ImAdd::CheckBox("Armor Bar", &settings::visuals::client_armorbar);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local armorbar color", settings::visuals::client_armorbar_color);

                            ImAdd::CheckBox("Distance", &settings::visuals::client_distance);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local distance color", settings::visuals::client_distance_color);

                            ImAdd::CheckBox("Tool", &settings::visuals::client_tool);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local tool color", settings::visuals::client_tool_color);

                            ImAdd::CheckBox("Flags", &settings::visuals::client_flags);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local flags state color", settings::visuals::client_flags_state_colour);

                            ImAdd::CheckBox("Chams", &settings::visuals::client_chams);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() * 2 - style.ItemSpacing.x + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## local chams fill color", settings::visuals::client_chams_fill_color);
                            ImGui::SameLine();
                            ImAdd::ColorEdit4("## local chams outline color", settings::visuals::client_chams_outline_color);

                            ImAdd::CheckBox("Material", &settings::visuals::client_material);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            float material_color[4] = { settings::visuals::client_material_color[0], settings::visuals::client_material_color[1], settings::visuals::client_material_color[2], 1.0f };
                            ImAdd::ColorEdit4("## local material color", material_color);
                            settings::visuals::client_material_color[0] = material_color[0];
                            settings::visuals::client_material_color[1] = material_color[1];
                            settings::visuals::client_material_color[2] = material_color[2];

                            ImAdd::CheckBox("Headless", &settings::visuals::client_headless);
                            ImAdd::CheckBox("Korblox", &settings::visuals::client_korblox);
                        }
                        else if (enemy_client_settings_tab == 2)
                        {
                            ImGui::Text("Font Type");
                            ImAdd::Combo("## Font Type", &settings::visuals::esp_font, { "Visitor", "Smallest Pixel", "Arial" });

                            ImGui::Text("Box Type");
                            ImAdd::Combo("## Box Type", &settings::visuals::box_type, { "Bounding Box", "Cornered Box" });

                            ImGui::Text("Name Display Type");
                            ImAdd::Combo("## Name Display Type", &settings::visuals::name_display_type, { "Display Name", "Username" });

                            ImGui::Text("Distance Measurement");
                            ImAdd::Combo("## Distance Measurement", &settings::visuals::distance_measurement, { "Studs", "Meters" });

                            ImGui::Text("Chams Type");
                            ImAdd::Combo("## Chams Type", &settings::visuals::chams_type, { "Cube", "Highlight", "Mesh" });

                            ImAdd::CheckBox("Dynamic Healthbar", &settings::visuals::health_based_healthbar);

                            ImAdd::CheckBox("Blend", &settings::visuals::blend);
                            if (settings::visuals::blend)
                            {
                                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() * 2 - style.ItemSpacing.x + style.ChildPadding.x);
                                ImAdd::ColorEdit4("## colorblend start", settings::visuals::name_color_blend_start);
                                ImGui::SameLine();
                                ImAdd::ColorEdit4("## colorblend end", settings::visuals::name_color_blend_end);
                            }

                            ImAdd::CheckBox("Knock Check", &settings::visuals::knock_check);

                            ImAdd::CheckBox("Debug Wallcheck", &settings::visuals::debug_wallcheck);

                            ImAdd::SliderFloat("Fade In Speed", &settings::visuals::fade_in_speed, 0.1f, 50.0f, "%.1f");
                            ImAdd::SliderFloat("Fade Out Speed", &settings::visuals::fade_out_speed, 0.1f, 50.0f, "%.1f");
                        }
                    }
                    ImAdd::EndChild();

                    ImGui::EndGroup();
                    ImGui::SameLine();
                    ImGui::BeginGroup();

                    if (ImAdd::BeginChild("group3", ImVec2(0.0f, 0.0f)))
                    {
                        ImAdd::CheckBox("Fog", &settings::lighting::fog::enabled);
                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                        float fog_color[4] = { settings::lighting::fog::fog_r, settings::lighting::fog::fog_g, settings::lighting::fog::fog_b, 1.0f };
                        ImAdd::ColorEdit4("## fog color", fog_color);
                        settings::lighting::fog::fog_r = fog_color[0];
                        settings::lighting::fog::fog_g = fog_color[1];
                        settings::lighting::fog::fog_b = fog_color[2];

                        ImAdd::CheckBox("Exposure", &settings::lighting::exposure::enabled);

                        ImAdd::CheckBox("Skybox", &settings::lighting::skybox::enabled);

                        if (settings::lighting::skybox::enabled)
                        {
                            static const char* preset_names[] = {
                                "Space",
                                "Pink Sky",
                                "Minecraft",
                                "Nighttime Cloudy",
                                "Sparkling Night",
                                "Winterness",
                                "Dark Crimson",
                                "Nebula",
                                "Tropical",
                                "Doom 2"
                            };

                            static int last_preset_index = -1;
                            
                            ImAdd::Combo("## Skybox Preset", &settings::lighting::skybox::preset_index, std::vector<const char*>(preset_names, preset_names + 10));
                            
                            if (settings::lighting::skybox::preset_index != last_preset_index && settings::lighting::skybox::preset_index >= 0 && settings::lighting::skybox::preset_index < 10)
                            {
                                const auto& preset = lighting::skybox::PRESETS[settings::lighting::skybox::preset_index + 1];
                                settings::lighting::skybox::skybox_bk = preset.bk ? preset.bk : "";
                                settings::lighting::skybox::skybox_dn = preset.dn ? preset.dn : "";
                                settings::lighting::skybox::skybox_ft = preset.ft ? preset.ft : "";
                                settings::lighting::skybox::skybox_lf = preset.lf ? preset.lf : "";
                                settings::lighting::skybox::skybox_rt = preset.rt ? preset.rt : "";
                                settings::lighting::skybox::skybox_up = preset.up ? preset.up : "";
                                settings::lighting::skybox::apply_skybox = true;
                                last_preset_index = settings::lighting::skybox::preset_index;
                            }
                        }

                        ImAdd::CheckBox("Clock Time", &settings::lighting::clocktime::enabled);

                        if (settings::lighting::clocktime::enabled)
                        {
                            ImAdd::SliderFloat("Time", &settings::lighting::clocktime::clock_time, 0.0f, 24.0f, "%.1f");
                        }

                        if (settings::lighting::fog::enabled)
                        {
                            ImAdd::SliderFloat("Fog Start", &settings::lighting::fog::fog_start, 0.0f, 1000.0f, "%.1f");
                            ImAdd::SliderFloat("Fog End", &settings::lighting::fog::fog_end, 0.0f, 10000.0f, "%.1f");
                        }

                        if (settings::lighting::exposure::enabled)
                        {
                            ImAdd::SliderFloat("Exposure", &settings::lighting::exposure::exposure, -2.0f, 2.0f, "%.2f");
                        }

                        ImAdd::EndChild();
                    }

                    ImGui::EndGroup();
                }
                else if (tab_index == 3)
                {
                    ImGui::BeginGroup();
                    if (ImAdd::BeginChild("Movement", ImVec2(group_width, group_height)))
                    {
                        ImAdd::CheckBox("Speedhack", &settings::movement::speedhack::enabled);
                        ImGui::SameLine();
                        static ImGuiKey speedhack_key = ImGuiKey_None;
                        speedhack_key = keybind::vk_to_imgui_key(settings::movement::speedhack::keybind);
                        if (ImAdd::KeyBind("##speedhack_keybind", &speedhack_key, ImVec2(0, 0), &settings::movement::speedhack::activation_mode))
                        {
                            settings::movement::speedhack::keybind = keybind::imgui_key_to_vk(speedhack_key);
                        }

                        ImAdd::CheckBox("Flyhack", &settings::movement::flyhack::enabled);
                        ImGui::SameLine();
                        static ImGuiKey flyhack_key = ImGuiKey_None;
                        flyhack_key = keybind::vk_to_imgui_key(settings::movement::flyhack::keybind);
                        if (ImAdd::KeyBind("##flyhack_keybind", &flyhack_key, ImVec2(0, 0), &settings::movement::flyhack::activation_mode))
                        {
                            settings::movement::flyhack::keybind = keybind::imgui_key_to_vk(flyhack_key);
                        }

                        ImAdd::CheckBox("Tickrate", &settings::movement::tickrate::enabled);
                        ImAdd::CheckBox("Noclip", &settings::rage::noclip);

                        ImAdd::EndChild();
                    }

                    ImGui::SameLine();

                    if (ImAdd::BeginChild("Movement Settings", ImVec2(group_width, group_height)))
                    {
                        if (settings::movement::speedhack::enabled)
                        {
                            ImGui::Text("Speedhack Mode");
                            ImAdd::Combo("##speedhack_mode", &settings::movement::speedhack::mode, { "Velocity", "WalkSpeed" });
                            ImAdd::SliderFloat("Speedhack Speed", &settings::movement::speedhack::speed, 1.0f, 1000.0f, "%.1f");
                        }

                        if (settings::movement::flyhack::enabled)
                        {
                            ImGui::Text("Flyhack Mode");
                            ImAdd::Combo("##flyhack_mode", &settings::movement::flyhack::mode, { "Velocity", "Position", "CFrame" });
                            ImAdd::SliderFloat("Flyhack Speed", &settings::movement::flyhack::speed, 1.0f, 1000.0f, "%.1f");
                        }

                        if (settings::movement::tickrate::enabled)
                        {
                            ImAdd::SliderFloat("Tickrate Value", &settings::movement::tickrate::value, 0.0f, 1000.0f, "%.1f");
                        }
                        ImAdd::EndChild();
                    }

                    ImGui::EndGroup();

                    ImGui::BeginGroup();
                    float big_child_width = ImTrunc((ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2);
                    float big_child_height = ImGui::GetContentRegionAvail().y;

                    if (ImAdd::BeginChild("Rage 1", ImVec2(big_child_width, big_child_height)))
                    {
                        ImAdd::CheckBox("HitSounds", &settings::rage::hitsounds);
                        ImAdd::CheckBox("Hit Tracers", &settings::rage::hit_tracers);
                        if (settings::rage::hit_tracers)
                        {
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## hit tracers color", settings::rage::hit_tracers_color);
                        }
                        ImAdd::CheckBox("RapidFire", &settings::rage::rapidfire);
                        ImAdd::CheckBox("Orbit", &settings::movement::orbit::enabled);
                        ImAdd::CheckBox("Hitbox Expander", &settings::rage::hitbox_expander::enabled);
                        ImAdd::CheckBox("360 Spin", &settings::rage::spin360::enabled);
                        ImGui::SameLine();
                        static ImGuiKey spin360_key = ImGuiKey_None;
                        spin360_key = keybind::vk_to_imgui_key(settings::rage::spin360::keybind);
                        if (ImAdd::KeyBind("##spin360_keybind", &spin360_key, ImVec2(0, 0), &settings::rage::spin360::activation_mode))
                        {
                            settings::rage::spin360::keybind = keybind::imgui_key_to_vk(spin360_key);
                        }

                        ImAdd::EndChild();
                    }

                    ImGui::SameLine();

                    if (ImAdd::BeginChild("Rage 2", ImVec2(big_child_width, big_child_height)))
                    {
                        if (settings::rage::hitsounds)
                        {
                            ImGui::Text("Detection Type");
                            ImAdd::Combo("##HitsoundMethod", &settings::rage::hitsound_method, { "Health", "Click", "Ammo" });
                            ImGui::Text("Hitsound Type");
                            ImAdd::Combo("##HitSound", &settings::rage::hitsound_type, { "Among Us", "Skeet", "Beep", "Bonk", "Bubble", "COD", "CSGO", "Fairy", "Fatality", "Osu", "Neverlose" });
                        }

                        if (settings::rage::hit_tracers)
                        {
                            ImGui::Text("Detection Type");
                            ImAdd::Combo("##HitTracersMethod", &settings::visuals::hit_tracers_method, { "Health", "Click", "Ammo" });
                            ImAdd::SliderFloat("Duration", &settings::rage::hit_tracers_duration, 0.1f, 5.0f, "%.1f");
                        }

                        if (settings::rage::hitbox_expander::enabled)
                        {
                            ImAdd::SliderFloat("Size X", &settings::rage::hitbox_expander::size_x, 0.1f, 30.0f, "%.1f");
                            ImAdd::SliderFloat("Size Y", &settings::rage::hitbox_expander::size_y, 0.1f, 30.0f, "%.1f");
                            ImAdd::SliderFloat("Size Z", &settings::rage::hitbox_expander::size_z, 0.1f, 30.0f, "%.1f");
                            ImAdd::CheckBox("Knock Check", &settings::rage::hitbox_expander::knock_check);
                            ImAdd::CheckBox("View Hitbox", &settings::visuals::view_hitbox);
                            ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImAdd::GetColorPickerWidth() + style.ChildPadding.x);
                            ImAdd::ColorEdit4("## view hitbox color", settings::visuals::view_hitbox_color);
                        }

                        if (settings::rage::spin360::enabled)
                        {
                            ImAdd::SliderFloat("Spin Speed", &settings::rage::spin360::speed, 1.0f, 10.0f, "%.1f");
                        }

                        if (settings::movement::orbit::enabled)
                        {
                            ImGui::Text("Target Type");
                            ImAdd::Combo("##Target Type", &settings::movement::orbit::orbit_type, { "Aimbot Target", "Silent Target" });
                            ImAdd::SliderFloat("Orbit Speed", &settings::movement::orbit::speed, 1.0f, 100.0f, "%.1f");
                            ImAdd::SliderFloat("Orbit Radius", &settings::movement::orbit::radius, 1.0f, 50.0f, "%.1f");
                            ImAdd::SliderFloat("Orbit Height Offset", &settings::movement::orbit::height_offset, 0.0f, 50.0f, "%.1f");
                            ImAdd::CheckBox("Spectate Target", &settings::movement::orbit::spectate_target);
                            ImAdd::CheckBox("Randomize", &settings::movement::orbit::randomize);
                            if (settings::movement::orbit::randomize)
                            {
                                ImAdd::SliderFloat("Randomize X", &settings::movement::orbit::randomize_x, 0.0f, 50.0f, "%.1f");
                                ImAdd::SliderFloat("Randomize Y", &settings::movement::orbit::randomize_y, 0.0f, 50.0f, "%.1f");
                            }
                        }

                        ImAdd::EndChild();
                    }

                    ImGui::EndGroup();
                }
                else if (tab_index == 4)
                {
                    float group_width = ImTrunc((ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2);

                    ImGui::BeginGroup();
                    if (ImAdd::BeginChild("Settings", ImVec2(group_width, ImGui::GetContentRegionAvail().y)))
                    {
                        ImGui::AlignTextToFramePadding();
                        static ImGuiKey menu_key = ImGuiKey_None;
                        menu_key = keybind::vk_to_imgui_key(settings::menu::menu_keybind);
                        if (ImAdd::KeyBind("## Menu Keybind", &menu_key))
                        {
                            settings::menu::menu_keybind = keybind::imgui_key_to_vk(menu_key);
                        }
                        
                        ImAdd::CheckBox("Watermark", &settings::menu::watermark);
                        ImAdd::CheckBox("Streamproof", &settings::menu::streamproof);
                        ImAdd::CheckBox("V-Sync", &settings::menu::vsync);
                        ImAdd::CheckBox("Hide Console", &settings::menu::hide_console);
                        ImAdd::CheckBox("Performance Mode", &settings::menu::performance_mode);

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImAdd::CheckBox("Show Custom Entities", &settings::custom_entities::show_custom_entities);
                        ImAdd::CheckBox("Auto Refresh", &settings::custom_entities::auto_refresh);
                        if (settings::custom_entities::auto_refresh)
                        {
                            ImAdd::SliderFloat("Refresh Rate", &settings::custom_entities::refresh_rate, 0.001f, 1.0f, "%.3f");
                        }

                        static char path_input[256];
                        strncpy_s(path_input, settings::custom_entities::current_input.c_str(), sizeof(path_input));
                        if (ImGui::InputText("Path", path_input, sizeof(path_input)))
                        {
                            settings::custom_entities::current_input = path_input;
                        }

                        if (ImGui::Button("Set"))
                        {
                            custom_entities::set_container(settings::custom_entities::current_input);
                        }

                        ImGui::Text("Entities:");
                        std::vector<settings::custom_entities::custom_container_t> containers_snapshot;
                        {
                            std::lock_guard<std::mutex> lock(custom_entities::containers_mtx);
                            containers_snapshot = settings::custom_entities::containers;
                        }
                        for (const auto& container : containers_snapshot)
                        {
                            ImGui::Text(container.path.c_str());
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        if (ImAdd::ButtonAccent("Unload", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                        {
                            exit(0);
                        }

                        ImAdd::EndChild();
                    }
                    ImGui::EndGroup();

                    ImGui::SameLine(0, style.ItemSpacing.x);

                    ImGui::BeginGroup();
                    if (ImAdd::BeginChild("Configs", ImVec2(group_width, ImGui::GetContentRegionAvail().y)))
                    {
                        static char config_name[64] = "";
                        static int selected_config = -1;
                        static std::vector<config::config_info_t> config_list;
                        static bool refresh_list = true;

                        if (refresh_list)
                        {
                            config_list = config::get_config_list();
                            refresh_list = false;
                        }

                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        bool input_active = ImGui::InputText("## Config Name", config_name, sizeof(config_name));
                        
                        if (input_active && ImGui::IsItemActive() && selected_config >= 0)
                        {
                            selected_config = -1;
                        }

                        ImGui::Spacing();

                        if (ImAdd::ButtonAccent("Save Config", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                        {
                            if (strlen(config_name) > 0)
                            {
                                if (config::save_config(config_name))
                                {
                                    config_list = config::get_config_list();
                                    selected_config = -1;
                                    memset(config_name, 0, sizeof(config_name));
                                    ImGui::SetKeyboardFocusHere(-1);
                                }
                            }
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        if (ImGui::BeginChild("## Config List", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetFontSize() - style.ItemSpacing.y * 2 - 60), true))
                        {
                            if (config_list.empty())
                            {
                                ImGui::TextDisabled("No configs found");
                            }
                            else
                            {
                                for (size_t i = 0; i < config_list.size(); i++)
                                {
                                    bool is_selected = (selected_config == static_cast<int>(i));
                                    if (ImGui::Selectable(config_list[i].name.c_str(), is_selected))
                                    {
                                        selected_config = static_cast<int>(i);
                                        strncpy_s(config_name, sizeof(config_name), config_list[i].name.c_str(), _TRUNCATE);
                                        if (ImGui::IsItemFocused())
                                        {
                                            ImGui::SetKeyboardFocusHere(-1);
                                        }
                                    }
                                }
                            }
                        }
                        ImGui::EndChild();

                        ImGui::Spacing();

                        if (selected_config >= 0 && selected_config < static_cast<int>(config_list.size()))
                        {
                            if (ImAdd::ButtonAccent("Load Config", ImVec2((ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2, 0)))
                            {
                                config::load_config(config_list[selected_config].name);
                            }

                            ImGui::SameLine();

                            if (ImAdd::ButtonAccent("Delete Config", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                if (config::delete_config(config_list[selected_config].name))
                                {
                                    config_list = config::get_config_list();
                                    selected_config = -1;
                                    memset(config_name, 0, sizeof(config_name));
                                }
                            }
                        }

                        ImGui::Spacing();

                        if (selected_config >= 0)
                        {
                            if (ImAdd::ButtonAccent("Open File Location", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                            {
                                config::open_file_location();
                            }
                        }

                        ImAdd::EndChild();
                    }
                    ImGui::EndGroup();
                }
            }
            ImAdd::EndChild();
        }
        ImGui::End();
    }

    if (m_bExplorerWindowOpen)
    {
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Once, ImVec2(0.5f, 0.5f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        bool explorer_window = ImGui::Begin("Explorer", &m_bExplorerWindowOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize);
        ImGui::PopStyleVar(2);

        if (explorer_window)
        {
            ImRect window_bb(ImGui::GetCurrentWindow()->Rect());

            if (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_NoBackground)
            {
                ImGuiWindow* window = ImGui::GetCurrentWindow();

                window->DrawList->AddRectFilled(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_WindowBg));

                if (style.WindowBorderSize > 0.0f)
                {
                    window->DrawList->AddRect(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, ImDrawFlags_None, style.WindowBorderSize);
                    window->DrawList->AddRect(window_bb.Min + ImVec2(style.WindowBorderSize, style.WindowBorderSize), window_bb.Max - ImVec2(style.WindowBorderSize, style.WindowBorderSize), ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, ImDrawFlags_None, style.WindowBorderSize);
                
                    window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize * 2.0f, style.WindowBorderSize * 2.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Min.y + style.WindowBorderSize * 2.0f), ImGui::GetColorU32(ImGuiCol_Header), style.WindowBorderSize);
                    window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize * 2.0f, style.WindowBorderSize * 3.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Min.y + style.WindowBorderSize * 3.0f), ImGui::GetColorU32(ImGuiCol_HeaderActive), style.WindowBorderSize);
                    window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize * 2.0f, style.WindowBorderSize * 4.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize, window_bb.Min.y + style.WindowBorderSize * 4.0f), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
                }

                ImAdd::RenderText(window_bb.Min + ImVec2(style.FramePadding.x + style.WindowBorderSize * 3.0f, style.FramePadding.y + style.WindowBorderSize * 4.0f), "Explorer", NULL, false, true);
            }

            ImGui::SetCursorScreenPos(window_bb.Min + ImVec2(style.WindowPadding.x, ImGui::GetFrameHeight() + style.WindowBorderSize * 3.0f));

            float bottom_height = ImGui::GetFontSize() * 20.0f;
            float available_height = ImGui::GetContentRegionAvail().y;
            float explorer_height = available_height - bottom_height - style.ItemSpacing.y;
            
            if (explorer_height > 0 && available_height > 0 && bottom_height > 0)
            {
                if (ImAdd::BeginChild("Explorer", ImVec2(0, explorer_height)))
                {
                    if (explorer::explorer && !explorer::explorer->is_refreshing.load())
                    {
                        if (explorer::explorer->root)
                        {
                            try
                            {
                                explorer::explorer->render_node(explorer::explorer->root);
                            }
                            catch (...)
                            {
                                ImGui::TextDisabled("Error rendering explorer tree");
                            }
                        }
                        else
                        {
                            ImGui::TextDisabled("No game instance found");
                        }
                    }
                    else if (explorer::explorer && explorer::explorer->is_refreshing.load())
                    {
                        ImGui::TextDisabled("Refreshing...");
                    }
                    else
                    {
                        ImGui::TextDisabled("Explorer not initialized");
                    }
                    ImAdd::EndChild();
                }

                ImGui::Spacing();
            }
            
            if (ImAdd::BeginChild("Settings", ImVec2(0, bottom_height)))
            {
                if (explorer::explorer && !explorer::explorer->is_refreshing.load())
                {
                    try
                    {
                        explorer::explorer->render_settings();
                        explorer::explorer->render_properties();
                    }
                    catch (...)
                    {
                        ImGui::TextDisabled("Error rendering explorer settings/properties");
                    }
                }
                else if (explorer::explorer && explorer::explorer->is_refreshing.load())
                {
                    ImGui::TextDisabled("Refreshing...");
                }
                else
                {
                    ImGui::TextDisabled("Explorer not initialized");
                }
                ImAdd::EndChild();
            }
        }
        ImGui::End();
    }

    if (m_bClientWindowOpen)
    {
        ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Once, ImVec2(0.5f, 0.5f));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        bool client_window = ImGui::Begin("Client - ESP Preview", &m_bClientWindowOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar(2);

        if (client_window)
        {
            ImRect window_bb(ImGui::GetCurrentWindow()->Rect());

            if (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_NoBackground)
            {
                ImGuiWindow* window = ImGui::GetCurrentWindow();

                window->DrawList->AddRectFilled(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_WindowBg));

                if (style.WindowBorderSize > 0.0f)
                {
                    window->DrawList->AddRect(window_bb.Min, window_bb.Max, ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, ImDrawFlags_None, style.WindowBorderSize);
                    window->DrawList->AddRect(window_bb.Min + ImVec2(style.WindowBorderSize, style.WindowBorderSize), window_bb.Max - ImVec2(style.WindowBorderSize, style.WindowBorderSize), ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding, ImDrawFlags_None, style.WindowBorderSize);
                
                    window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize * 2.0f, style.WindowBorderSize * 2.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize * 2.0f, window_bb.Min.y + style.WindowBorderSize * 2.0f), ImGui::GetColorU32(ImGuiCol_Header), style.WindowBorderSize);
                    window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize * 2.0f, style.WindowBorderSize * 3.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize * 2.0f, window_bb.Min.y + style.WindowBorderSize * 3.0f), ImGui::GetColorU32(ImGuiCol_HeaderActive), style.WindowBorderSize);
                    window->DrawList->AddLine(window_bb.Min + ImVec2(style.WindowBorderSize * 2.0f, style.WindowBorderSize * 4.0f), ImVec2(window_bb.Max.x - style.WindowBorderSize * 2.0f, window_bb.Min.y + style.WindowBorderSize * 4.0f), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
                }

                ImAdd::RenderText(window_bb.Min + ImVec2(style.FramePadding.x + style.WindowBorderSize * 3.0f, style.FramePadding.y + style.WindowBorderSize * 4.0f), "Client", NULL, false, true);
            }

            ImGui::SetCursorScreenPos(window_bb.Min + ImVec2(style.WindowPadding.x, ImGui::GetFrameHeight() + style.WindowBorderSize * 3.0f));

            if (ImGui::BeginChild("esp_preview_content", ImGui::GetContentRegionAvail(), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground))
            {
                static bool model_parsed = false;
                static c_obj_model parsed_model;
                static std::string last_loaded_user_id;

                static std::unordered_map<std::string, float> rotation_map;
                static std::unordered_map<std::string, float> pitch_map;
                static std::unordered_map<std::string, float> zoom_map;
                static std::unordered_map<std::string, float> ambient_map;
                static std::unordered_map<std::string, float> diffuse_map;

                std::string local_user_id = "";
                std::uint64_t local_player_address = memory->read<std::uint64_t>(game::players.address + Offsets::Player::LocalPlayer);
                if (local_player_address != 0) {
                    std::uint64_t user_id = memory->read<std::uint64_t>(local_player_address + Offsets::Player::UserId);
                    local_user_id = std::to_string(user_id);
                }

                c_avatar_3d_data* avatar_3d = c_avatar_3d_api::get().request_data(local_user_id);
                e_avatar_3d_load_state load_state = c_avatar_3d_api::get().get_state(local_user_id);

                ImVec2 child_size = ImGui::GetContentRegionAvail();

                if (load_state == e_avatar_3d_load_state::failed) {
                    ImVec2 text_size = ImGui::CalcTextSize("Failed to load 3D avatar");
                    ImGui::SetCursorPos(ImVec2((child_size.x - text_size.x) * 0.5f, child_size.y * 0.5f));
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Failed to load 3D avatar");
                }
                else if (avatar_3d && avatar_3d->ready) {
                    if (last_loaded_user_id != local_user_id || !model_parsed) {
                        c_texture_cache::get().clear_user(local_user_id);

                        model_parsed = c_avatar_3d_api::get().parse_obj_model(avatar_3d->obj_data, parsed_model);

                        if (model_parsed && !avatar_3d->mtl_data.empty()) {
                            bool mtl_parsed = c_avatar_3d_api::get().parse_mtl_data(avatar_3d->mtl_data, parsed_model, avatar_3d->texture_hashes);

                            if (mtl_parsed && !avatar_3d->texture_data.empty()) {
                                std::unordered_set<int> requested_indices;

                                for (const auto& mat_pair : parsed_model.materials) {
                                    int tex_idx = mat_pair.second.texture_index;
                                    if (tex_idx >= 0 && tex_idx < static_cast<int>(avatar_3d->texture_data.size()) &&
                                        !avatar_3d->texture_data[tex_idx].empty() &&
                                        requested_indices.find(tex_idx) == requested_indices.end()) {

                                        c_texture_cache::get().request_texture(local_user_id, tex_idx, avatar_3d->texture_data[tex_idx], true);
                                        requested_indices.insert(tex_idx);
                                    }
                                }
                            }
                        }
                        last_loaded_user_id = local_user_id;
                    }

                    ImDrawList* preview_draw = ImGui::GetWindowDrawList();
                    ImVec2 child_window_pos = ImGui::GetWindowPos();
                    ImVec2 canvas_min = child_window_pos + ImGui::GetStyle().WindowPadding;
                    float preview_width = child_size.x;
                    float available_height = child_size.y;
                    ImVec2 canvas_center = ImVec2(canvas_min.x + preview_width * 0.5f, canvas_min.y + available_height * 0.5f);

                    if (rotation_map.find(local_user_id) == rotation_map.end()) rotation_map[local_user_id] = 0.0f;
                    if (pitch_map.find(local_user_id) == pitch_map.end()) pitch_map[local_user_id] = 0.0f;
                    if (zoom_map.find(local_user_id) == zoom_map.end()) zoom_map[local_user_id] = 1.2f;
                    if (ambient_map.find(local_user_id) == ambient_map.end()) ambient_map[local_user_id] = 1.0f;
                    if (diffuse_map.find(local_user_id) == diffuse_map.end()) diffuse_map[local_user_id] = 1.0f;

                    float& rotation = rotation_map[local_user_id];
                    float& pitch = pitch_map[local_user_id];
                    float& zoom = zoom_map[local_user_id];
                    float& ambient = ambient_map[local_user_id];
                    float& diffuse = diffuse_map[local_user_id];

                    rotation += 0.01f;
                    if (rotation > 3.14159f * 2.0f) rotation -= 3.14159f * 2.0f;

                    float preview_min_x = FLT_MAX, preview_min_y = FLT_MAX;
                    float preview_max_x = -FLT_MAX, preview_max_y = -FLT_MAX;

                    if (model_parsed && !parsed_model.vertices.empty()) {
                        float aabb_width = avatar_3d->aabb.max[0] - avatar_3d->aabb.min[0];
                        float aabb_height = avatar_3d->aabb.max[1] - avatar_3d->aabb.min[1];
                        float aabb_depth = avatar_3d->aabb.max[2] - avatar_3d->aabb.min[2];
                        float max_dim = max(aabb_width, max(aabb_height, aabb_depth));

                        float base_scale = 200.0f / max_dim;
                        float auto_fit_zoom = zoom;
                        if (max_dim > 10.0f) {
                            auto_fit_zoom = zoom * (10.0f / max_dim);
                        }

                        float scale = base_scale * auto_fit_zoom;

                        float cos_rot = std::cos(rotation);
                        float sin_rot = std::sin(rotation);
                        float cos_pitch = std::cos(pitch);
                        float sin_pitch = std::sin(pitch);

                        float aabb_center_x = (avatar_3d->aabb.min[0] + avatar_3d->aabb.max[0]) * 0.5f;
                        float aabb_center_y = (avatar_3d->aabb.min[1] + avatar_3d->aabb.max[1]) * 0.5f;
                        float aabb_center_z = (avatar_3d->aabb.min[2] + avatar_3d->aabb.max[2]) * 0.5f;

                        std::vector<std::tuple<float, int>> depth_sorted_faces;
                        depth_sorted_faces.reserve(parsed_model.faces.size());

                        for (size_t i = 0; i < parsed_model.faces.size(); i++) {
                            const auto& face = parsed_model.faces[i];
                            float avg_z = 0.0f;

                            for (int j = 0; j < 3; j++) {
                                const auto& v = parsed_model.vertices[face.vertex_indices[j]];
                                float x = v.x - aabb_center_x;
                                float y = v.y - aabb_center_y;
                                float z = v.z - aabb_center_z;

                                float rotated_x = x * cos_rot - z * sin_rot;
                                float rotated_z = x * sin_rot + z * cos_rot;
                                float final_z = y * sin_pitch + rotated_z * cos_pitch;

                                avg_z += final_z;
                            }
                            avg_z /= 3.0f;
                            depth_sorted_faces.push_back(std::make_tuple(avg_z, static_cast<int>(i)));
                        }

                        std::sort(depth_sorted_faces.begin(), depth_sorted_faces.end(),
                            [](const std::tuple<float, int>& a, const std::tuple<float, int>& b) { return std::get<0>(a) < std::get<0>(b); });

                        float light_dir_x = 0.5f;
                        float light_dir_y = 0.8f;
                        float light_dir_z = -0.3f;
                        float light_len = std::sqrt(light_dir_x * light_dir_x + light_dir_y * light_dir_y + light_dir_z * light_dir_z);
                        light_dir_x /= light_len;
                        light_dir_y /= light_len;
                        light_dir_z /= light_len;

                        for (const auto& face_tuple : depth_sorted_faces) {
                            int face_idx = std::get<1>(face_tuple);
                            const auto& face = parsed_model.faces[face_idx];

                            ImVec2 screen_points[3];
                            float transformed_vertices[3][3];
                            float uv_coords[3][2];
                            bool has_uvs = false;

                            for (int j = 0; j < 3; j++) {
                                const auto& v = parsed_model.vertices[face.vertex_indices[j]];
                                float x = v.x - aabb_center_x;
                                float y = v.y - aabb_center_y;
                                float z = v.z - aabb_center_z;

                                float rotated_x = x * cos_rot - z * sin_rot;
                                float rotated_z = x * sin_rot + z * cos_rot;
                                float final_y = y * cos_pitch - rotated_z * sin_pitch;
                                float final_z = y * sin_pitch + rotated_z * cos_pitch;

                                transformed_vertices[j][0] = rotated_x;
                                transformed_vertices[j][1] = final_y;
                                transformed_vertices[j][2] = final_z;

                                screen_points[j] = ImVec2(canvas_center.x + rotated_x * scale, canvas_center.y - final_y * scale);
                                
                                preview_min_x = min(preview_min_x, screen_points[j].x);
                                preview_min_y = min(preview_min_y, screen_points[j].y);
                                preview_max_x = max(preview_max_x, screen_points[j].x);
                                preview_max_y = max(preview_max_y, screen_points[j].y);

                                if (face.texcoord_indices.size() >= 3) {
                                    int uv_idx = face.texcoord_indices[j];
                                    if (uv_idx >= 0 && uv_idx < static_cast<int>(parsed_model.tex_coords.size())) {
                                        uv_coords[j][0] = parsed_model.tex_coords[uv_idx].u;
                                        uv_coords[j][1] = parsed_model.tex_coords[uv_idx].v;
                                        has_uvs = true;
                                    }
                                    else {
                                        has_uvs = false;
                                        break;
                                    }
                                }
                            }

                            float v1x = transformed_vertices[1][0] - transformed_vertices[0][0];
                            float v1y = transformed_vertices[1][1] - transformed_vertices[0][1];
                            float v1z = transformed_vertices[1][2] - transformed_vertices[0][2];

                            float v2x = transformed_vertices[2][0] - transformed_vertices[0][0];
                            float v2y = transformed_vertices[2][1] - transformed_vertices[0][1];
                            float v2z = transformed_vertices[2][2] - transformed_vertices[0][2];

                            float face_normal_x = v1y * v2z - v1z * v2y;
                            float face_normal_y = v1z * v2x - v1x * v2z;
                            float face_normal_z = v1x * v2y - v1y * v2x;

                            float nlen = std::sqrt(face_normal_x * face_normal_x + face_normal_y * face_normal_y + face_normal_z * face_normal_z);
                            if (nlen > 0.0001f) {
                                face_normal_x /= nlen;
                                face_normal_y /= nlen;
                                face_normal_z /= nlen;
                            }

                            float dotProduct = face_normal_x * light_dir_x + face_normal_y * light_dir_y + face_normal_z * light_dir_z;
                            dotProduct = max(0.0f, dotProduct);
                            float lighting = ambient + (diffuse * dotProduct);
                            lighting = min(1.0f, max(0.4f, lighting));

                            float r = 0.5f, g = 0.5f, b = 0.5f;
                            c_decoded_texture* texture = nullptr;

                            if (!face.material_name.empty()) {
                                auto mat_it = parsed_model.materials.find(face.material_name);
                                if (mat_it != parsed_model.materials.end()) {
                                    const c_obj_material& material = mat_it->second;
                                    r = material.diffuse[0];
                                    g = material.diffuse[1];
                                    b = material.diffuse[2];

                                    if (material.texture_index >= 0 && has_uvs) {
                                        texture = c_texture_cache::get().get_texture(local_user_id, material.texture_index);
                                        if (!texture || !texture->ready.load(std::memory_order_acquire)) {
                                            texture = nullptr;
                                        }
                                    }
                                }
                            }

                            if (texture && has_uvs) {
                                ImVec2 min_pt = screen_points[0];
                                ImVec2 max_pt = screen_points[0];

                                for (int j = 1; j < 3; j++) {
                                    min_pt.x = min(min_pt.x, screen_points[j].x);
                                    min_pt.y = min(min_pt.y, screen_points[j].y);
                                    max_pt.x = max(max_pt.x, screen_points[j].x);
                                    max_pt.y = max(max_pt.y, screen_points[j].y);
                                }

                                int min_x = max(0, static_cast<int>(std::floor(min_pt.x)));
                                int min_y = max(0, static_cast<int>(std::floor(min_pt.y)));
                                int max_x = min(static_cast<int>(canvas_min.x + preview_width), static_cast<int>(std::ceil(max_pt.x)));
                                int max_y = min(static_cast<int>(canvas_min.y + available_height), static_cast<int>(std::ceil(max_pt.y)));

                                float denom = (screen_points[1].y - screen_points[2].y) * (screen_points[0].x - screen_points[2].x) +
                                    (screen_points[2].x - screen_points[1].x) * (screen_points[0].y - screen_points[2].y);

                                if (std::abs(denom) > 0.0001f) {
                                    for (int py = min_y; py <= max_y; py++) {
                                        for (int px = min_x; px <= max_x; px++) {
                                            float w0 = ((screen_points[1].y - screen_points[2].y) * (px - screen_points[2].x) +
                                                (screen_points[2].x - screen_points[1].x) * (py - screen_points[2].y)) / denom;
                                            float w1 = ((screen_points[2].y - screen_points[0].y) * (px - screen_points[2].x) +
                                                (screen_points[0].x - screen_points[2].x) * (py - screen_points[2].y)) / denom;
                                            float w2 = 1.0f - w0 - w1;

                                            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                                                float u = w0 * uv_coords[0][0] + w1 * uv_coords[1][0] + w2 * uv_coords[2][0];
                                                float v = w0 * uv_coords[0][1] + w1 * uv_coords[1][1] + w2 * uv_coords[2][1];

                                                float texR, texG, texB, texA;
                                                texture->sample(u, v, texR, texG, texB, texA);

                                                float final_r = min(1.0f, texR * lighting);
                                                float final_g = min(1.0f, texG * lighting);
                                                float final_b = min(1.0f, texB * lighting);

                                                ImU32 pixel_color = IM_COL32(static_cast<int>(final_r * 255),
                                                    static_cast<int>(final_g * 255),
                                                    static_cast<int>(final_b * 255), 255);

                                                preview_draw->AddRectFilled(ImVec2(static_cast<float>(px), static_cast<float>(py)),
                                                    ImVec2(static_cast<float>(px + 1), static_cast<float>(py + 1)), pixel_color);
                                            }
                                        }
                                    }
                                }
                            }
                            else {
                                r = min(1.0f, r * lighting);
                                g = min(1.0f, g * lighting);
                                b = min(1.0f, b * lighting);

                                ImU32 fillColor = IM_COL32(static_cast<int>(r * 255),
                                    static_cast<int>(g * 255),
                                    static_cast<int>(b * 255), 255);

                                preview_draw->AddTriangleFilled(screen_points[0], screen_points[1], screen_points[2], fillColor);
                            }
                        }
                    }

                    if (preview_min_x < preview_max_x && preview_min_y < preview_max_y)
                    {
                        float left = preview_min_x;
                        float top = preview_min_y;
                        float right = preview_max_x;
                        float bottom = preview_max_y;

                        ImVec2 c1(left, top);
                        ImVec2 c2(right - left, bottom - top);

                        ImDrawList* draw = preview_draw;

                        if (settings::visuals::box_fill)
                        {
                            ImU32 fill_col = ImGui::ColorConvertFloat4ToU32({
                                settings::visuals::box_fill_color[0],
                                settings::visuals::box_fill_color[1],
                                settings::visuals::box_fill_color[2],
                                settings::visuals::box_fill_color[3]
                            });

                            ImRect fill_rect(c1.x, c1.y, c1.x + c2.x, c1.y + c2.y);
                            draw->AddRectFilled(fill_rect.Min, fill_rect.Max, fill_col);
                        }

                        if (settings::visuals::box)
                        {
                            ImU32 box_col = ImGui::ColorConvertFloat4ToU32(
                                {
                                    settings::visuals::box_color[0],
                                    settings::visuals::box_color[1],
                                    settings::visuals::box_color[2],
                                    settings::visuals::box_color[3]
                                }
                            );

                            if (settings::visuals::box_type == 0)
                            {
                                c1.x = std::round(c1.x);
                                c1.y = std::round(c1.y);
                                c2.x = std::round(c2.x);
                                c2.y = std::round(c2.y);

                                ImDrawListFlags old_flags = draw->Flags;
                                draw->Flags &= ~ImDrawListFlags_AntiAliasedFill;
                                draw->Flags |= ImDrawListFlags_AntiAliasedLines;

                                ImRect rect(c1.x, c1.y, c1.x + c2.x, c1.y + c2.y);

                                draw->AddRect(rect.Min, rect.Max, IM_COL32(0, 0, 0, box_col >> 24));
                                draw->AddRect({ rect.Min.x - 1.f, rect.Min.y - 1.f }, { rect.Max.x + 1.f, rect.Max.y + 1.f }, box_col);
                                draw->AddRect({ rect.Min.x - 2.f, rect.Min.y - 2.f }, { rect.Max.x + 2.f, rect.Max.y + 2.f }, IM_COL32(0, 0, 0, box_col >> 24));
                                
                                draw->Flags = old_flags;
                            }
                            else
                            {
                                ImRect rect_bb(c1.x, c1.y, c1.x + c2.x, c1.y + c2.y);
                                helper::corner_box(draw, rect_bb.Min, rect_bb.Max, box_col, 1.f);
                            }
                        }
                        float box_center_x = left + (right - left) * 0.5f;
                        float box_top = top;
                        float box_bottom = bottom;
                        float box_left = std::round(left);
                        float box_right = std::round(right);
                        float box_width = box_right - box_left;
                        float box_height = box_bottom - box_top;
                        
                        float actual_box_left = box_left;
                        float actual_box_right = box_right;
                        if (settings::visuals::box && settings::visuals::box_type == 0)
                        {
                            actual_box_left = c1.x;
                            actual_box_right = c1.x + c2.x;
                        }

                        ImFont* font = esp_font ? esp_font : ImGui::GetFont();
                        float font_size = esp_font ? font_config::visitor::font_size : ImGui::GetFontSize();

                        if (settings::visuals::healthbar)
                        {
                            static float health_animation_time = 0.0f;
                            health_animation_time += ImGui::GetIO().DeltaTime * 0.5f;
                            if (health_animation_time > 1.0f) health_animation_time -= 1.0f;
                            
                            float t = (std::sin(health_animation_time * 3.14159f * 2.0f) + 1.0f) * 0.5f;
                            float min_health = 0.2f;
                            float max_health = 1.0f;
                            float health_percent = min_health + (max_health - min_health) * t;
                            
                            if (health_percent < 0.f) health_percent = 0.f;
                            if (health_percent > 1.f) health_percent = 1.f;

                            float health_percent_actual = health_percent * 100.f;
                            bool use_health_based = settings::visuals::health_based_healthbar;
                            bool use_gradient = settings::visuals::gradient_healthbar;

                            float color_r, color_g, color_b;
                            if (use_health_based)
                            {
                                if (health_percent_actual > 60.f)
                                {
                                    color_r = 0.f;
                                    color_g = 1.f;
                                    color_b = 0.f;
                                }
                                else if (health_percent_actual >= 50.f)
                                {
                                    color_r = 1.f;
                                    color_g = 0.647f;
                                    color_b = 0.f;
                                }
                                else if (health_percent_actual <= 20.f)
                                {
                                    color_r = 1.f;
                                    color_g = 0.f;
                                    color_b = 0.f;
                                }
                                else
                                {
                                    color_r = 1.f;
                                    color_g = 0.647f;
                                    color_b = 0.f;
                                }
                            }
                            else
                            {
                                color_r = settings::visuals::healthbar_color[0];
                                color_g = settings::visuals::healthbar_color[1];
                                color_b = settings::visuals::healthbar_color[2];
                            }

                            float transparency = 255.f;
                            float health_bar_height = box_height + 2.f;
                            draw->AddRectFilled(ImVec2(box_left - 7.f, box_top - 2.f), ImVec2(box_left - 3.f, box_bottom + 2.f), IM_COL32(0, 0, 0, 255));
                            if (health_percent > 0.f)
                            {
                                float health_fill_height = health_bar_height * health_percent;
                                ImVec2 fill_min = ImVec2(box_left - 6.f, box_bottom + 1.f - health_fill_height);
                                ImVec2 fill_max = ImVec2(box_left - 4.f, box_bottom + 1.f);

                                draw->AddRectFilled(fill_min, fill_max, IM_COL32(color_r * 255.f, color_g * 255.f, color_b * 255.f, transparency));
                            }
                        }

                        if (settings::visuals::armorbar)
                        {
                            static float armor_animation_time = 0.0f;
                            armor_animation_time += ImGui::GetIO().DeltaTime * 0.4f;
                            if (armor_animation_time > 1.0f) armor_animation_time -= 1.0f;
                            
                            float t = (std::sin(armor_animation_time * 3.14159f * 2.0f) + 1.0f) * 0.5f;
                            float min_armor = 0.0f;
                            float max_armor = 1.0f;
                            float armor_percent = min_armor + (max_armor - min_armor) * t;
                            
                            if (armor_percent < 0.f) armor_percent = 0.f;
                            if (armor_percent > 1.f) armor_percent = 1.f;

                            float bar_height = 2.0f;
                            float bar_y = std::round(box_bottom) + 5.0f;
                            float bar_left = actual_box_left - 1.0f;
                            float bar_right = actual_box_right + 1.0f;
                            float bar_width = bar_right - bar_left;
                            float fill_width = bar_width * armor_percent;

                            ImDrawListFlags old_flags = draw->Flags;
                            draw->Flags &= ~ImDrawListFlags_AntiAliasedFill;
                            draw->Flags |= ImDrawListFlags_AntiAliasedLines;

                            draw->AddRectFilled(ImVec2(std::round(bar_left - 1.0f), std::round(bar_y - 1.0f)), ImVec2(std::round(bar_right + 1.0f), std::round(bar_y + bar_height + 1.0f)), IM_COL32(0, 0, 0, 255));
                            draw->AddRectFilled(ImVec2(std::round(bar_left), std::round(bar_y)), ImVec2(std::round(bar_right), std::round(bar_y + bar_height)), IM_COL32(45, 45, 45, 220));

                            ImU32 armor_col = ImGui::ColorConvertFloat4ToU32({
                                settings::visuals::armorbar_color[0],
                                settings::visuals::armorbar_color[1],
                                settings::visuals::armorbar_color[2],
                                settings::visuals::armorbar_color[3]
                            });

                            if (fill_width > 0.0f)
                            {
                                draw->AddRectFilled(ImVec2(std::round(bar_left), std::round(bar_y)), ImVec2(std::round(bar_left + fill_width), std::round(bar_y + bar_height)), armor_col);
                            }

                            draw->Flags = old_flags;
                        }

                        if (settings::visuals::name)
                        {
                            std::string name_to_display = "DisplayName";
                            if (local_player_address != 0) {
                                rbx::player_t local_player(local_player_address);
                                if (settings::visuals::name_display_type == 0) {
                                    name_to_display = local_player.get_display_name();
                                    if (name_to_display.empty()) {
                                        name_to_display = local_player.get_name();
                                    }
                                }
                                else {
                                    name_to_display = local_player.get_name();
                                }
                            }
                            float text_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.f, name_to_display.c_str()).x;

                            ImU32 name_col = ImGui::ColorConvertFloat4ToU32(
                                {
                                    settings::visuals::name_color[0],
                                    settings::visuals::name_color[1],
                                    settings::visuals::name_color[2],
                                    settings::visuals::name_color[3]
                                }
                            );

                            helper::draw_text_outlined(draw, font, font_size,
                                ImVec2(box_center_x - text_width * 0.5f, box_top - font_size - 5.f),
                                name_col, name_to_display.c_str());
                        }

                        if (settings::visuals::distance)
                        {
                            static float distance_animation_time = 0.0f;
                            distance_animation_time += ImGui::GetIO().DeltaTime * 0.5f;
                            if (distance_animation_time > 1.0f) distance_animation_time -= 1.0f;
                            
                            float t = (std::sin(distance_animation_time * 3.14159f * 2.0f) + 1.0f) * 0.5f;
                            float min_distance = 50.0f;
                            float max_distance = 100.0f;
                            float animated_distance = min_distance + (max_distance - min_distance) * t;
                            
                            char distance_str[32];
                            if (settings::visuals::distance_measurement == 1)
                            {
                                float distance_meters = animated_distance * 0.28f;
                                std::snprintf(distance_str, sizeof(distance_str), "%.1fm", distance_meters);
                            }
                            else
                            {
                                std::snprintf(distance_str, sizeof(distance_str), "%.1f", animated_distance);
                            }

                            float text_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.f, distance_str).x;

                            ImU32 distance_col = ImGui::ColorConvertFloat4ToU32(
                                {
                                    settings::visuals::distance_color[0],
                                    settings::visuals::distance_color[1],
                                    settings::visuals::distance_color[2],
                                    settings::visuals::distance_color[3]
                                }
                            );

                            float distance_y = box_bottom + 5.f;
                            if (settings::visuals::armorbar)
                            {
                                distance_y = std::round(box_bottom) + 5.0f + 2.0f + 3.0f;
                            }

                            helper::draw_text_outlined(draw, font, font_size,
                                ImVec2(box_center_x - text_width * 0.5f, distance_y),
                                distance_col, distance_str);
                        }

                        if (settings::visuals::tool)
                        {
                            std::string tool_name = "Sword";
                            float text_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.f, tool_name.c_str()).x;
                            float tool_y = box_bottom + 5.f;
                            
                            if (settings::visuals::armorbar)
                            {
                                tool_y = std::round(box_bottom) + 5.0f + 2.0f + 3.0f;
                            }
                            
                            if (settings::visuals::distance)
                            {
                                tool_y += font_size + 2.f;
                            }

                            ImU32 tool_col = ImGui::ColorConvertFloat4ToU32(
                                {
                                    settings::visuals::tool_color[0],
                                    settings::visuals::tool_color[1],
                                    settings::visuals::tool_color[2],
                                    settings::visuals::tool_color[3]
                                }
                            );

                            helper::draw_text_outlined(draw, font, font_size,
                                ImVec2(box_center_x - text_width * 0.5f, tool_y),
                                tool_col, tool_name.c_str());
                        }

                        if (settings::visuals::flags)
                        {
                            float current_y = box_top;
                            
                            static float state_animation_time = 0.0f;
                            state_animation_time += ImGui::GetIO().DeltaTime * 0.3f;
                            if (state_animation_time > 2.0f) state_animation_time -= 2.0f;
                            
                            const char* state = "Running";
                            if (state_animation_time < 1.0f)
                            {
                                state = "Running";
                            }
                            else
                            {
                                state = "Idle";
                            }
                            
                            char state_buffer[64];
                            std::snprintf(state_buffer, sizeof(state_buffer), "[%s]", state);
                            
                            ImU32 state_col = ImGui::ColorConvertFloat4ToU32(
                                {
                                    settings::visuals::flags_state_colour[0],
                                    settings::visuals::flags_state_colour[1],
                                    settings::visuals::flags_state_colour[2],
                                    settings::visuals::flags_state_colour[3]
                                }
                            );
                            
                            float text_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.f, state_buffer).x;
                            float flag_x = box_right + 5.f;
                            
                            helper::draw_text_outlined(draw, font, font_size, ImVec2(flag_x, current_y), state_col, state_buffer);
                        }
                    }
                }
            }
            ImGui::EndChild();
        }
    }
    ImGui::End();

}

void Menu::Shutdown()
{
    if (!m_bInitialized) return;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Menu::InvalidateDeviceObjects()
{
    if (!m_bInitialized) return;

    ImGui_ImplDX11_InvalidateDeviceObjects();
}

void Menu::CreateDeviceObjects()
{
    if (!m_bInitialized) return;

    ImGui_ImplDX11_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool Menu::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_bInitialized) return false;

#if 0
    if (uMsg == WM_KEYDOWN && wParam == VK_DELETE)
    {
        // Toggle menu

        return true; // important, incase we trying to close the menu while selecting an input, delete wont cause the content to be erased
    }
#endif

    return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}
