#include "Player.h"

Player::Player(Microsoft::WRL::ComPtr<ID3D12Device> dev) :Object(dev)
{
	CreateRootSignature(dev);
	InitPipeLine(dev);
}

Player::~Player()
{
}

bool Player::VertexBufferInit(Microsoft::WRL::ComPtr<ID3D12Device> dev)
{
	return false;
}

bool Player::InitPipeLine(Microsoft::WRL::ComPtr<ID3D12Device> dev)
{
	return false;
}

void Player::Update()
{
}

void Player::Draw(ID3D12Device * dev, ID3D12GraphicsCommandList * cmd, D3D12_VIEWPORT & view, D3D12_RECT & rect, ID3D12DescriptorHeap * wvp)
{
}
