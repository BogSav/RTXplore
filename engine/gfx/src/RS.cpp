#include "RS.hpp"

#include "engine/core/DxgiInfoManager.hpp"
#include "engine/core/Utilities.hpp"

#include <assert.h>

using namespace Microsoft::WRL;

RootSignature::Ptr RootSignature::CreateEmptyRootSignature(size_t nrOfRootParams, size_t nrOfStaticSamplers)
{
	return RootSignature::Ptr(new RootSignature(nrOfRootParams, nrOfStaticSamplers));
}

void RootSignature::AddRootParameter(const CD3DX12_ROOT_PARAMETER1& desc)
{
}

CD3DX12_ROOT_PARAMETER1& RootSignature::GetRootParameter(int index)
{
	assert(index < m_rootParams.size());

	return m_rootParams.at(index);
}

CD3DX12_STATIC_SAMPLER_DESC& RootSignature::GetStaticSampler(int index)
{
	assert(index < m_staticSamplers.size());

	return m_staticSamplers.at(index);
}

RootSignature::RootSignature(size_t nrOfRootParams, size_t nrOfStaticSamplers)
	: m_rootSignatureFlags(D3D12_ROOT_SIGNATURE_FLAG_NONE)
{
	m_rootParams.reserve(nrOfRootParams);
	m_rootParams.insert(m_rootParams.end(), nrOfRootParams, {});


}

void RootSignature::AddDefaultStaticSamplers()
{
	UINT shaderRegister = (UINT)m_staticSamplers.size();

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		shaderRegister++,
		D3D12_FILTER_MIN_MAG_MIP_POINT,  // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);  // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		shaderRegister++,
		D3D12_FILTER_MIN_MAG_MIP_POINT,  // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);  // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		shaderRegister++,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,  // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);  // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		shaderRegister++,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,  // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);  // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		shaderRegister++,
		D3D12_FILTER_ANISOTROPIC,  // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,  // mipLODBias
		16);  // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		shaderRegister,
		D3D12_FILTER_ANISOTROPIC,  // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,  // mipLODBias
		16);  // maxAnisotrop

	m_staticSamplers.push_back(pointWrap);
	m_staticSamplers.push_back(pointClamp);
	m_staticSamplers.push_back(linearWrap);
	m_staticSamplers.push_back(linearClamp);
	m_staticSamplers.push_back(anisotropicWrap);
	m_staticSamplers.push_back(anisotropicClamp);
}

void RootSignature::AddDefaultShadowMapSampler()
{
	UINT shaderRegister = (UINT)m_staticSamplers.size();

	const CD3DX12_STATIC_SAMPLER_DESC shadowSampler(
		shaderRegister,
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,  // mipLODBias
		16,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);  // maxAnisotrop

	m_staticSamplers.push_back(shadowSampler);
}

void RootSignature::Finalize(ID3D12Device5* pDevice)
{
	HRESULT hr;

	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	m_descriptor.Init_1_1(
		(UINT)m_rootParams.size(),
		m_rootParams.data(),
		(UINT)m_staticSamplers.size(),
		m_staticSamplers.data(),
		m_rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	if (FAILED(D3DX12SerializeVersionedRootSignature(&m_descriptor, featureData.HighestVersion, &signature, &error)))
	{
		const std::string msg = misc::convertBlobToString(error.Get());
		throw misc::CustomException(msg);
	}

	GFX_THROW_INFO(pDevice->CreateRootSignature(
		0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&pRootSignature)));

	m_rootParams.clear();
	m_staticSamplers.clear();
}
