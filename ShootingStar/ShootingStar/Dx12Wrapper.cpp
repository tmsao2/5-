#pragma comment(lib,"d3d12.lib") 
#pragma comment(lib,"dxgi.lib") 
#include <d3dx12.h>
#include "Dx12Wrapper.h"
#include "Application.h"
#include "Player.h"
#include "Input.h"

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

bool Dx12Wrapper::DepthInit()
{
	auto wsize = Application::Instance().GetWindowSize();

	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Width = wsize.w;
	resDesc.Height = wsize.h;
	resDesc.DepthOrArraySize = 1;
	resDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;

	auto result = _dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue, IID_PPV_ARGS(_depthBuffer.GetAddressOf()));

	//�f�X�N���v�^�q�[�v�ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	//�q�[�v�쐬
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_dsvHeap.GetAddressOf()));
	//�[�x�X�e���V���r���[�ݒ�
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	auto h = _dsvHeap->GetCPUDescriptorHandleForHeapStart();

	_dev->CreateDepthStencilView(_depthBuffer.Get(), &dsvDesc, h);
	return true;
}

bool Dx12Wrapper::CameraInit()
{
	auto wsize = Application::Instance().GetWindowSize();

	XMFLOAT3 eye(0, 20, -30);	//���_
	XMFLOAT3 up(0, 1, 0);		//��x�N�g��
	target = XMFLOAT3(0, 10, 0);  //�����_

								  //����n�A�r���[�s��
	_wvp.view = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));

	//�v���W�F�N�V�����s��
	//����p�A�A�X�y�N�g��A�j�A�A�t�@�[
	_wvp.projection = XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		static_cast<float>(wsize.w) / static_cast<float>(wsize.h),
		0.1f,
		1000
	);

	XMFLOAT3 light(50, 50, 50);
	XMFLOAT3 tolight(1, 1, 1);
	auto lightview = XMMatrixLookAtLH(XMLoadFloat3(&light), XMLoadFloat3(&target), XMLoadFloat3(&up));
	auto lightproj = XMMatrixOrthographicLH(40, 40, 0.1f, 1000);
	_wvp.lvp = lightview * lightproj;
	_wvp.light = XMLoadFloat3(&tolight);

	//���[���h�s��
	_wvp.world = XMMatrixIdentity();

	_wvp.eye = XMLoadFloat3(&eye);

	size_t size = sizeof(_wvp);
	size = (size + 0xff) & ~0xff;

	ID3D12Resource* _cbBuffer = nullptr;

	auto result = _dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_cbBuffer));

	result = _cbBuffer->Map(0, nullptr, (void**)&_mapWvp);
	*_mapWvp = _wvp;
	_cbBuffer->Unmap(0, nullptr);

	//�R���X�^���g�o�b�t�@�r���[�̐ݒ�
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _cbBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = size;

	//�f�X�N���v�^�q�[�v�̐ݒ�(�e�N�X�`���p)
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 1;
	heapDesc.NodeMask = 0;

	//�f�X�N���v�^�q�[�v�̏�����
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_wvpHeap));

	auto h = _wvpHeap->GetCPUDescriptorHandleForHeapStart();

	_dev->CreateConstantBufferView(&cbvDesc, h);

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
	DepthInit();
	CameraInit();
	_dev->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.GetAddressOf()));

	_pl.reset(new Player(_dev.Get()));
	_input.reset(new Input());

	auto wsize = Application::Instance().GetWindowSize();
	//�r���[�|�[�g�ݒ�
	_viewPort.TopLeftX = 0;
	_viewPort.TopLeftY = 0;
	_viewPort.Width = wsize.w;
	_viewPort.Height = wsize.h;
	_viewPort.MaxDepth = 1.0f;
	_viewPort.MinDepth = 0.0f;
	//�V�U�[��`�ݒ�
	_scissorRect.left = 0;
	_scissorRect.top = 0;
	_scissorRect.right = wsize.w;
	_scissorRect.bottom = wsize.h;
}

void Dx12Wrapper::Update()
{
	//_pl->Update();
	CameraMove();
	float clearColor[] = { 0.0f,0.0f,0.0f,1.0f };//�N���A�J���[�ݒ� 

	_cmdAllocator->Reset();//�A���P�[�^���Z�b�g 
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);//�R�}���h���X�g���Z�b�g 

	auto bbIndex = _swapchain->GetCurrentBackBufferIndex();
	auto heapStart = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto heapSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	heapStart.ptr = heapStart.ptr + bbIndex * heapSize;

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = _renderTargets[bbIndex].Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	_cmdList->OMSetRenderTargets(1, &heapStart, false, nullptr);//�����_�[�^�[�Q�b�g�ݒ� 
	_cmdList->ClearRenderTargetView(heapStart, clearColor, 0, nullptr);//�N���A 

	_pl->Draw(_dev.Get(), _cmdList.Get(), _viewPort, _scissorRect, _wvpHeap.Get());

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &BarrierDesc);
	_cmdList->Close();//�R�}���h�̃N���[�Y 
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

void Dx12Wrapper::CameraMove()
{
	XMFLOAT3 up(0, 1, 0);		//��x�N�g��
	float speed = 0.05f;

	if (_input->GetKey()['W'] & 0x80)
	{
		auto mat = XMMatrixTranslation(0, speed, 0);
		_wvp.eye = XMVector3Transform(_wvp.eye, mat);
		target = XMFLOAT3(target.x, target.y + speed, target.z);
	}
	if (_input->GetKey()['A'] & 0x80)
	{
		auto mat = XMMatrixTranslation(-speed, 0, 0);
		_wvp.eye = XMVector3Transform(_wvp.eye, mat);
		target = XMFLOAT3(target.x - speed, target.y, target.z);
	}
	if (_input->GetKey()['S'] & 0x80)
	{
		auto mat = XMMatrixTranslation(0, -speed, 0);
		_wvp.eye = XMVector3Transform(_wvp.eye, mat);
		target = XMFLOAT3(target.x, target.y - speed, target.z);
	}
	if (_input->GetKey()['D'] & 0x80)
	{
		auto mat = XMMatrixTranslation(speed, 0, 0);
		_wvp.eye = XMVector3Transform(_wvp.eye, mat);
		target = XMFLOAT3(target.x + speed, target.y, target.z);
	}
	if (_input->GetKey()[VK_UP] & 0x80)
	{
		auto mat = XMMatrixTranslation(0, 0, speed);
		_wvp.eye = XMVector3Transform(_wvp.eye, mat);
		target = XMFLOAT3(target.x, target.y, target.z + speed);
	}
	if (_input->GetKey()[VK_DOWN] & 0x80)
	{
		auto mat = XMMatrixTranslation(0, 0, -speed);
		_wvp.eye = XMVector3Transform(_wvp.eye, mat);
		target = XMFLOAT3(target.x, target.y, target.z - speed);
	}
	if (_input->GetKey()[VK_LEFT] & 0x80)
	{
		auto mat = XMMatrixRotationY(speed);

		_wvp.eye = XMVector3Transform(_wvp.eye, mat);
	}
	if (_input->GetKey()[VK_RIGHT] & 0x80)
	{
		auto mat = XMMatrixRotationY(-speed);

		_wvp.eye = XMVector3Transform(_wvp.eye, mat);
	}

	_wvp.view = XMMatrixLookAtLH(
		_wvp.eye,
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));

	*_mapWvp = _wvp;
}


