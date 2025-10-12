#include "TextureManager.hpp"

#include "engine/core/DxgiInfoManager.hpp"
#include "engine/core/Utilities.hpp"
#include "engine/core/Settings.hpp"

#include <DDSTextureLoader.h>
#include <ResourceUploadBatch.h>

#include <memory>

using namespace DirectX;

// Helper macro for texture paths
#ifndef TEXTURE_DIR
#define TEXTURE_DIR "assets/processed/"
#endif

// Macro to convert to wide string
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define TEXTURE_DIR_W WIDEN(TEXTURE_DIR)

// Helper function to create full texture path
inline std::wstring TexturePath(const wchar_t* filename)
{
	return std::wstring(TEXTURE_DIR_W) + filename;
}

TextureManager::TextureManager()
{
}

void TextureManager::LoadTexturesFormFiles(ID3D12Device10* pDevice, ID3D12CommandQueue* pCommandQueue)
{
	HRESULT hr;

	// Create the textures first
	m_textures.push_back(std::make_unique<ColorTexture>(
		TexturePath(L"Terrain.Array.Diffuse.dds").c_str(), L"Terrain.Diffuse", false, engine::core::Settings::UseRayTracing()));
	m_textures.push_back(std::make_unique<ColorTexture>(
		TexturePath(L"Terrain.Array.Normal.dds").c_str(), L"Terrain.Normal", false, engine::core::Settings::UseRayTracing()));
	m_textures.push_back(std::make_unique<ColorTexture>(
		TexturePath(L"Water.Normal.dds").c_str(), L"Water.Normal", false, engine::core::Settings::UseRayTracing()));
	m_textures.push_back(
		std::make_unique<ColorTexture>(TexturePath(L"SkyBox.dds").c_str(), L"SkyBox", true, engine::core::Settings::UseRayTracing()));

	m_textures.push_back(
		std::make_unique<ColorTexture>(TexturePath(L"Terrain.Array.Disp.dds").c_str(), L"Terrain.Displacement", false, true));
	m_textures.push_back(std::make_unique<ColorTexture>(TexturePath(L"Water.Disp.dds").c_str(), L"Water.Disp", false, true));

	// Load from files
	ResourceUploadBatch resourceUpload(pDevice);
	resourceUpload.Begin();

	for (auto& texture : m_textures)
	{
		if (texture->GetPath() == L"")
			continue;

		GFX_THROW_INFO(CreateDDSTextureFromFile(
			pDevice,
			resourceUpload,
			texture->GetPath().c_str(),
			texture->m_pResource.ReleaseAndGetAddressOf(),
			false,
			0u,
			nullptr,
			&texture->m_isCubeMap));

		texture->SetName(texture->GetName());
		texture->m_UsageState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		texture->m_format = texture->m_pResource->GetDesc().Format;
		texture->m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	auto uploadResourcesFinished = resourceUpload.End(pCommandQueue);
	uploadResourcesFinished.wait();
}

void TextureManager::CreateSRVHandles()
{
	for (auto& texture : m_textures)
	{
		texture->AllocateSrvHandle();
	}
}

void TextureManager::SwapNonPixelShaderTextures(GraphicsContext& context)
{
	for (auto& texture : m_textures)
	{
		if (!texture->IsNonPixelShaderResource())
			continue;

		context.TransitionResource(
			*texture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	context.FlushResourceBarriers();
}

const ColorTexture& TextureManager::GetTexture(std::wstring textureName) const
{
	return **std::find_if(
		m_textures.begin(),
		m_textures.end(),
		[&textureName](const auto& texture) { return texture->GetName() == textureName; });
}
