#include "SkyBoxRenderer.hpp"

#include "GeometryGenerator.hpp"

using namespace engine::gfx::rasterization;
using namespace engine::gfx::RSBinding;
using namespace engine::gfx::render_descriptors;

SkyBoxRenderer::Ptr SkyBoxRenderer::CreateSkyBoxRenderer(engine::gfx::render_descriptors::DX_SKYBOX_DESCRIPTOR& descriptor)
{
	SkyBoxRenderer::Ptr skyBox = Ptr(new SkyBoxRenderer(descriptor.objectDescriptor));


	skyBox->m_basePSO = descriptor.basePSO;
	skyBox->m_baseToplogy = descriptor.baseToplogy;

	skyBox->m_dynamicCubeMapPSO = descriptor.dynamicCubeMapPSO;
	skyBox->m_dynamicCubeMapTopology = descriptor.dynamicCubeMapToplogy;

	skyBox->m_rotationAxis = descriptor.rotationAxis;
	skyBox->m_rotationSpeed = descriptor.rotationSpeed;
	skyBox->m_lightDirection = descriptor.lightDirection;

	skyBox->LoadGeometry(descriptor);

	return skyBox;
}

SkyBoxRenderer::SkyBoxRenderer(engine::gfx::render_descriptors::DX_OBJECT_DESCRIPTOR& objectDesc)
	: Object(objectDesc), GeometryRenderer()
{
}

void SkyBoxRenderer::LoadGeometry(DescriptorVariant descriptor)
{
	m_mesh = GeometryGenerator::GenerateCube(2.f);

	CreateVertexAndIndexBuffer();
}

void SkyBoxRenderer::Render(engine::gfx::rasterization::RenderLayer::Value renderLayer) const
{
	GraphicsContext& graphicsContext = GraphicsResources::GetInstance().GetGraphicsContext();
	FrameResources& frameResources = GraphicsResources::GetInstance().GetFrameResources();

	graphicsContext.SetVertexBuffer(0, m_vertexBuffer->GetVertexBufferView());
	graphicsContext.SetIndexBuffer(m_indexBuffer->GetIndexBufferView());

	graphicsContext.SetConstantBuffer(
		DefaultRSBindings::ObjectCB, frameResources.m_perObjectCB.GetGpuVirtualAdress(GetObjectCB_ID()));

	switch (renderLayer)
	{
	case engine::gfx::rasterization::RenderLayer::Base:
		graphicsContext.SetPipelineState(*m_basePSO);
		graphicsContext.DrawIndexed((UINT)m_mesh->GetIndexCount());

		break;
	case RenderLayer::CubeMap:
		graphicsContext.SetPipelineState(*m_dynamicCubeMapPSO);
		graphicsContext.DrawIndexed((UINT)m_mesh->GetIndexCount());

		break;
	case engine::gfx::rasterization::RenderLayer::ShadowMap:
	case engine::gfx::rasterization::RenderLayer::DebugShadowMap:
	default: throw engine::core::CustomException("Tip de randare inexistent!!");
	}
}

void SkyBoxRenderer::Update(float deltaTime)
{
	using namespace engine::math;

	Quaternion rot(m_rotationAxis, Scalar(m_rotationSpeed * deltaTime));

	m_lightDirection = rot * m_lightDirection;

	Object::SetTextureTransform(GetTextureTransform() * Matrix4::MakeMatrixRotationQuaternion(rot));
	Object::Update(deltaTime);
}

SkyBoxRenderer::~SkyBoxRenderer()
{
	OutputDebugString(L"SkyBox renderere destroied");
}

void SkyBoxRenderer::BuildAccelerationStructures()
{
	return;
}

void SkyBoxRenderer::FrustumCulling(const CameraController& cameraController)
{
	return;
}
