#pragma once

#include "Camera.hpp"
#include "Context.hpp"

class ShadowMap
{
public:
	using Ptr = std::unique_ptr<ShadowMap>;

	ShadowMap(UINT width, UINT height);

	void Create();
	void Update();

	void SetDirection(engine::math::Vector3 direction);

	void Setup(GraphicsContext& graphicsContext);
	void Finish(GraphicsContext& graphicsContext);

	inline const misc::DescriptorHandle& GetTextureSRVHandle() { return m_depthTexture->GetSrvHandle(); }

	inline const engine::math::Vector3& GetLightDirection() const { return m_lightDirection; }
	inline const OrthograficCamera& GetCamera() const { return m_camera; }

	inline void SetRenderDirty(bool toSet) { m_isRenderDirty = toSet; }
	inline bool IsRenderDirty() const { return m_isRenderDirty; }
	inline void SetDirty();
	inline bool IsDirty() const;
	inline void DecreaseDirtyCount();

private:
	bool m_isRenderDirty;
	UINT8 m_framesDirty;

	const UINT m_height;
	const UINT m_width;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	DepthTexture::Ptr m_depthTexture;

	OrthograficCamera m_camera;
	float m_distanceFromCamera;
	engine::math::Vector3 m_lightDirection;
};

inline void ShadowMap::SetDirty()
{
	m_framesDirty = Settings::GetFrameResourcesCount();
}

inline bool ShadowMap::IsDirty() const
{
	return m_framesDirty != 0;
}

inline void ShadowMap::DecreaseDirtyCount()
{
	assert(m_framesDirty > 0);

	m_framesDirty--;
}
