#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "source/imgui/imgui.h"

extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern UINT g_ResizeWidth;
extern UINT g_ResizeHeight;
extern ID3D11RenderTargetView* g_mainRenderTargetView;
extern HWND g_overlay_hwnd;
extern IDXGISwapChain* g_pOverlaySwapChain;
extern ID3D11RenderTargetView* g_overlayRTV;
extern ImGuiContext* g_overlay_ctx;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
bool CreateOverlay(int screen_w, int screen_h, ImFontAtlas* shared_fonts, LPCWSTR class_name, HINSTANCE h_instance);
void CleanupOverlay();
bool CreateTextureFromPNGInMemory(ID3D11Device* device, const unsigned char* data, size_t size, ID3D11ShaderResourceView** out_srv);
