#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>

using namespace DirectX;
using namespace Microsoft::WRL;

class Dx12Wrapper
{
private:
	HWND _hwnd;
	ComPtr<ID3D12Device> _dev = nullptr;
	ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<ID3D12CommandQueue> _cmdQue = nullptr;
	ComPtr<IDXGIFactory6> _dxgi = nullptr;
	ComPtr<IDXGISwapChain4> _swapchain = nullptr;
	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceValue = 0;
	//�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@
	std::vector<ComPtr<ID3D12Resource>> _renderTargets;
	//�f�X�N���v�^���i�[����̈�
	ComPtr<ID3D12DescriptorHeap> _rtvHeap;		//�����_�[�^�[�Q�b�g�r���[�p

	bool DeviceInit();
	bool CommandInit();
	bool SwapChainInit();
	bool RTInit();
	void ExecuteCommand();
	void WaitFence();
public:
	Dx12Wrapper(HWND hwnd);
	~Dx12Wrapper();
	void Init();
	void Update();
};

