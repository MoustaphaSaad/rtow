#include "stdio.h"

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

LRESULT CALLBACK
_window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
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
		exit(-1);
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

	IDXGIFactory* factory = nullptr;
	IDXGIAdapter* adapter = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;

	auto res = CreateDXGIFactory(IID_PPV_ARGS(&factory));
	if (FAILED(res))
	{
		fprintf(stderr, "failed to create DXGIFactory");
		return EXIT_FAILURE;
	}

	const D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
	};

	UINT creation_flags = 0;
	#if defined(_DEBUG)
		creation_flags = D3D11_CREATE_DEVICE_DEBUG;
	#endif

	res = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creation_flags,
		feature_levels,
		2,
		D3D11_SDK_VERSION,
		&device,
		nullptr,
		&context
	);
	if (FAILED(res))
	{
		fprintf(stderr, "failed to create device");
		return EXIT_FAILURE;
	}

	context->Release();
	device->Release();
	adapter->Release();
	factory->Release();

	printf("Hello, World!\n");
	return 0;
}
