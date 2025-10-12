#pragma once

#include "CameraController.hpp"
#include "Context.hpp"
#include "Texture.hpp"

class DynamicCubeMap
{
public:
	using Ptr = std::unique_ptr<DynamicCubeMap>;

	DynamicCubeMap(UINT width, UINT height);

	void Create();
	void Update();

	inline const misc::DescriptorHandle& GetRtvHandle(size_t index) const
	{
		return m_cubeMapTexture->GetRtvHandle(index);
	}
	inline const CubeMapCameraController& GetCameraController() const { return m_cameraController; }

	void Setup(GraphicsContext& graphicsContext);
	void SetRenderTarget(GraphicsContext& graphicsContext, UINT index);
	void Finish(GraphicsContext& graphicsContext);

	inline float GetCubeMapSphereRadius() const { return m_cubeMapSphereRadius; }
	inline engine::math::Vector3 GetCubeMapCenter() const { return m_center; }

	inline void SetRenderDirty(bool toSet) { m_isRenderDirty = toSet; }
	inline bool IsRenderDirty() const { return m_isRenderDirty; }
	void SetCenter(engine::math::Vector3 center);

	inline void DecreaseDirtyCount();
	inline bool IsDirty() const;
	inline void SetDirty();

private:
	bool m_isRenderDirty;
	UINT8 m_framesDirty;

	float m_cubeMapSphereRadius;
	engine::math::Vector3 m_center;

	const UINT m_height;
	const UINT m_width;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	ColorTexture::Ptr m_cubeMapTexture;
	DepthTexture::Ptr m_depthTexture;

	CubeMapCameraController m_cameraController;
};

inline void DynamicCubeMap::DecreaseDirtyCount()
{
	assert(m_framesDirty > 0);

	m_framesDirty--;
}

inline bool DynamicCubeMap::IsDirty() const
{
	return m_framesDirty != 0;
}

inline void DynamicCubeMap::SetDirty()
{
	m_framesDirty = Settings::GetFrameResourcesCount();
}
