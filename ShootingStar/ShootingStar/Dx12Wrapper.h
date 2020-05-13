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
	//バックバッファとフロントバッファ
	std::vector<ComPtr<ID3D12Resource>> _renderTargets;
	//デスクリプタを格納する領域
	ComPtr<ID3D12DescriptorHeap> _rtvHeap;		//レンダーターゲットビュー用

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

