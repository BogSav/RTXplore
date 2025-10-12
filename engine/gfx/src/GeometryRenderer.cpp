#include "GeometryRenderer.hpp"

namespace engine::gfx
{

void GeometryRenderer::ReleaseUploadBuffers()
{
	// m_vertexBuffer->ReleaseUploadBuffer();
	// m_indexBuffer->ReleaseUploadBuffer();
}

ID3D12Resource* GeometryRenderer::GetBottomLevelAccelerationStructure()
{
	return m_bottomLevelAccelerationStructure.GetResource();
}

void GeometryRenderer::CreateVertexAndIndexBuffer(bool allocateSRVs)
{
	m_vertexBuffer.reset(new VertexBuffer());
	m_indexBuffer.reset(new IndexBuffer());

	m_vertexBuffer->Create(*m_mesh);
	m_indexBuffer->Create(*m_mesh);

	if (allocateSRVs)
	{
		m_vertexBuffer->AllocateSRV();
		m_indexBuffer->AllocateSRV();

		assert(
			m_indexBuffer->GetSRVHandle()
			== (m_vertexBuffer->GetSRVHandle()
				+ GraphicsResources::GetInstance().GetDescriptorIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));
	}
}

GeometryRenderer::~GeometryRenderer()
{
}

}  // namespace engine::gfx
