#pragma once

#include "DX_Context.hpp"
#include "Texture.hpp"

class TextureManager
{
public:
	TextureManager();

	void CreateSRVHandles();
	void LoadTexturesFormFiles(ID3D12Device10* pDevice, ID3D12CommandQueue* pCommandQueue);
	void SwapNonPixelShaderTextures(GraphicsContext& context);

	const ColorTexture& GetTexture(std::wstring textureName) const;

private:
	std::vector<ColorTexture::Ptr> m_textures;
};