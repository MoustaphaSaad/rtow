#include <stdio.h>
#include <assert.h>
#include <vector>
#include <fstream>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>

inline std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	bool exists = (bool)file;

	if (!exists || !file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
};


int main()
{
	auto cs_simple_bytecode = readFile("simple.so");
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

	D3D12_ROOT_PARAMETER parameters[1] = {};
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	parameters[0].Descriptor.ShaderRegister = 0;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
	root_signature_desc.pParameters = parameters;
	root_signature_desc.NumParameters = sizeof(parameters) / sizeof(*parameters);

	ID3D10Blob* root_signature_blob = nullptr;
	ID3D10Blob* root_signature_error = nullptr;
	res = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &root_signature_blob, &root_signature_error);
	assert(SUCCEEDED(res));
	// freeing error immediatly because we don't handle error at the moment
	if (root_signature_error) root_signature_error->Release();

	ID3D12RootSignature* root_signature = nullptr;
	res = device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&root_signature));
	assert(SUCCEEDED(res));

	ID3D12PipelineState* compute_pipeline = nullptr;
	D3D12_COMPUTE_PIPELINE_STATE_DESC compute_pipeline_desc{};
	compute_pipeline_desc.pRootSignature = root_signature;
	compute_pipeline_desc.CS.pShaderBytecode = cs_simple_bytecode.data();
	compute_pipeline_desc.CS.BytecodeLength = cs_simple_bytecode.size();
	res = device->CreateComputePipelineState(&compute_pipeline_desc, IID_PPV_ARGS(&compute_pipeline));
	assert(SUCCEEDED(res));

	ID3D12GraphicsCommandList* command_list = nullptr;
	res = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, compute_pipeline, IID_PPV_ARGS(&command_list));
	assert(SUCCEEDED(res));

	command_list->Dispatch(1, 1, 1);

	command_allocator->Release();
	compute_pipeline->Release();
	command_queue->Release();
	device->Release();
	adapter->Release();
	factory->Release();
	debugger->Release();
	printf("Hello, World!\n");
	return 0;
}