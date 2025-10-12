#pragma once

#include "GeometryRenderer.hpp"
#include "Object.hpp"

namespace engine::gfx
{

class WaterRenderer : public GeometryRenderer, public Object
{
public:
	using Ptr = std::unique_ptr<WaterRenderer>;

	static WaterRenderer::Ptr CreateWaterRenderer(engine::gfx::render_descriptors::DX_WATER_DESCRIPTOR& descriptor);

	void Render(engine::gfx::rasterization::RenderLayer::Value renderLayer) const override;
	void BuildAccelerationStructures() override;
	void FrustumCulling(const CameraController& cameraController) override;
	void Update(float delatTime) override;

	inline const WaveProperties& GetWaveParameters(int index) const { return m_waveProperties[index]; }
	inline const float GetCubeMapSphereRadius() const { return m_cubeMapSphereRadius; }
	inline const engine::math::Vector3& GetCubeMapCenter() const { return m_cubeMapCenter; }
	inline void SetCubeMapCenter(engine::math::Vector3 toSet) { m_cubeMapCenter = toSet; };

	~WaterRenderer();

private:
	WaterRenderer() = delete;
	WaterRenderer(const engine::gfx::render_descriptors::DX_OBJECT_DESCRIPTOR&);

	void LoadGeometry(DescriptorVariant descriptor) override;

	std::array<WaveProperties, WAVE_PROPERTIES_COUNT> m_waveProperties;

	std::vector<Chunk> m_chunks;
	std::vector<D3D12_RAYTRACING_AABB> m_AABBs;
	GpuResource m_AABBsResource;

	engine::math::Vector3 m_texturePosition;
	engine::math::Vector3 m_textureDirection;
	engine::math::Vector3 m_textureScale;
	float m_textureMoveSpeed;

	engine::math::Vector3 m_cubeMapCenter;
	float m_cubeMapSphereRadius;
};

}  // namespace engine::gfx
