#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>
#include <vector>

using namespace DirectX;
using namespace Microsoft::WRL;

class Player;
class Input;

struct WVPMatrix {
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
	XMMATRIX lvp;
	XMVECTOR eye;
	XMVECTOR light;
};

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
	ComPtr<ID3D12DescriptorHeap> _dsvHeap;		//深度ステンシルビュー用
	ComPtr<ID3D12DescriptorHeap> _depthSrvHeap;	//カメラ深度描画用
	ComPtr<ID3D12DescriptorHeap> _wvpHeap;		//カメラ用

												//マルチパス用
	ComPtr<ID3D12DescriptorHeap>	_peraRtvHeap;
	ComPtr<ID3D12DescriptorHeap>	_peraSrvHeap;
	std::array<ComPtr<ID3D12Resource>, 3>	_resources;
	ComPtr<ID3D12RootSignature>		_peraSignature;
	ComPtr<ID3D12PipelineState>		_peraPipeline;

	//シェーダー
	ComPtr<ID3DBlob> _peraVsShader = nullptr;
	ComPtr<ID3DBlob> _peraPsShader = nullptr;

	D3D12_VERTEX_BUFFER_VIEW		_vb;
	ComPtr<ID3D12Resource> _vertexBuffer = nullptr;
	//深度バッファー作成
	ComPtr<ID3D12Resource> _depthBuffer = nullptr;

	//ビューポート
	D3D12_VIEWPORT _viewPort;
	//シザー矩形
	D3D12_RECT _scissorRect;

	WVPMatrix _wvp;
	WVPMatrix* _mapWvp;
	XMFLOAT3 target;

	std::shared_ptr<Player> _pl;
	std::shared_ptr<Input> _input;

	bool DeviceInit();
	bool CommandInit();
	bool SwapChainInit();
	bool RTInit();
	bool DepthInit();
	bool CameraInit();

	void ExecuteCommand();
	void WaitFence();
	void CameraMove();
public:
	Dx12Wrapper(HWND hwnd);
	~Dx12Wrapper();
	void Init();
	void Update();
};

