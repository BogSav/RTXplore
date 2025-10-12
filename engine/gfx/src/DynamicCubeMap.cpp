#include "DynamicCubeMap.hpp"
#include "engine/core/DxgiInfoManager.hpp"

namespace engine::gfx
{

DynamicCubeMap::DynamicCubeMap(UINT width, UINT height) : m_width(width), m_height(height)
{
	if (engine::core::Settings::UseAdvancedReflections())
	{
		m_isRenderDirty = true;
		SetDirty();
	}
	else
	{
		m_isRenderDirty = false;
		m_framesDirty = 0;
	}

	m_viewport.Height = static_cast<float>(m_height);
	m_viewport.Width = static_cast<float>(m_width);
	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;

	m_scissorRect.bottom = m_height;
	m_scissorRect.right = m_width;
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;

	m_cubeMapSphereRadius = 200.f;
}

void DynamicCubeMap::Create()
{
	m_cubeMapTexture.reset(new ColorTexture(engine::math::Color(0.f, 0.f, 0.f), true));
	m_cubeMapTexture->Create(
		m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, L"DynamicCubeMap");
	m_cubeMapTexture->AllocateRtvHandle();
	m_cubeMapTexture->AllocateSrvHandle();

	m_depthTexture.reset(new DepthTexture(1.0f, 0));
	m_depthTexture->Create(m_width, m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, L"DynamicCubeMapDepthTexture");
	m_depthTexture->AllocateDsvHandle();
}

void DynamicCubeMap::Setup(GraphicsContext& graphicsContext)
{
	graphicsContext.SetViewportAndScissor(m_viewport, m_scissorRect);
	graphicsContext.TransitionResource(*m_cubeMapTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void DynamicCubeMap::SetRenderTarget(GraphicsContext& graphicsContext, UINT index)
{
	graphicsContext.SetRenderTargetAndDepthStencil(
		m_cubeMapTexture->GetRtvHandle(index), m_depthTexture->GetDsvHandle());
	graphicsContext.ClearCubeMapFace(*m_cubeMapTexture, index);
	graphicsContext.ClearDepthAndStencil(*m_depthTexture);
}

void DynamicCubeMap::Finish(GraphicsContext& graphicsContext)
{
	graphicsContext.TransitionResource(*m_cubeMapTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void DynamicCubeMap::Update()
{
	m_cameraController.Update(m_center);

	m_isRenderDirty = true;
}

void DynamicCubeMap::SetCenter(engine::math::Vector3 center)
{
	m_center = center;

	SetDirty();
}

}  // namespace engine::gfx
