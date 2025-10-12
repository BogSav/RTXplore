#pragma once

#include "GeometryRenderer.hpp"
#include "Object.hpp"

class SkyBoxRenderer : public GeometryRenderer, public Object
{
public:
	using Ptr = std::unique_ptr<SkyBoxRenderer>;

	static SkyBoxRenderer::Ptr CreateSkyBoxRenderer(engine::gfx::render_descriptors::DX_SKYBOX_DESCRIPTOR& descriptor);

	void Render(engine::gfx::rasterization::RenderLayer::Value renderLayer) const override;
	void BuildAccelerationStructures() override;
	void FrustumCulling(const CameraController& cameraController) override;
	void Update(float delatTime) override;

	inline const engine::math::Vector3& GetLightDirection() const { return m_lightDirection; }

	~SkyBoxRenderer();

private:
	SkyBoxRenderer() = delete;
	SkyBoxRenderer(engine::gfx::render_descriptors::DX_OBJECT_DESCRIPTOR&);

	void LoadGeometry(DescriptorVariant descriptor) override;

	engine::math::Vector3 m_lightDirection = {};
	engine::math::Vector3 m_rotationAxis = {};
	float m_rotationSpeed = 0.f;
};