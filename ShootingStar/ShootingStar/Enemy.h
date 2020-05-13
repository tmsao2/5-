#pragma once
#include "Object.h"
class Enemy :
	public Object
{
public:
	Enemy(Microsoft::WRL::ComPtr<ID3D12Device> dev);
	~Enemy();
};

