#include "TextureRenderer.hpp"
#include "GeometryGenerator.hpp"

namespace engine::gfx
{

using namespace engine::gfx::RSBinding;
using namespace engine::gfx::rasterization;

TextureRenderer::Ptr TextureRenderer::CreateTextureRenderer(engine::gfx::render_descriptors::DX_TEXTURE_DESCRIPTOR& descriptor)
{
	TextureRenderer::Ptr textureRenderer = Ptr(new TextureRenderer());

	textureRenderer->m_shadowTopology = descriptor.baseTopology;
	textureRenderer->m_shadowPSO = descriptor.basePSO;

	textureRenderer->m_baseToplogy = descriptor.baseTopology;
	textureRenderer->m_basePSO = descriptor.basePSO;

	textureRenderer->m_dynamicCubeMapTopology = descriptor.baseTopology;
	textureRenderer->m_dynamicCubeMapPSO = descriptor.basePSO;

	textureRenderer->m_textureSRVHandle = descriptor.textureSRVHandle;

	textureRenderer->LoadGeometry(descriptor);

	return textureRenderer;
}

void TextureRenderer::LoadGeometry(DescriptorVariant descriptor)
{
	using namespace engine::gfx::render_descriptors;

	DX_TEXTURE_DESCRIPTOR& textureRenderereDescriptor = std::get<DX_TEXTURE_DESCRIPTOR>(descriptor);

	m_mesh = GeometryGenerator::GenerateSimpleQuad(2.f);

	CreateVertexAndIndexBuffer();
}

void TextureRenderer::Render(engine::gfx::rasterization::RenderLayer::Value renderLayer) const
{
	GraphicsContext& graphicsContext = GraphicsResources::GetInstance().GetGraphicsContext();

	graphicsContext.SetRootSignature(m_basePSO->GetRootSiganture());
	graphicsContext.SetPipelineState(*m_basePSO);

	graphicsContext.SetVertexBuffer(0, m_vertexBuffer->GetVertexBufferView());
	graphicsContext.SetIndexBuffer(m_indexBuffer->GetIndexBufferView());
	graphicsContext.SetPrimitiveTopology(m_baseToplogy);

	graphicsContext.SetDescriptorTable(0, m_textureSRVHandle);

	graphicsContext.Draw(m_mesh->GetVertexCount());
}

void TextureRenderer::BuildAccelerationStructures()
{
}

void TextureRenderer::FrustumCulling(const CameraController& cameraController)
{
}

void TextureRenderer::Update(float deltaTime)
{
}

void TextureRenderer::SetTextureSRVHandle(const engine::gfx::DescriptorHandle& textureSRVHandle)
{
	m_textureSRVHandle = textureSRVHandle;
}

}  // namespace engine::gfx
