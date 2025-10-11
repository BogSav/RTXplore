#pragma once

#include "GeometryRenderer.hpp"

class TextureRenderer : public GeometryRenderer
{
public:
	using Ptr = std::unique_ptr<TextureRenderer>;

	static TextureRenderer::Ptr CreateTextureRenderer(misc::render_descriptors::DX_TEXTURE_DESCRIPTOR& descriptor);

	void Render(misc::rasterization::RenderLayer::Value renderLayer) const;
	void BuildAccelerationStructures();
	void FrustumCulling(const CameraController& cameraController);
	void Update(float deltaTime);

	void SetTextureSRVHandle(const misc::DescriptorHandle& textureSRVHandle);

private:
	TextureRenderer () = default;

	void LoadGeometry(DescriptorVariant descriptor) override;

private:
	misc::DescriptorHandle m_textureSRVHandle;
};