#pragma once
#include "Object.h"

class Input;

class Player :
	public Object
{
private:
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	bool VertexBufferInit(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	bool InitPipeLine(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	std::shared_ptr<Input>		_input;

public:
	Player(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	~Player();
	void Update();
	void Draw(ID3D12Device* dev, ID3D12GraphicsCommandList* cmd, D3D12_VIEWPORT& view, D3D12_RECT& rect,
		ID3D12DescriptorHeap* wvp);
};

