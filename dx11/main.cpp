#include "stdio.h"

#include <vector>

#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

const char* WINDOW_CLASS = "rtow_window_class";

struct Window
{
	int width, height;
	const char* title;
	HWND handle;
	HDC hdc;
};

struct Renderer
{
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapchain;
};

IDXGISwapChain* renderer_create_swapchain(Renderer& self, Window& window)
{
	DXGI_SWAP_CHAIN_DESC desc{};
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferCount = 1;
	desc.BufferDesc.Width = window.width;
	desc.BufferDesc.Height = window.height;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	IDXGIOutput* output = nullptr;
	auto res = self.adapter->EnumOutputs(0, &output);
	if (FAILED(res))
	{
		fprintf(stderr, "failed to enum output of DXGIAdapter");
		exit(EXIT_FAILURE);
	}

	UINT modes_count = 0;
	res = output->GetDisplayModeList(desc.BufferDesc.Format, DXGI_ENUM_MODES_INTERLACED, &modes_count, NULL);
	if (FAILED(res))
	{
		fprintf(stderr, "failed to enum available modes for this DXGIOutput");
		exit(EXIT_FAILURE);
	}

	std::vector<DXGI_MODE_DESC> modes{modes_count};
	res = output->GetDisplayModeList(desc.BufferDesc.Format, DXGI_ENUM_MODES_INTERLACED, &modes_count, modes.data());
	if (FAILED(res))
	{
		fprintf(stderr, "failed to enum available modes for this DXGIOutput");
		exit(EXIT_FAILURE);
	}
	output->Release();

	for (const auto& mode: modes)
	{
		if (mode.Width == desc.BufferDesc.Width && mode.Height == desc.BufferDesc.Height)
		{
			desc.BufferDesc.RefreshRate = mode.RefreshRate;
		}
	}

	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow = window.handle;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Windowed = TRUE;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	IDXGISwapChain* swapchain = nullptr;
	res = self.factory->CreateSwapChain(self.device, &desc, &swapchain);
	if (FAILED(res))
	{
		fprintf(stderr, "failed to create swapchain");
		exit(EXIT_FAILURE);
	}

	return swapchain;
}

Renderer renderer_new()
{
	Renderer self{};

	const D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
	};

	UINT creation_flags = 0;
	#if defined(_DEBUG)
		creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	auto res = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creation_flags,
		feature_levels,
		ARRAYSIZE(feature_levels),
		D3D11_SDK_VERSION,
		&self.device,
		nullptr,
		&self.context
	);
	if (FAILED(res))
	{
		fprintf(stderr, "failed to create device");
		exit(EXIT_FAILURE);
	}

	IDXGIDevice* dxgi_device = nullptr;
	res = self.device->QueryInterface(IID_PPV_ARGS(&dxgi_device));
	if (FAILED(res))
	{
		fprintf(stderr, "failed to get dxgi device from d3d11 device");
		exit(EXIT_FAILURE);
	}

	res = dxgi_device->GetAdapter(&self.adapter);
	if (FAILED(res))
	{
		fprintf(stderr, "failed to get adapter from dxgi device");
		exit(EXIT_FAILURE);
	}

	res = self.adapter->GetParent(IID_PPV_ARGS(&self.factory));
	if (FAILED(res))
	{
		fprintf(stderr, "failed to get DXGIFactory from adapter");
		exit(EXIT_FAILURE);
	}

	return self;
}

void renderer_free(Renderer& self)
{
	self.context->Release();
	self.device->Release();
	self.adapter->Release();
	self.factory->Release();
	if (self.swapchain) self.swapchain->Release();
}

LRESULT CALLBACK _window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

void window_register_class(const char* window_class)
{
	WNDCLASSEX wc{};
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = _window_callback;
	wc.hInstance = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = window_class;
	RegisterClassEx(&wc);
}

Window window_new(int width, int height, const char* title)
{
	Window self{};
	self.width = width;
	self.height = height;
	self.title = title;

	RECT wr{};
	wr.right = LONG(self.width);
	wr.bottom = LONG(self.height);

	self.handle = CreateWindowEx(
		NULL,
		WINDOW_CLASS,
		self.title,
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		NULL,
		NULL
	);
	if (self.handle == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "failed to create window");
		exit(EXIT_FAILURE);
	}

	self.hdc = GetDC(self.handle);
	ShowWindow(self.handle, SW_SHOW);
	SetForegroundWindow(self.handle);
	SetFocus(self.handle);

	return self;
}

void window_free(Window& self)
{
	ReleaseDC(self.handle, self.hdc);
	DestroyWindow(self.handle);
}

int main(int argc, char** argv)
{
	auto renderer = renderer_new();

	window_register_class(WINDOW_CLASS);
	auto window = window_new(800, 600, "RTOW");

	renderer.swapchain = renderer_create_swapchain(renderer, window);

	MSG msg{};
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	window_free(window);
	renderer_free(renderer);
	return 0;
}
