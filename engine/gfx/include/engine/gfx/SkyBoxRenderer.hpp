#pragma once

#include "GeometryRenderer.hpp"
#include "Object.hpp"

class SkyBoxRenderer : public GeometryRenderer, public Object
{
public:
	using Ptr = std::unique_ptr<SkyBoxRenderer>;

	static SkyBoxRenderer::Ptr CreateSkyBoxRenderer(misc::render_descriptors::DX_SKYBOX_DESCRIPTOR& descriptor);

	void Render(misc::rasterization::RenderLayer::Value renderLayer) const override;
	void BuildAccelerationStructures() override;
	void FrustumCulling(const CameraController& cameraController) override;
	void Update(float delatTime) override;

	inline const Math::Vector3& GetLightDirection() const { return m_lightDirection; }

	~SkyBoxRenderer();

private:
	SkyBoxRenderer() = delete;
	SkyBoxRenderer(misc::render_descriptors::DX_OBJECT_DESCRIPTOR&);

	void LoadGeometry(DescriptorVariant descriptor) override;

	Math::Vector3 m_lightDirection = {};
	Math::Vector3 m_rotationAxis = {};
	float m_rotationSpeed = 0.f;
};