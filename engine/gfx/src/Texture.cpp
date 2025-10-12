#include "Texture.hpp"

#include "GraphicsResources.hpp"

////////////////////////////////////////////////////////////////////////////////////////////
// Creare Handleri si Descriptori
void Texture::AllocateSrvHandle(DXGI_FORMAT srvFormat, D3D12_SHADER_RESOURCE_VIEW_DESC* srvDescriptor)
{
	if (srvFormat == DXGI_FORMAT_UNKNOWN)
	{
		if (DepthTexture* depthTexture = dynamic_cast<DepthTexture*>(this))
		{
			srvFormat = GetDepthFormat(m_format);
		}
		else
		{
			srvFormat = m_format;
		}
	}

	if (srvDescriptor != nullptr)
	{
		m_SrvHandle = GpuResource::CreateTextureView(*this, srvDescriptor);
	}
	else
	{
		const D3D12_RESOURCE_DESC& textDesc = m_pResource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = srvFormat;

		if (textDesc.DepthOrArraySize == 1)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipLevels = textDesc.MipLevels;
			desc.Texture2D.MostDetailedMip = 0;
			desc.Texture2D.ResourceMinLODClamp = 0.f;
		}
		else if (m_isCubeMap)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			desc.TextureCube.MostDetailedMip = 0;
			desc.TextureCube.MipLevels = textDesc.MipLevels;
			desc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		else
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MostDetailedMip = 0;
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.MipLevels = textDesc.MipLevels;
			desc.Texture2DArray.ArraySize = textDesc.DepthOrArraySize;
			desc.Texture2DArray.ResourceMinLODClamp = 0.f;
		}

		m_SrvHandle = GpuResource::CreateTextureView(*this, &desc);
	}
}

void ColorTexture::AllocateUavHandle(DXGI_FORMAT uavFormat, D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDescriptor)
{
	if (uavFormat == DXGI_FORMAT_UNKNOWN)
	{
		uavFormat = GetUAVFormat(m_format);
	}

	if (uavDescriptor != nullptr)
	{
		m_UavHandle = GpuResource::CreateTextureView(*this, uavDescriptor);
	}
	else
	{
		const D3D12_RESOURCE_DESC& textDesc = m_pResource->GetDesc();

		D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.Format = uavFormat;

		if (textDesc.DepthOrArraySize == 1 && !m_isCubeMap)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		}
		else if (textDesc.DepthOrArraySize > 1)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = textDesc.DepthOrArraySize;
		}

		m_UavHandle = GpuResource::CreateTextureView(*this, &desc);
	}
}

void ColorTexture::AllocateRtvHandle(DXGI_FORMAT rtvFormat, D3D12_RENDER_TARGET_VIEW_DESC* rtvDescriptor)
{
	if (rtvFormat == DXGI_FORMAT_UNKNOWN)
	{
		rtvFormat = m_format;
	}

	if (rtvDescriptor != nullptr)
	{
		m_RtvHandle[0] = GpuResource::CreateTextureView(*this, rtvDescriptor);
	}
	else
	{
		D3D12_RENDER_TARGET_VIEW_DESC desc = {};
		desc.Format = rtvFormat;

		if (m_isCubeMap)
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = 0;
			desc.Texture2DArray.PlaneSlice = 0;
			desc.Texture2DArray.ArraySize = 1;

			for (int i = 0; i < 6; i++)
			{
				desc.Texture2DArray.FirstArraySlice = i;

				m_RtvHandle[i] = GpuResource::CreateTextureView(*this, &desc);
			}
		}
		else
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			m_RtvHandle[0] = GpuResource::CreateTextureView(*this, &desc);
		}
	}
}

void DepthTexture::AllocateDsvHandle(DXGI_FORMAT dsvFormat, D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDescriptor)
{
	if (dsvFormat == DXGI_FORMAT_UNKNOWN)
	{
		dsvFormat = GetDSVFormat(m_format);
	}

	if (dsvDescriptor != nullptr)
	{
		m_DsvHandle = GpuResource::CreateTextureView(*this, dsvDescriptor);
	}
	else
	{
		const D3D12_RESOURCE_DESC& textDesc = m_pResource->GetDesc();

		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
		desc.Format = dsvFormat;

		if (textDesc.DepthOrArraySize == 1 && !m_isCubeMap)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		}
		else if (textDesc.DepthOrArraySize > 1)
		{
			desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.FirstArraySlice = 0;
			desc.Texture2DArray.ArraySize = textDesc.DepthOrArraySize;
		}

		m_DsvHandle = GpuResource::CreateTextureView(*this, &desc);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Creare resurse
void DepthTexture::Create(UINT width, UINT height, DXGI_FORMAT format, std::wstring name)
{
	HRESULT hr;

	m_UsageState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	m_format = format;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = Settings::GetGraphicsSettings().GetMSAASampleCount();
	sampleDesc.Quality = Settings::GetGraphicsSettings().GetMSAAQuality();

	D3D12_RESOURCE_DESC textureDecs = Texture::CeeateTextureDescriptor(
		width, height, format, sampleDesc, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, false);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = m_format;
	clearValue.DepthStencil.Depth = m_clearDepth;
	clearValue.DepthStencil.Stencil = m_clearStencil;

	GFX_THROW_INFO(GraphicsResources::GetDevice()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDecs,
		m_UsageState,
		&clearValue,
		IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())));

	SetName(name);
}

