#include "Object.h"
#include <d3dx12.h>


bool Object::CreatePipeLine(Microsoft::WRL::ComPtr<ID3D12Device> dev, std::vector<D3D12_INPUT_ELEMENT_DESC> layout)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	//ルートシグネチャと頂点レイアウト
	gpsDesc.pRootSignature = _rootSignature.Get();
	gpsDesc.InputLayout.pInputElementDescs = layout.data();
	gpsDesc.InputLayout.NumElements = layout.size();
	//シェーダー系
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(_vsShader.Get());
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(_psShader.Get());
	//レンダーターゲット
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	/*gpsDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;*/

	//深度ステンシル
	gpsDesc.DepthStencilState.DepthEnable = true;
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	//ラスタライザ
	//ラスタライザの設定
	D3D12_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rsDesc.CullMode = D3D12_CULL_MODE_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rsDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rsDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rsDesc.DepthClipEnable = true;
	rsDesc.MultisampleEnable = false;
	rsDesc.AntialiasedLineEnable = false;
	rsDesc.ForcedSampleCount = 0;
	rsDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	gpsDesc.RasterizerState = rsDesc;

	D3D12_RENDER_TARGET_BLEND_DESC renderBlendDesc = {};
	renderBlendDesc.BlendEnable = true;
	renderBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	renderBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	renderBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderBlendDesc.LogicOp = D3D12_LOGIC_OP_CLEAR;
	renderBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//その他
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	for (auto i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		blendDesc.RenderTarget[i] = renderBlendDesc;
	}
	gpsDesc.NodeMask = 0;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.SampleMask = 0xffffffff;
	gpsDesc.BlendState = blendDesc;
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	auto result = dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(_pipelineState.GetAddressOf()));
	if (FAILED(result))
	{
		return false;
	}
	return true;
}

bool Object::CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> dev)
{
	//サンプラの設定
	D3D12_STATIC_SAMPLER_DESC sampleDesc[2] = {};
	sampleDesc[0].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;//補間しない
	sampleDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
	sampleDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
	sampleDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
	sampleDesc[0].MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
	sampleDesc[0].MinLOD = 0.0f;//ミップマップ最小値
	sampleDesc[0].MipLODBias = 0.0f;
	sampleDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampleDesc[0].ShaderRegister = 0;
	sampleDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	sampleDesc[0].RegisterSpace = 0;
	sampleDesc[0].MaxAnisotropy = 0;
	sampleDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampleDesc[1] = sampleDesc[0];
	sampleDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampleDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampleDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampleDesc[1].ShaderRegister = 1;

	//レンジの設定
	D3D12_DESCRIPTOR_RANGE range[1] = {};
	//カメラ
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;
	range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメーターの設定
	D3D12_ROOT_PARAMETER rootParam[1] = {};
	//カメラ
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[0].DescriptorTable.pDescriptorRanges = &range[0];
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;

	//ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC rsd = {};
	rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsd.NumParameters = _countof(rootParam);
	rsd.pParameters = rootParam;
	rsd.NumStaticSamplers = _countof(sampleDesc);
	rsd.pStaticSamplers = sampleDesc;

	//シグネチャ、エラーの初期化
	auto result = D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	//ルートシグネチャの生成
	result = dev->CreateRootSignature(0, signature->GetBufferPointer(),
		signature->GetBufferSize(), IID_PPV_ARGS(_rootSignature.GetAddressOf()));

	if (FAILED(result))
	{
		return false;
	}
	return true;
}

Object::Object(Microsoft::WRL::ComPtr<ID3D12Device> dev)
{
}


Object::~Object()
{
}
