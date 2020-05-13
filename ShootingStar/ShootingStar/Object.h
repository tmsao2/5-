#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <memory>
#include <map>
#include <wrl.h>

class Object
{
protected:
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	//インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW _ibView = {};

	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;

	//パイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;

	Microsoft::WRL::ComPtr<ID3DBlob> _vsShader;
	Microsoft::WRL::ComPtr<ID3DBlob> _psShader;

	Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer = nullptr;

	bool CreatePipeLine(Microsoft::WRL::ComPtr<ID3D12Device> dev, std::vector<D3D12_INPUT_ELEMENT_DESC> layout);
	bool CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	
public:
	Object(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	~Object();
};

