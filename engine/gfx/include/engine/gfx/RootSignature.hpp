#pragma once

#include "engine/core/CustomException.hpp"
#include "d3dx12.h"

class RootSignature
{
public:
	using Ptr = std::shared_ptr<RootSignature>;

	static Ptr CreateEmptyRootSignature(size_t nrOfRootParams, size_t nrOfStaticSamplers = 0);

	void AddRootParameter(const CD3DX12_ROOT_PARAMETER1& desc);

	CD3DX12_ROOT_PARAMETER1& GetRootParameter(int index);
	CD3DX12_STATIC_SAMPLER_DESC& GetStaticSampler(int index);

	void AddDefaultStaticSamplers();
	void AddDefaultShadowMapSampler();

	void SetRootSignatureFlags(D3D12_ROOT_SIGNATURE_FLAGS flags) { m_rootSignatureFlags = flags; }
	ID3D12RootSignature* GetID3D12RootSignature() const { return pRootSignature.Get(); }

	void Finalize(ID3D12Device5* pDevice);

private:
	RootSignature() = delete;
	RootSignature(RootSignature&) = delete;
	RootSignature& operator=(RootSignature&) = delete;
	RootSignature(size_t nrOfRootParams, size_t nrOfStaticSamplers = 0);

private:
	std::vector<CD3DX12_STATIC_SAMPLER_DESC> m_staticSamplers;
	std::vector<CD3DX12_ROOT_PARAMETER1> m_rootParams;

	D3D12_ROOT_SIGNATURE_FLAGS m_rootSignatureFlags;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_descriptor;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
};