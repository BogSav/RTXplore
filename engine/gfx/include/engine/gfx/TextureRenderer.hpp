#pragma once

#include "GeometryRenderer.hpp"

class TextureRenderer : public GeometryRenderer
{
public:
	using Ptr = std::unique_ptr<TextureRenderer>;

	static TextureRenderer::Ptr CreateTextureRenderer(engine::gfx::render_descriptors::DX_TEXTURE_DESCRIPTOR& descriptor);

	void Render(engine::gfx::rasterization::RenderLayer::Value renderLayer) const override;
	void BuildAccelerationStructures() override;
	void FrustumCulling(const CameraController& cameraController) override;
	void Update(float deltaTime) override;

	void SetTextureSRVHandle(const engine::gfx::DescriptorHandle& textureSRVHandle);

private:
	TextureRenderer () = default;

	void LoadGeometry(DescriptorVariant descriptor) override;

private:
	engine::gfx::DescriptorHandle m_textureSRVHandle;
};