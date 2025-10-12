#pragma once

#include "CameraController.hpp"
#include "AccelerationStructures.hpp"
#include "PSO.hpp"
#include "Utilities.hpp"
#include "GraphicsResources.hpp"
#include "engine/math/Frustum.hpp"
#include "Mesh.hpp"

#include <variant>

class GeometryRenderer
{
public:
	virtual void Render(engine::gfx::rasterization::RenderLayer::Value renderLayer) const = 0;
	virtual void BuildAccelerationStructures() = 0;
	virtual void FrustumCulling(const CameraController& cameraController) = 0;
	virtual void Update(float deltaTime) = 0;

	const VertexBuffer& GetVertexBuffer() const { return *m_vertexBuffer; }
	const IndexBuffer& GetIndexBuffer() const { return *m_indexBuffer; }

	ID3D12Resource* GetBottomLevelAccelerationStructure();

protected:
	class Chunk
	{
	public:
		using ChunkVec = std::vector<Chunk>;

		Chunk() = delete;
		Chunk(const SubMesh& subMesh, const engine::math::AABB& aabb)
			: m_subMesh(subMesh), m_boundingBox(aabb), m_isVisible(true)
		{
		}

		inline const engine::math::AABB& GetAABB() const { return m_boundingBox; }
		inline const SubMesh& GetSubMesh() const { return m_subMesh; }
		inline const bool IsVisible() const { return m_isVisible; }

		void SetVisible(bool toSet) { m_isVisible = toSet; }

	private:
		bool m_isVisible;

		const engine::math::AABB m_boundingBox;
		const SubMesh m_subMesh;
	};

	using DescriptorVariant = std::variant<
		engine::gfx::render_descriptors::DX_OBJECTS_RENDERER_DESCRIPTOR,
		engine::gfx::render_descriptors::DX_SKYBOX_DESCRIPTOR,
		engine::gfx::render_descriptors::DX_TERRAIN_DESCRIPTOR,
		engine::gfx::render_descriptors::DX_WATER_DESCRIPTOR,
		engine::gfx::render_descriptors::DX_TEXTURE_DESCRIPTOR>;

protected:
	GeometryRenderer() = default;
	virtual ~GeometryRenderer();

	virtual void LoadGeometry(DescriptorVariant descriptor) = 0;
	void CreateVertexAndIndexBuffer(bool allocateSRVs = false);
	void ReleaseUploadBuffers();

protected:
	Mesh::Ptr m_mesh;

	IndexBuffer::Ptr m_indexBuffer;
	VertexBuffer::Ptr m_vertexBuffer;

	GraphicsPSO::Ptr m_basePSO;
	D3D12_PRIMITIVE_TOPOLOGY m_baseToplogy;

	GraphicsPSO::Ptr m_shadowPSO;
	D3D12_PRIMITIVE_TOPOLOGY m_shadowTopology;

	GraphicsPSO::Ptr m_dynamicCubeMapPSO;
	D3D12_PRIMITIVE_TOPOLOGY m_dynamicCubeMapTopology;

	BottomLevelAccelerationStructure m_bottomLevelAccelerationStructure;
};