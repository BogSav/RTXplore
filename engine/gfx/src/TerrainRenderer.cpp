#include "TerrainRenderer.hpp"

#include "Utilities.hpp"
#include "GeometryGenerator.hpp"
#include "engine/math/SimplexNoise.hpp"

using namespace misc::RSBinding;
using namespace misc::rasterization;
using namespace misc::render_descriptors;

TerrainRenderer::Ptr TerrainRenderer::CreateTerrainRenderer(DX_TERRAIN_DESCRIPTOR& descriptor)
{
	TerrainRenderer::Ptr terrain = Ptr(new TerrainRenderer(descriptor.objectDescriptor));

	terrain->m_shadowTopology = descriptor.cubeShadowToplogy;
	terrain->m_shadowPSO = descriptor.shadowPSO;

	terrain->m_baseToplogy = descriptor.baseToplogy;
	terrain->m_basePSO = descriptor.basePSO;

	terrain->m_dynamicCubeMapTopology = descriptor.cubeShadowToplogy;
	terrain->m_dynamicCubeMapPSO = descriptor.cubePSO;

	terrain->LoadGeometry(descriptor);

	return terrain;
}

TerrainRenderer::TerrainRenderer(const misc::render_descriptors::DX_OBJECT_DESCRIPTOR& objectDesc)
	: Object(objectDesc), GeometryRenderer()
{
}

void TerrainRenderer::LoadGeometry(DescriptorVariant descriptor)
{
	////////////////////////////////////////////////
	// Conventia de coordonate raportate la lungime si latime
	// Width - Z
	// Length - X
	///////////////////////////////////////////////
	DX_TERRAIN_DESCRIPTOR& terrainDesc = std::get<DX_TERRAIN_DESCRIPTOR>(descriptor);

	engine::math::SimplexNoise flatHillNoise(
		terrainDesc.simplexProperties.frequency,
		terrainDesc.simplexProperties.amplitude,
		terrainDesc.simplexProperties.lacunarity,
		terrainDesc.simplexProperties.persistence);

	const auto heightFunction = [&flatHillNoise, &terrainDesc](float x, float z) -> float
	{
		float amplitudeFactor = terrainDesc.simplexProperties.amplitudeFactor;

		if (z > 0)
		{
			amplitudeFactor += z / 1.5f;
		}

		return amplitudeFactor * flatHillNoise.fractal(terrainDesc.simplexProperties.octaveCount, x, z);
	};

	std::vector<engine::math::AABB> aabbs;
	std::vector<SubMesh> submeshs;

	m_mesh = GeometryGenerator::GenerateChunks(
		aabbs,
		submeshs,
		heightFunction,
		terrainDesc.width,
		terrainDesc.length,
		terrainDesc.chunkKernelSize,
		terrainDesc.chunkCountPerSide);

	GeometryHelper::ChnageColor(m_mesh, engine::math::Vector4(1, 0, 0, 0));

	for (int i = 0; i < submeshs.size(); i++)
	{
		m_chunks.emplace_back(submeshs[i], aabbs[i]);
	}

	CreateVertexAndIndexBuffer(Settings::UseRayTracing());

	/*D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
	pGraphicsResources->GetDevice()->CreateUnorderedAccessView(
		m_vertexBuffer->GetVertexBufferResource(),
		nullptr,
		&desc,
		pGraphicsResources->GetCbvSrvUavDescriptorHeap()->GetCPUDescriptorHandleForHeapStart());*/
}

void TerrainRenderer::Render(misc::rasterization::RenderLayer::Value renderLayer) const
{
	GraphicsContext& graphicsContext = GraphicsResources::GetInstance().GetGraphicsContext();
	FrameResources& frameResources = GraphicsResources::GetInstance().GetFrameResources();

	graphicsContext.SetVertexBuffer(0, m_vertexBuffer->GetVertexBufferView());
	graphicsContext.SetIndexBuffer(m_indexBuffer->GetIndexBufferView());

	graphicsContext.SetConstantBuffer(
		DefaultRSBindings::ObjectCB, frameResources.m_perObjectCB.GetGpuVirtualAdress(GetObjectCB_ID()));
	graphicsContext.SetConstantBuffer(
		DefaultRSBindings::MaterialCB, frameResources.m_perMaterialCB.GetGpuVirtualAdress(GetMaterialCB_ID()));

	const auto renderChunks = [this, &graphicsContext](const bool performVisbilityTest = true)
	{
		for (const auto& chunk : m_chunks)
		{
			if (performVisbilityTest && !chunk.IsVisible())
				continue;

			const SubMesh& subMesh = chunk.GetSubMesh();

			graphicsContext.DrawIndexed(
				(UINT)subMesh.indexCount, (UINT)subMesh.startIndexLocation, (UINT)subMesh.baseVertexLocation);
		}
	};

	switch (renderLayer)
	{
	case RenderLayer::Base:
		graphicsContext.SetPipelineState(*m_basePSO);

		renderChunks(false);

		break;
	case RenderLayer::CubeMap:
		graphicsContext.SetPipelineState(*m_dynamicCubeMapPSO);

		renderChunks(false);

		break;
	case RenderLayer::ShadowMap:
		graphicsContext.SetPipelineState(*m_shadowPSO);

		renderChunks(false);

		break;
	case RenderLayer::DebugShadowMap:
	default: throw misc::customException("Tip de randare inexistent!!");
	}
}

void TerrainRenderer::Update(float deltaTime)
{
	Object::Update(deltaTime);
}

void TerrainRenderer::BuildAccelerationStructures()
{
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDescs;

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geometryDesc.Triangles.IndexBuffer = m_indexBuffer->GetIndexBufferView().BufferLocation;
	geometryDesc.Triangles.IndexCount = (UINT)m_mesh->GetIndexCount();
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexCount = (UINT)m_mesh->GetVertexCount();
	geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer->GetVertexBufferView().BufferLocation;
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = Mesh::GetSizeOfVertex();

	geomDescs.push_back(geometryDesc);

	m_bottomLevelAccelerationStructure.Build(
		geomDescs, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);
}

void TerrainRenderer::FrustumCulling(const CameraController& cameraController)
{
	const float zFarSq = std::pow(cameraController.GetCamera().GetZFar(), 2.f);

	for (auto& chunk : m_chunks)
	{
		chunk.SetVisible(
			(chunk.GetAABB().GetCenter() - cameraController.GetCamera().GetPosition()).Length2() < zFarSq
			&& cameraController.GetWorldSpaceFrustum().IntersectBoundingBox(chunk.GetAABB()));
	}
}