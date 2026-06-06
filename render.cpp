#include "render.hpp"
#include <vector>
#include <objbase.h>
#include <wincodec.h>
#include <dwmapi.h>

ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
UINT g_ResizeWidth = 0;
UINT g_ResizeHeight = 0;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
HWND g_overlay_hwnd = nullptr;
IDXGISwapChain* g_pOverlaySwapChain = nullptr;
ID3D11RenderTargetView* g_overlayRTV = nullptr;
ImGuiContext* g_overlay_ctx = nullptr;

bool CreateTextureFromPNGInMemory(ID3D11Device* device, const unsigned char* data, size_t size, ID3D11ShaderResourceView** out_srv) {
    if (!device || !data || !size || !out_srv) return false;
    *out_srv = nullptr;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)size);
    if (!hMem) return false;
    void* pMem = GlobalLock(hMem);
    if (!pMem) { GlobalFree(hMem); return false; }
    memcpy(pMem, data, size);
    GlobalUnlock(hMem);
    IStream* pStream = nullptr;
    if (CreateStreamOnHGlobal(hMem, TRUE, &pStream) != S_OK) { GlobalFree(hMem); return false; }
    IWICImagingFactory* pFactory = nullptr;
    if (CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory)) != S_OK) { pStream->Release(); return false; }
    IWICBitmapDecoder* pDecoder = nullptr;
    if (pFactory->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pDecoder) != S_OK) { pFactory->Release(); pStream->Release(); return false; }
    IWICBitmapFrameDecode* pFrame = nullptr;
    if (pDecoder->GetFrame(0, &pFrame) != S_OK) { pDecoder->Release(); pFactory->Release(); pStream->Release(); return false; }
    UINT width = 0, height = 0;
    if (pFrame->GetSize(&width, &height) != S_OK || !width || !height) { pFrame->Release(); pDecoder->Release(); pFactory->Release(); pStream->Release(); return false; }
    IWICFormatConverter* pConverter = nullptr;
    if (pFactory->CreateFormatConverter(&pConverter) != S_OK) { pFrame->Release(); pDecoder->Release(); pFactory->Release(); pStream->Release(); return false; }
    if (pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA, (WICBitmapDitherType)0, nullptr, 0.f, WICBitmapPaletteTypeCustom) != S_OK) { pConverter->Release(); pFrame->Release(); pDecoder->Release(); pFactory->Release(); pStream->Release(); return false; }
    UINT stride = width * 4;
    UINT buf_size = stride * height;
    std::vector<BYTE> pixels(buf_size);
    if (pConverter->CopyPixels(nullptr, stride, buf_size, pixels.data()) != S_OK) { pConverter->Release(); pFrame->Release(); pDecoder->Release(); pFactory->Release(); pStream->Release(); return false; }
    pConverter->Release();
    pFrame->Release();
    pDecoder->Release();
    pFactory->Release();
    pStream->Release();
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA init = {};
    init.pSysMem = pixels.data();
    init.SysMemPitch = stride;
    ID3D11Texture2D* pTex = nullptr;
    if (device->CreateTexture2D(&td, &init, &pTex) != S_OK) return false;
    D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    sd.Texture2D.MipLevels = 1;
    HRESULT hr = device->CreateShaderResourceView(pTex, &sd, out_srv);
    pTex->Release();
    return hr == S_OK;
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK) return false;
    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

bool CreateOverlay(int screen_w, int screen_h, ImFontAtlas* shared_fonts, LPCWSTR class_name, HINSTANCE h_instance) {
    g_overlay_hwnd = ::CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, class_name, L"", WS_POPUP, 0, 0, screen_w, screen_h, nullptr, nullptr, h_instance, nullptr);
    if (!g_overlay_hwnd) return false;
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(g_overlay_hwnd, &margins);
    ::SetLayeredWindowAttributes(g_overlay_hwnd, 0, 255, LWA_ALPHA);
    IDXGIDevice* pDXGIDevice = nullptr;
    if (g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) != S_OK) return false;
    IDXGIAdapter* pAdapter = nullptr;
    if (pDXGIDevice->GetAdapter(&pAdapter) != S_OK) { pDXGIDevice->Release(); return false; }
    IDXGIFactory* pFactory = nullptr;
    if (pAdapter->GetParent(IID_PPV_ARGS(&pFactory)) != S_OK) { pAdapter->Release(); pDXGIDevice->Release(); return false; }
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = screen_w;
    sd.BufferDesc.Height = screen_h;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_overlay_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    HRESULT hr = pFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pOverlaySwapChain);
    pFactory->Release();
    pAdapter->Release();
    pDXGIDevice->Release();
    if (hr != S_OK || !g_pOverlaySwapChain) return false;
    ID3D11Texture2D* pOverlayBackBuffer = nullptr;
    if (g_pOverlaySwapChain->GetBuffer(0, IID_PPV_ARGS(&pOverlayBackBuffer)) != S_OK) return false;
    hr = g_pd3dDevice->CreateRenderTargetView(pOverlayBackBuffer, nullptr, &g_overlayRTV);
    pOverlayBackBuffer->Release();
    if (hr != S_OK || !g_overlayRTV) return false;
    ImGuiContext* prev_ctx = ImGui::GetCurrentContext();
    g_overlay_ctx = ImGui::CreateContext(shared_fonts);
    if (!g_overlay_ctx) return false;
    ImGui::SetCurrentContext(g_overlay_ctx);
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().LogFilename = nullptr;
    ImGui::SetCurrentContext(prev_ctx);
    ::ShowWindow(g_overlay_hwnd, SW_HIDE);
    return true;
}

void CleanupOverlay() {
    if (g_overlayRTV) { g_overlayRTV->Release(); g_overlayRTV = nullptr; }
    if (g_pOverlaySwapChain) { g_pOverlaySwapChain->Release(); g_pOverlaySwapChain = nullptr; }
    if (g_overlay_hwnd) { ::DestroyWindow(g_overlay_hwnd); g_overlay_hwnd = nullptr; }
}
