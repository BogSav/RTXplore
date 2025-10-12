#pragma once

#include "RS.hpp"
#include "d3dx12.h"

class GraphicsPSO;
class ComputePSO;
class RayTracingPSO;

class PSO
{
public:
	ID3D12PipelineState* GetID3D12PipelineState() const { return m_pipelineState.Get(); }
	ID3D12StateObject* GetID3D12StateObject() const { return m_pipelineStateObject.Get(); }
	ID3D12RootSignature* GetID3D12RootSignature() const { return m_rootSignature->GetID3D12RootSignature(); }
	const RootSignature& GetRootSignature() const { return *m_rootSignature; };
	RootSignature& GetRootSiganture() { return *m_rootSignature; }

	RootSignature::Ptr GetRootSignaturePtr() const { return m_rootSignature; }

	void SetRootSignature(RootSignature::Ptr rootSiganture) { m_rootSignature = rootSiganture; }

protected:
	PSO() = default;
	PSO(const PSO&) = delete;
	PSO operator=(const PSO&) = delete;

protected:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	Microsoft::WRL::ComPtr<ID3D12StateObject> m_pipelineStateObject;

	RootSignature::Ptr m_rootSignature;
};


class GraphicsPSO : public PSO
{
public:
	using Ptr = std::shared_ptr<GraphicsPSO>;

	static Ptr CreateEmptyPSO();

	// Configurare propietati PSO
	void SetBlendState(const D3D12_BLEND_DESC& BlendDesc);
	void SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc);
	void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);
	void SetSampleMask(UINT SampleMask);
	void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType);
	void SetInputLayout(UINT NumElements, std::initializer_list<D3D12_INPUT_ELEMENT_DESC> inputElementDescs);
	void SetSampleState(const DXGI_SAMPLE_DESC& SampleDesc);
	void SetDSVFormat(DXGI_FORMAT DSVFormat);
	void SetNodeMask(UINT NodeMask);
	void SetFlags(D3D12_PIPELINE_STATE_FLAGS Flags);
	void SetRTVs(UINT NumRTVs, std::initializer_list<DXGI_FORMAT> RTVFormats);

	// Setteri pentru shadere normale
	void SetVertexShader(const void* Binary, size_t Size);
	void SetPixelShader(const void* Binary, size_t Size);
	void SetGeometryShader(const void* Binary, size_t Size);
	void SetHullShader(const void* Binary, size_t Size);
	void SetDomainShader(const void* Binary, size_t Size);

	// Setteri pentru shadere alternative
	void SetVertexShader(const D3D12_SHADER_BYTECODE& Binary) { m_PSODesc.VS = Binary; }
	void SetPixelShader(const D3D12_SHADER_BYTECODE& Binary) { m_PSODesc.PS = Binary; }
	void SetGeometryShader(const D3D12_SHADER_BYTECODE& Binary) { m_PSODesc.GS = Binary; }
	void SetHullShader(const D3D12_SHADER_BYTECODE& Binary) { m_PSODesc.HS = Binary; }
	void SetDomainShader(const D3D12_SHADER_BYTECODE& Binary) { m_PSODesc.DS = Binary; }

	const inline D3D12_GRAPHICS_PIPELINE_STATE_DESC& GetDescriptor() const { return m_PSODesc; }

	void Finalize(ID3D12Device10* pDevice);

private:
	GraphicsPSO() = default;
	GraphicsPSO(const GraphicsPSO&) = delete;
	GraphicsPSO operator=(const GraphicsPSO&) = delete;

private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PSODesc = {};
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescpritors;
};


class ComputePSO : public PSO
{
public:
	using Ptr = std::shared_ptr<ComputePSO>;

	static Ptr CreateEmptyPSO();

	void SetComputeShader(const void* Binary, size_t Size);
	void SetFlags(D3D12_PIPELINE_STATE_FLAGS Flags);
	void SetNodeMask(UINT NodeMask);

	void Finalize(ID3D12Device10* pDevice);

private:
	ComputePSO() = default;
	ComputePSO(const ComputePSO&) = delete;
	ComputePSO operator=(const ComputePSO&) = delete;

private:
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_PSODesc = {};
};


class RayTracingPSO : public PSO
{
public:
	using Ptr = std::shared_ptr<RayTracingPSO>;

	static Ptr CreateEmptyPSO();

	void Finalize(ID3D12Device10* pDevice);

	CD3DX12_STATE_OBJECT_DESC desc;

private:
	RayTracingPSO() = default;
	RayTracingPSO(const GraphicsPSO&) = delete;
	RayTracingPSO operator=(const RayTracingPSO&) = delete;
};
