#include "stdio.h"

#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

struct Renderer
{
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
};

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
}

const char* WINDOW_CLASS = "rtow_window_class";

struct Window
{
	int width, height;
	const char* title;
	HWND handle;
	HDC hdc;
};

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

void register_window_class(const char* window_class)
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

Window create_window(int width, int height, const char* title)
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

void close_window(Window& self)
{
	ReleaseDC(self.handle, self.hdc);
	DestroyWindow(self.handle);
}

int main(int argc, char** argv)
{
	auto renderer = renderer_new();

	register_window_class(WINDOW_CLASS);
	auto window = create_window(800, 600, "RTOW");

	MSG msg{};
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, window.handle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	close_window(window);
	renderer_free(renderer);
	return 0;
}
