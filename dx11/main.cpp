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
	ID3D11RenderTargetView* render_target_view;
	ID3D11Buffer* screen_rect_vertices;
	ID3D10Blob* compiled_vs_shader;
	ID3D11VertexShader* screen_rect_vertex_shader;
	ID3D11PixelShader* screen_rect_pixel_shader;
	ID3D11RasterizerState* screen_rect_rasterizer_state;
	ID3D11InputLayout* screen_rect_input_layout;
};

void renderer_draw(Renderer& self)
{
	self.context->OMSetRenderTargets(1, &self.render_target_view, nullptr);
	D3D11_VIEWPORT viewport{};
	viewport.Width = 800;
	viewport.Height = 600;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	self.context->RSSetViewports(1, &viewport);

	self.context->RSSetState(self.screen_rect_rasterizer_state);
	self.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	self.context->VSSetShader(self.screen_rect_vertex_shader, NULL, 0);
	self.context->PSSetShader(self.screen_rect_pixel_shader, NULL, 0);
	self.context->IASetInputLayout(self.screen_rect_input_layout);

	UINT offset = 0;
	UINT stride = 2 * sizeof(float);
	self.context->IASetVertexBuffers(0, 1, &self.screen_rect_vertices, &stride, &offset);

	self.context->Draw(6, 0);
	self.swapchain->Present(0, 0);
}

void renderer_setup_resources(Renderer& self, Window& window)
{
	static float RECT_VERTICES[] = {
		-1.0, -1.0,
		 1.0,  1.0,
		-1.0,  1.0,

		-1.0, -1.0,
		 1.0, -1.0,
		 1.0,  1.0
	};

	static const char* RECT_VERTEX_SHADER = R"SHADER(
		struct VS_Input
		{
			float2 pos: POSITION;
		};

		struct VS_Output
		{
			float4 pos: SV_POSITION;
			float2 uv: TEXCOORD0;
		};

		VS_Output main(VS_Input input)
		{
			VS_Output output;
			output.pos = float4(input.pos, 0, 1);
			output.uv = input.pos + float2(0.5, 0.5);
			return output;
		}
	)SHADER";

	static const char* RECT_PIXEL_SHADER = R"SHADER(
		struct PS_Input
		{
			float4 pos: SV_POSITION;
			float2 uv: TEXCOORD0;
		};

		float4 main(PS_Input input): SV_TARGET
		{
			return float4(input.uv, 0.25, 1);
		}
	)SHADER";

	// setup screen rect vertex buffer;
	{
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = sizeof(RECT_VERTICES);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA data_desc{};
		data_desc.pSysMem = RECT_VERTICES;

		auto res = self.device->CreateBuffer(&desc, &data_desc, &self.screen_rect_vertices);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create buffer");
			exit(EXIT_FAILURE);
		}
	}

	// vertex shader compilation
	{
		ID3D10Blob* error = nullptr;
		UINT compile_flags = 0;
		#if defined(_DEBUG)
			compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		#endif

		auto res = D3DCompile(RECT_VERTEX_SHADER, strlen(RECT_VERTEX_SHADER), NULL, NULL, NULL, "main", "vs_5_0", compile_flags, 0, &self.compiled_vs_shader, &error);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to compile shader, %s\n", (char*)error->GetBufferPointer());
			exit(EXIT_FAILURE);
		}

		res = self.device->CreateVertexShader(
			self.compiled_vs_shader->GetBufferPointer(),
			self.compiled_vs_shader->GetBufferSize(),
			NULL,
			&self.screen_rect_vertex_shader
		);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create vertex shader\n");
			exit(EXIT_FAILURE);
		}
	}

	// pixel shader compilation
	{
		ID3D10Blob* error = nullptr;
		UINT compile_flags = 0;
		#if defined(_DEBUG)
			compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		#endif

		ID3D10Blob* shader_blob;
		auto res = D3DCompile(RECT_PIXEL_SHADER, strlen(RECT_PIXEL_SHADER), NULL, NULL, NULL, "main", "ps_5_0", compile_flags, 0, &shader_blob, &error);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to compile shader, %s\n", (char*)error->GetBufferPointer());
			exit(EXIT_FAILURE);
		}

		res = self.device->CreatePixelShader(
			shader_blob->GetBufferPointer(),
			shader_blob->GetBufferSize(),
			NULL,
			&self.screen_rect_pixel_shader
		);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create pixel shader\n");
			exit(EXIT_FAILURE);
		}

		shader_blob->Release();
	}

	// create input layout
	{
		D3D11_INPUT_ELEMENT_DESC input_layout_desc[1];
		::memset(input_layout_desc, 0, sizeof(input_layout_desc));
		for (size_t i = 0; i < ARRAYSIZE(input_layout_desc); ++i)
		{
			auto& attribute = input_layout_desc[i];
			attribute.SemanticName = "POSITION";
			attribute.SemanticIndex = 0;
			attribute.Format = DXGI_FORMAT_R32G32_FLOAT;
			attribute.InputSlot = i;
		}
		auto res = self.device->CreateInputLayout(input_layout_desc, ARRAYSIZE(input_layout_desc), self.compiled_vs_shader->GetBufferPointer(), self.compiled_vs_shader->GetBufferSize(), &self.screen_rect_input_layout);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create input layout");
			exit(EXIT_FAILURE);
		}
	}

	// create rasterizer state
	{
		D3D11_RASTERIZER_DESC raster_desc{};
		raster_desc.CullMode = D3D11_CULL_NONE;
		raster_desc.FillMode = D3D11_FILL_SOLID;
		raster_desc.FrontCounterClockwise = true;
		auto res = self.device->CreateRasterizerState(&raster_desc, &self.screen_rect_rasterizer_state);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create rasterizer state");
			exit(EXIT_FAILURE);
		}
	}

	// create swapchain
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

		res = self.factory->CreateSwapChain(self.device, &desc, &self.swapchain);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create swapchain");
			exit(EXIT_FAILURE);
		}

		ID3D11Texture2D* color_buffer = nullptr;
		res = self.swapchain->GetBuffer(0, IID_PPV_ARGS(&color_buffer));
		if (FAILED(res))
		{
			fprintf(stderr, "failed to get swapchain buffer");
			exit(EXIT_FAILURE);
		}

		res = self.device->CreateRenderTargetView(color_buffer, nullptr, &self.render_target_view);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to get render target view");
			exit(EXIT_FAILURE);
		}
	}
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
	if (self.swapchain) self.swapchain->Release();
	if (self.render_target_view) self.render_target_view->Release();
	if (self.screen_rect_vertices) self.screen_rect_vertices->Release();
	if (self.compiled_vs_shader) self.compiled_vs_shader->Release();
	if (self.screen_rect_vertex_shader) self.screen_rect_vertex_shader->Release();
	if (self.screen_rect_pixel_shader) self.screen_rect_pixel_shader->Release();
	if (self.screen_rect_rasterizer_state) self.screen_rect_rasterizer_state->Release();
	if (self.screen_rect_input_layout) self.screen_rect_input_layout->Release();
	self.context->Release();
	self.device->Release();
	self.adapter->Release();
	self.factory->Release();
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

	renderer_setup_resources(renderer, window);

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
			renderer_draw(renderer);
		}
	}

	window_free(window);
	renderer_free(renderer);
	return 0;
}
