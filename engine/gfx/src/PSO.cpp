#include "PSO.hpp"

#include "engine/core/DxgiInfoManager.hpp"
#include "engine/core/Utilities.hpp"

GraphicsPSO::Ptr GraphicsPSO::CreateEmptyPSO()
{
	return GraphicsPSO::Ptr(new GraphicsPSO());
}

void GraphicsPSO::SetBlendState(const D3D12_BLEND_DESC& BlendDesc)
{
	m_PSODesc.BlendState = BlendDesc;
}

void GraphicsPSO::SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc)
{
	m_PSODesc.RasterizerState = RasterizerDesc;
}

void GraphicsPSO::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc)
{
	m_PSODesc.DepthStencilState = DepthStencilDesc;
}

void GraphicsPSO::SetSampleMask(UINT SampleMask)
{
	m_PSODesc.SampleMask = SampleMask;
}

void GraphicsPSO::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType)
{
	if (TopologyType == D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED)
		throw misc::CustomException("Nu se poate crea PSO cu topologie undefined");
	m_PSODesc.PrimitiveTopologyType = TopologyType;
}

void GraphicsPSO::SetInputLayout(UINT NumElements, std::initializer_list<D3D12_INPUT_ELEMENT_DESC> inputElementDescs)
{
	if (NumElements > 0)
	{
		m_inputElementDescpritors.reserve(NumElements);
		m_inputElementDescpritors = std::vector<D3D12_INPUT_ELEMENT_DESC>(inputElementDescs);

		m_PSODesc.InputLayout.pInputElementDescs = &m_inputElementDescpritors[0];
	}
	else
	{
		m_PSODesc.InputLayout.pInputElementDescs = nullptr;
	}

	m_PSODesc.InputLayout.NumElements = NumElements;
}

void GraphicsPSO::SetSampleState(const DXGI_SAMPLE_DESC& SampleDesc)
{
	m_PSODesc.SampleDesc = SampleDesc;
}

void GraphicsPSO::SetDSVFormat(DXGI_FORMAT DSVFormat)
{
	m_PSODesc.DSVFormat = DSVFormat;
}

void GraphicsPSO::SetNodeMask(UINT NodeMask)
{
	m_PSODesc.NodeMask = NodeMask;
}

void GraphicsPSO::SetFlags(D3D12_PIPELINE_STATE_FLAGS Flags)
{
	m_PSODesc.Flags = Flags;
}

void GraphicsPSO::SetRTVs(UINT NumRTVs, std::initializer_list<DXGI_FORMAT> RTVFormats)
{
	m_PSODesc.NumRenderTargets = NumRTVs;

	for (auto it = RTVFormats.begin(); it != RTVFormats.end(); it++)
	{
		size_t i = it - RTVFormats.begin();
		m_PSODesc.RTVFormats[i] = *it;
	}

	for (size_t i = RTVFormats.size(); i < 8; i++)
	{
		m_PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}
}

void GraphicsPSO::SetVertexShader(const void* Binary, size_t Size)
{
	m_PSODesc.VS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size);
}

void GraphicsPSO::SetPixelShader(const void* Binary, size_t Size)
{
	m_PSODesc.PS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size);
}

void GraphicsPSO::SetGeometryShader(const void* Binary, size_t Size)
{
	m_PSODesc.GS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size);
}

void GraphicsPSO::SetHullShader(const void* Binary, size_t Size)
{
	m_PSODesc.HS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size);
}

void GraphicsPSO::SetDomainShader(const void* Binary, size_t Size)
{
	m_PSODesc.DS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size);
}

void GraphicsPSO::Finalize(ID3D12Device10* pDevice)
{
	HRESULT hr;

	m_PSODesc.pRootSignature = m_rootSignature->GetID3D12RootSignature();
	GFX_THROW_INFO(pDevice->CreateGraphicsPipelineState(&m_PSODesc, IID_PPV_ARGS(&m_pipelineState)));
}

RayTracingPSO::Ptr RayTracingPSO::CreateEmptyPSO()
{
	return RayTracingPSO::Ptr(new RayTracingPSO());
}

void RayTracingPSO::Finalize(ID3D12Device10* pDevice)
{
	HRESULT hr;

	// misc::PrintStateObjectDesc(desc);

	GFX_THROW_INFO(pDevice->CreateStateObject(desc, IID_PPV_ARGS(&m_pipelineStateObject)));
}

ComputePSO::Ptr ComputePSO::CreateEmptyPSO()
{
	return ComputePSO::Ptr(new ComputePSO());
}

void ComputePSO::SetComputeShader(const void* Binary, size_t Size)
{
	m_PSODesc.CS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size);
}

void ComputePSO::SetFlags(D3D12_PIPELINE_STATE_FLAGS Flags)
{
	m_PSODesc.Flags = Flags;
}

void ComputePSO::SetNodeMask(UINT NodeMask)
{
	m_PSODesc.NodeMask = NodeMask;
}

void ComputePSO::Finalize(ID3D12Device10* pDevice)
{
	HRESULT hr;

	// misc::PrintStateObjectDesc(desc);
	m_PSODesc.pRootSignature = m_rootSignature->GetID3D12RootSignature();
	GFX_THROW_INFO(pDevice->CreateComputePipelineState(&m_PSODesc, IID_PPV_ARGS(&m_pipelineState)));
}