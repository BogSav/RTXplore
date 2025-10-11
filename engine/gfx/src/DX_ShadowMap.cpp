#include "DX_ShadowMap.hpp"

#include "engine/core/DxgiInfoManager.hpp"

ShadowMap::ShadowMap(UINT width, UINT height) : m_width(width), m_height(height)
{
	m_isRenderDirty = true;

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

	m_distanceFromCamera = 150.f;
	m_lightDirection = Math::Vector3(Math::kZero);

	m_camera = OrthograficCamera(
		Math::Vector3(0, 100, 0), Math::Vector3(Math::kZero), Math::Vector3(0, 0, 1), -200, 200, -200, 200, 400, 10);
}

void ShadowMap::Create()
{
	m_depthTexture.reset(new DepthTexture(1.f, 0));
	m_depthTexture->Create(m_width, m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, L"ShadowMap");
	m_depthTexture->AllocateDsvHandle();
	m_depthTexture->AllocateSrvHandle();
}

void ShadowMap::SetDirection(Math::Vector3 direction)
{
	m_lightDirection = direction;

	SetDirty();
}

void ShadowMap::Setup(GraphicsContext& graphicsContext)
{
	graphicsContext.SetViewportAndScissor(m_viewport, m_scissorRect);
	graphicsContext.TransitionResource(*m_depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	graphicsContext.SetDepthStencil(m_depthTexture->GetDsvHandle());
	graphicsContext.ClearDepth(*m_depthTexture);
}

void ShadowMap::Finish(GraphicsContext& graphicsContext)
{
	graphicsContext.TransitionResource(*m_depthTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void ShadowMap::Update()
{
	m_camera.SetViewCoordinateSystem(-m_lightDirection * m_distanceFromCamera, Math::Vector3(Math::kZero));

	m_isRenderDirty = true;
}