void ColorTexture::Create(UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, std::wstring name)
{
	HRESULT hr;

	m_UsageState = D3D12_RESOURCE_STATE_COMMON;
	m_format = format;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = Settings::GetGraphicsSettings().GetMSAASampleCount();
	sampleDesc.Quality = Settings::GetGraphicsSettings().GetMSAAQuality();

	D3D12_RESOURCE_DESC textureDecs =
		Texture::CeeateTextureDescriptor(width, height, format, sampleDesc, flags, m_isCubeMap);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = m_format;
	clearValue.Color[0] = m_clearColor.R();
	clearValue.Color[1] = m_clearColor.G();
	clearValue.Color[2] = m_clearColor.B();
	clearValue.Color[3] = m_clearColor.A();

	GFX_THROW_INFO(GraphicsResources::GetDevice()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDecs,
		m_UsageState,
		&clearValue,
		IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())));

	SetName(name);
}

void ColorTexture::CreateFromSwapChain(IDXGISwapChain4* pSwapChain, UINT frameIndex)
{
	pSwapChain->GetBuffer(frameIndex, IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf()));

	m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	m_UsageState = D3D12_RESOURCE_STATE_COMMON;
	m_format = m_pResource->GetDesc().Format;

	SetName(L"RenderTexture" + std::to_wstring(frameIndex));
}

DXGI_FORMAT Texture::GetBaseFormat(DXGI_FORMAT defaultFormat)
{
	switch (defaultFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_TYPELESS;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_TYPELESS;

	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_TYPELESS;

	// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return DXGI_FORMAT_R32G8X24_TYPELESS;

	// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT: return DXGI_FORMAT_R32_TYPELESS;

	// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;

	// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM: return DXGI_FORMAT_R16_TYPELESS;

	default: return defaultFormat;
	}
}

DXGI_FORMAT Texture::GetUAVFormat(DXGI_FORMAT defaultFormat)
{
	switch (defaultFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;

	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM;

	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM;

	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;

#ifdef _DEBUG
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_D16_UNORM: throw misc::customException("Requested a UAV Format for a depth stencil Format.");
#endif

	default: return defaultFormat;
	}
}

DXGI_FORMAT Texture::GetDSVFormat(DXGI_FORMAT defaultFormat)
{
	switch (defaultFormat)
	{
	// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

	// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT: return DXGI_FORMAT_D32_FLOAT;

	// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM: return DXGI_FORMAT_D16_UNORM;

	default: return defaultFormat;
	}
}

DXGI_FORMAT Texture::GetDepthFormat(DXGI_FORMAT defaultFormat)
{
	switch (defaultFormat)
	{
	// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

	// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;

	// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

	// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM: return DXGI_FORMAT_R16_UNORM;

	default: return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_RESOURCE_DESC Texture::CeeateTextureDescriptor(
	UINT width, UINT height, DXGI_FORMAT format, DXGI_SAMPLE_DESC sampleDesc, D3D12_RESOURCE_FLAGS flags, bool isCube)
{
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Alignment = 0;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.DepthOrArraySize = isCube ? 6 : 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = GetBaseFormat(format);
	textureDesc.SampleDesc = sampleDesc;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = flags;

	return textureDesc;
}

ColorTexture::ColorTexture(engine::math::Color color, bool isCubeMap, bool isNonPixelShaderResource)
	: Texture(isNonPixelShaderResource, isCubeMap), m_clearColor()
{
}

ColorTexture::ColorTexture(std::wstring path, std::wstring name, bool isCubeMap, bool isNonPixelShaderResource)
	: Texture(path, name, isCubeMap, isNonPixelShaderResource)
{
}

DepthTexture::DepthTexture(float clearDepth, UINT8 clearStencil)
	: Texture(false, false), m_clearDepth(clearDepth), m_clearStencil(clearStencil)
{
}

Texture::Texture(std::wstring path, std::wstring name, bool isCube, bool isNonPixelShaderResource)
	: m_path(path), m_isCubeMap(isCube), m_isNonPixelShaderResource(isNonPixelShaderResource), m_name(name)
{
}

Texture::Texture(bool isNonPixelShaderResource, bool isCubeMap)
	: m_isCubeMap(isCubeMap), m_isNonPixelShaderResource(isNonPixelShaderResource)
{
}
