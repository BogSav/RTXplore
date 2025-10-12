#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include "GPUBuffers.hpp"
#include "engine/math/Color.hpp"

class Texture : public GpuResource
{
public:
	using Ptr = std::unique_ptr<Texture>;

	inline void SetName(std::wstring toSet)
	{
		m_pResource->SetName(toSet.c_str());
		m_name = toSet;
	}

	inline bool IsNonPixelShaderResource() const { return m_isNonPixelShaderResource; }
	inline const std::wstring& GetName() const { return m_name; }
	inline const std::wstring& GetPath() const { return m_path; }

	void AllocateSrvHandle(
		DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN, D3D12_SHADER_RESOURCE_VIEW_DESC* srvDescriptor = nullptr);

	INLINE const misc::DescriptorHandle& GetSrvHandle() const { return m_SrvHandle; }

protected:
	Texture() = delete;
	Texture(std::wstring path, std::wstring name, bool isCube = false, bool isNonPixelShaderResource = false);
	Texture(bool isNonPixelShaderResource = false, bool isCubeMap = false);

	static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetUAVFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT Format);
	static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT Format);

	static D3D12_RESOURCE_DESC CeeateTextureDescriptor(
		UINT width,
		UINT height,
		DXGI_FORMAT format,
		DXGI_SAMPLE_DESC sampleDesc,
		D3D12_RESOURCE_FLAGS flags,
		bool isCube = true);

protected:
	misc::DescriptorHandle m_SrvHandle;
	DXGI_FORMAT m_format;

	bool m_isNonPixelShaderResource = false;
	bool m_isCubeMap = false;
	std::wstring m_path;
	std::wstring m_name;
};


class ColorTexture : public Texture
{
public:
	using Ptr = std::unique_ptr<ColorTexture>;

	ColorTexture(engine::math::Color color = engine::math::Color(0.f, 0.f, 0.f), bool isCubeMap = false, bool isNonPixelShaderResource = false);
	ColorTexture(std::wstring path, std::wstring name, bool isCubeMap = false, bool isNonPixelShaderResource = false);

	void Create(UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, std::wstring name);
	void CreateFromSwapChain(IDXGISwapChain4* pSwapChain, UINT frameIndex);

	void AllocateUavHandle(
		DXGI_FORMAT uavFormat = DXGI_FORMAT_UNKNOWN, D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDescriptor = nullptr);
	void AllocateRtvHandle(
		DXGI_FORMAT rtvFormat = DXGI_FORMAT_UNKNOWN, D3D12_RENDER_TARGET_VIEW_DESC* rtvDescriptor = nullptr);

	INLINE const misc::DescriptorHandle& GetRtvHandle(size_t index) const { return m_RtvHandle[index]; };
	INLINE const misc::DescriptorHandle& GetRtvHandle() const { return m_RtvHandle[0]; };
	INLINE const misc::DescriptorHandle& GetUavHandle() const { return m_UavHandle; };
	INLINE const engine::math::Color& GetClearColor() const { return m_clearColor; }

	friend class TextureManager;

private:
	engine::math::Color m_clearColor;

	misc::DescriptorHandle m_UavHandle;
	misc::DescriptorHandle m_RtvHandle[6];
};


class DepthTexture : public Texture
{
public:
	using Ptr = std::unique_ptr<DepthTexture>;

	DepthTexture(float clearDepth, UINT8 clearStencil);

	void Create(UINT width, UINT height, DXGI_FORMAT format, std::wstring name);

	void AllocateDsvHandle(
		DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN, D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDescriptor = nullptr);

	INLINE const misc::DescriptorHandle& GetDsvHandle() const { return m_DsvHandle; }
	INLINE const UINT8& GetClearStencil() const { return m_clearStencil; }
	INLINE const float& GetClearDepth() const { return m_clearDepth; }

	friend class TextureManager;

private:
	float m_clearDepth;
	UINT8 m_clearStencil;

	misc::DescriptorHandle m_DsvHandle;
};