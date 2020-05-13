#pragma once
#include "Object.h"
class Player :
	public Object
{
private:
	bool VertexBufferInit(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	bool InitPipeLine(Microsoft::WRL::ComPtr<ID3D12Device> dev);
public:
	Player(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	~Player();
	void Update();
	void Draw(ID3D12Device* dev, ID3D12GraphicsCommandList* cmd, D3D12_VIEWPORT& view, D3D12_RECT& rect,
		ID3D12DescriptorHeap* wvp);
};

