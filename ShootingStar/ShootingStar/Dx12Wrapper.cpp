#pragma comment(lib,"d3d12.lib") 
#pragma comment(lib,"dxgi.lib") 

#include "Dx12Wrapper.h"
#include "Application.h"

bool Dx12Wrapper::DeviceInit()
{
	//�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_1;
	HRESULT result = S_OK;
	for (auto lv : levels)
	{
		result = D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(_dev.GetAddressOf()));
		if (SUCCEEDED(result))
		{
			level = lv;
			break;
		}
	}
	if (_dev == nullptr)
	{
		return false;
	}
	return true;
}

bool Dx12Wrapper::CommandInit()
{
	//�R�}���h�L���[�̍쐬
	D3D12_COMMAND_QUEUE_DESC cmdQDesc = {};
	cmdQDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQDesc.NodeMask = 0;
	cmdQDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	auto result = _dev->CreateCommandQueue(&cmdQDesc, IID_PPV_ARGS(_cmdQue.GetAddressOf()));

	//�R�}���h�A���P�[�^�[�̍쐬
	_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_cmdAllocator.GetAddressOf()));
	//�R�}���h���X�g�̍쐬
	_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator.Get(), nullptr, IID_PPV_ARGS(_cmdList.GetAddressOf()));
	_cmdList->Close();

	return true;
}

bool Dx12Wrapper::SwapChainInit()
{
	//DXGI�̏�����
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(_dxgi.GetAddressOf()));

	auto wsize = Application::Instance().GetWindowSize();
	//�X���b�v�`�F�[���̐ݒ�
	DXGI_SWAP_CHAIN_DESC1 swDesc = {};
	swDesc.Width = wsize.w;
	swDesc.Height = wsize.h;
	swDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swDesc.Stereo = false;
	swDesc.SampleDesc.Count = 1;
	swDesc.SampleDesc.Quality = 0;
	swDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swDesc.BufferCount = 2;
	swDesc.Scaling = DXGI_SCALING_STRETCH;
	swDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swDesc.Flags = 0;

	//�X���b�v�`�F�[���̏�����
	result = _dxgi->CreateSwapChainForHwnd(_cmdQue.Get(), _hwnd, &swDesc,
		nullptr, nullptr, (IDXGISwapChain1**)(_swapchain.GetAddressOf()));

	return true;
}

bool Dx12Wrapper::RTInit()
{
	//�f�X�N���v�^�q�[�v�̐ݒ�(�����_�[�^�[�Q�b�g�p)
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NumDescriptors = 2;
	heapDesc.NodeMask = 0;
	//�f�X�N���v�^�q�[�v�̏�����
	auto result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_rtvHeap.GetAddressOf()));

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();

	DXGI_SWAP_CHAIN_DESC swDesc = {};
	_swapchain->GetDesc(&swDesc);
	int renderTargetNum = swDesc.BufferCount;

	_renderTargets.resize(renderTargetNum);

	auto heapSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//�����_�[�^�[�Q�b�g�r���[�̏�����
	for (int i = 0; i < renderTargetNum; ++i)
	{
		result = _swapchain->GetBuffer(i, IID_PPV_ARGS(_renderTargets[i].GetAddressOf()));
		_dev->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, descriptorHandle);
		descriptorHandle.ptr += heapSize;
	}
	return true;
}

Dx12Wrapper::Dx12Wrapper(HWND hwnd) :_hwnd(hwnd)
{
}


Dx12Wrapper::~Dx12Wrapper()
{
}

void Dx12Wrapper::Init()
{
	HRESULT result = S_OK;

	//�f�o�b�O���C���[�̋N��
	ID3D12Debug* debugLayer;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
	{
		debugLayer->EnableDebugLayer();
		debugLayer->Release();
	}

	DeviceInit();
	CommandInit();
	SwapChainInit();
	RTInit();
	_dev->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.GetAddressOf()));
}

void Dx12Wrapper::Update()
{
	auto heapStart = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	float clearColor[] = { 1.0f,0.0f,0.0f,1.0f };//�N���A�J���[�ݒ� 
	_cmdAllocator->Reset();//�A���P�[�^���Z�b�g 
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);//�R�}���h���X�g���Z�b�g 
	_cmdList->OMSetRenderTargets(1, &heapStart, false, nullptr);//�����_�[�^�[�Q�b�g�ݒ� 
	_cmdList->ClearRenderTargetView(heapStart, clearColor, 0, nullptr);//�N���A 

	ExecuteCommand();
	WaitFence();
	_swapchain->Present(0, 0);
}

void Dx12Wrapper::ExecuteCommand()
{
	ID3D12CommandList* cmdLists[] = { _cmdList.Get() };
	_cmdQue->ExecuteCommandLists(1, cmdLists);
	_cmdQue->Signal(_fence.Get(), ++_fenceValue);
}

void Dx12Wrapper::WaitFence()
{
	if (_fence->GetCompletedValue() != _fenceValue)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceValue, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
}


