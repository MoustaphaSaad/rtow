#include <stdio.h>
#include <stdlib.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

const char* WINDOW_CLASS = "rtow_window_class";

constexpr int WINDOW_DEFAULT_WIDTH = 1024;
constexpr int WINDOW_DEFAULT_HEIGHT = WINDOW_DEFAULT_WIDTH / (16.0 / 9.0);
constexpr bool USE_WARP_DEVICE = false;

void check_handle(HANDLE h, const char* msg)
{
	if (h == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		fprintf(stderr, "%s, Error: %d\n", msg, error);
		exit(error);
	}
}

void check_result(HRESULT h, const char* msg)
{
	if (FAILED(h))
	{
		DWORD error = GetLastError();
		fprintf(stderr, "%s, Error: %d\n", msg, error);
		exit(error);
	}
}

struct Window
{
	int width, height;
	const char* title;
	HWND handle;
	HDC hdc;
};

struct Renderer
{
	IDXGIFactory4* factory;
	ID3D12Device* device;
	ID3D12CommandQueue* command_queue;
	ID3D12CommandAllocator* command_allocator;
	UINT frame_index;
	HANDLE fence_event;
	ID3D12Fence* fence;
	UINT64 fence_value;
	IDXGISwapChain3* swapchain;
	bool ready;
};

Renderer renderer;

IDXGIAdapter1* get_hardware_adapter(IDXGIFactory1* factory, bool request_high_performance_adapter)
{
	IDXGIAdapter1* adapter = nullptr;
	IDXGIFactory6* factory6 = nullptr;
	if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			UINT adapter_index = 0;
			SUCCEEDED(factory6->EnumAdapterByGpuPreference(
				adapter_index,
				request_high_performance_adapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS(&adapter)));
			++adapter_index)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	if (adapter == nullptr)
	{
		for (UINT adapter_index = 0; SUCCEEDED(factory->EnumAdapters1(adapter_index, &adapter)); ++adapter_index)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	return adapter;
}

Renderer renderer_new()
{
	Renderer self{};

	UINT dxgiFactoryFlags = 0;

	#if defined(_DEBUG)
	{
		ID3D12Debug* dc;
		auto res = D3D12GetDebugInterface(IID_PPV_ARGS(&dc));
		check_result(res, "D3D12GetDebugInterface failed");

		ID3D12Debug1* debugController;
		res = dc->QueryInterface(IID_PPV_ARGS(&debugController));
		check_result(res, "QueryInterface failed");

		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(true);
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		debugController->Release();
	}
	#endif

	auto res = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&self.factory));
	check_result(res, "CreateDXGIFactory2 failed");

	if (USE_WARP_DEVICE)
	{
		IDXGIAdapter* warpAdapter;
		auto res = self.factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
		check_result(res, "EnumWarpAdapter failed");

		res = D3D12CreateDevice(warpAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&self.device));
		check_result(res, "D3D12CreateDevice failed");
	}
	else
	{
		IDXGIAdapter1* hardwareAdapter = get_hardware_adapter(self.factory, true);
		res = D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&self.device));
		check_result(res, "D3D12CreateDevice failed");
	}

	// Create Command Queue
	{
		D3D12_COMMAND_QUEUE_DESC queue_desc{};
		queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		auto res = self.device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&self.command_queue));
		check_result(res, "CreateCommandQueue failed");
	}

	// Create Command Allocator
	{
		auto res = self.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&self.command_allocator));
		check_result(res, "CreateCommandAllocator failed");
	}

	// Create Fence
	{
		auto res = self.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&self.fence));
		check_result(res, "CreateFence failed");
	}

	return self;
}

void renderer_free(Renderer& self)
{
	self.device->Release();
	self.factory->Release();
	self.command_queue->Release();
	self.command_allocator->Release();
	self.fence->Release();
	self.swapchain->Release();
}

void renderer_setup_resource(Renderer& self, Window& window)
{
	// Create swapchain
	{
		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.BufferCount = 2;
		desc.Width = window.width;
		desc.Height = window.height;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.SampleDesc.Count = 1;

		IDXGISwapChain1* swapchain;
		auto res = self.factory->CreateSwapChainForHwnd(
			self.command_queue,
			window.handle,
			&desc,
			nullptr,
			nullptr,
			&swapchain
		);
		check_result(res, "CreateSwapChainForHwnd failed");

		res = swapchain->QueryInterface(IID_PPV_ARGS(&self.swapchain));
		check_result(res, "QueryInterface failed");

		self.frame_index = self.swapchain->GetCurrentBackBufferIndex();
	}
	self.ready = true;
}

LRESULT CALLBACK _window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_SIZE:
		// renderer_draw(renderer);
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

void window_register_class(const char* window_class)
{
	WNDCLASSEX wc{};
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
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
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		NULL,
		NULL
	);
	check_handle(self.handle, "CreateWindowEx failed");

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
	renderer = renderer_new();

	window_register_class(WINDOW_CLASS);
	auto window = window_new(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, "Ray Tracing in One Weekend");

	MSG msg{};
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// renderer_draw(renderer);
		}
	}

	window_free(window);
	renderer_free(renderer);
	return 0;
}
