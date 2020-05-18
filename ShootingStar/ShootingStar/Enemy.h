#pragma once
#include "Object.h"
class Enemy :
	public Object
{
protected:
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	bool VertexBufferInit(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	bool InitPipeLine(Microsoft::WRL::ComPtr<ID3D12Device> dev);
public:
	Enemy(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	~Enemy();
	void Update();
	void Draw(ID3D12Device* dev, ID3D12GraphicsCommandList* cmd, D3D12_VIEWPORT& view, D3D12_RECT& rect,
		ID3D12DescriptorHeap* wvp);
};

