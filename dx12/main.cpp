#include <stdio.h>
#include <assert.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>

int main()
{
	// debug interface
	ID3D12Debug1* debugger = nullptr;
	UINT dxgi_factory_flags = 0;

	#if defined(_DEBUG)
	{
		ID3D12Debug* d = nullptr;
		auto res = D3D12GetDebugInterface(IID_PPV_ARGS(&d));
		assert(SUCCEEDED(res));

		res = d->QueryInterface(IID_PPV_ARGS(&debugger));
		assert(SUCCEEDED(res));

		debugger->EnableDebugLayer();
		debugger->SetEnableGPUBasedValidation(true);

		dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
		d->Release();
		d = nullptr;
	}
	#endif

	IDXGIFactory4* factory = nullptr;
	auto res = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory));
	assert(SUCCEEDED(res));

	IDXGIAdapter1* adapter = nullptr;
	for (UINT adapter_index = 0;
		factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND;
		++adapter_index)
	{
		DXGI_ADAPTER_DESC1 desc{};
		adapter->GetDesc1(&desc);

		// ignore software adapters
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapter->Release();
			continue;
		}

		// adapter should support dx12
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}

		adapter->Release();
	}

	ID3D12Device* device = nullptr;
	res = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
	assert(SUCCEEDED(res));

	ID3D12DebugDevice* debug_device = nullptr;
	#if defined(_DEBUG)
	{
		auto res = device->QueryInterface(&debug_device);
		assert(SUCCEEDED(res));
	}
	#endif

	ID3D12CommandQueue* command_queue = nullptr;
	D3D12_COMMAND_QUEUE_DESC queue_desc{};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	res = device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue));
	assert(SUCCEEDED(res));

	ID3D12CommandAllocator* command_allocator = nullptr;
	res = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator));
	assert(SUCCEEDED(res));

	ID3D12CommandList* command_list = nullptr;
	res = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, pipeline_state, IID_PPV_ARGS(&command_list));
	assert(SUCCEEDED(res));


	command_allocator->Release();
	command_queue->Release();
	device->Release();
	adapter->Release();
	factory->Release();
	debugger->Release();
	printf("Hello, World!\n");
	return 0;
}