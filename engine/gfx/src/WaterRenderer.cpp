#include "WaterRenderer.hpp"

#include "GeometryGenerator.hpp"

namespace engine::gfx
{

using namespace engine::gfx::rasterization;
using namespace engine::gfx::render_descriptors;
using namespace engine::gfx::RSBinding;

WaterRenderer::Ptr WaterRenderer::CreateWaterRenderer(engine::gfx::render_descriptors::DX_WATER_DESCRIPTOR& descriptor)
{
	WaterRenderer::Ptr water = Ptr(new WaterRenderer(descriptor.objectDescriptor));

	water->m_basePSO = descriptor.basePSO;
	water->m_baseToplogy = descriptor.baseToplogy;

	water->m_shadowPSO = nullptr;
	water->m_dynamicCubeMapPSO = nullptr;

	water->m_waveProperties = descriptor.waveProperties;

	water->m_textureDirection = descriptor.textureDirection;
	water->m_textureMoveSpeed = descriptor.textureMoveSpeed;
	water->m_textureScale = descriptor.textureScale;

	water->m_texturePosition = engine::math::Vector3(engine::math::kOrigin);
	water->m_cubeMapCenter = engine::math::Vector3(engine::math::kOrigin);
	water->m_cubeMapSphereRadius = descriptor.cubeMapSphereRadius;

	water->LoadGeometry(descriptor);

	return water;
}

WaterRenderer::WaterRenderer(const DX_OBJECT_DESCRIPTOR& objectDesc) : Object(objectDesc), GeometryRenderer()
{
}

WaterRenderer::~WaterRenderer()
{
	OutputDebugStringA("Water Renderere Destroyed");
}

void WaterRenderer::LoadGeometry(DescriptorVariant descriptor)
{
	DX_WATER_DESCRIPTOR& waterDesc = std::get<DX_WATER_DESCRIPTOR>(descriptor);

	if (!engine::core::Settings::UseRayTracing())
	{
		std::vector<engine::math::AABB> aabbs;
		std::vector<SubMesh> submeshs;

		m_mesh = GeometryGenerator::GenerateChunks(
			aabbs,
			submeshs,
			[](float, float) -> float { return 0.f; },
			waterDesc.width,
			waterDesc.length,
			waterDesc.chunkKernelSize,
			waterDesc.chunkCountPerSide);

		for (int i = 0; i < submeshs.size(); i++)
		{
			m_chunks.emplace_back(submeshs[i], aabbs[i]);
		}

		// GeometryHelper::ChnageColor(m_mesh, GetColor());

		CreateVertexAndIndexBuffer(engine::core::Settings::UseRayTracing());
	}
	else
	{
		const auto convertMathAABBtoRTAABB = [](const engine::math::AABB& aabb)
		{
			D3D12_RAYTRACING_AABB rtaabb;

			rtaabb.MaxX = (float)aabb.GetMaxX();
			rtaabb.MaxY = (float)aabb.GetMaxY();
			rtaabb.MaxZ = (float)aabb.GetMaxZ();

			rtaabb.MinX = (float)aabb.GetMinX();
			rtaabb.MinY = (float)aabb.GetMinY();
			rtaabb.MinZ = (float)aabb.GetMinZ();

			return rtaabb;
		};

		engine::math::AABB aabb;

		aabb.SetCorners(
			engine::math::Vector3(-waterDesc.width / 2., -4, -waterDesc.length / 2),
			engine::math::Vector3(waterDesc.width / 2, 4, waterDesc.length / 2));

		m_AABBs.push_back(convertMathAABBtoRTAABB(aabb));

		GpuResource::AllocateUploadBuffer(
			m_AABBs.data(), m_AABBs.size() * sizeof(m_AABBs[0]), m_AABBsResource, L"AABB_RT_Resource");
	}
}

void WaterRenderer::Update(float deltaTime)
{
	using namespace engine::math;

	m_texturePosition += m_textureDirection * deltaTime * m_textureMoveSpeed;

	Object::SetTextureTransform(Matrix4::MakeScale(m_textureScale) * Matrix4::MakeTranslation(m_texturePosition));
	Object::Update(deltaTime);
}

void WaterRenderer::FrustumCulling(const CameraController& cameraController)
{
	const float zFarSq = std::pow(cameraController.GetCamera().GetZFar(), 2.f);

	for (auto& chunk : m_chunks)
	{
		chunk.SetVisible(
			(chunk.GetAABB().GetCenter() - cameraController.GetCamera().GetPosition()).Length2() < zFarSq
			&& cameraController.GetWorldSpaceFrustum().IntersectBoundingBox(chunk.GetAABB()));
	}
}

void WaterRenderer::Render(engine::gfx::rasterization::RenderLayer::Value renderLayer) const
{
	GraphicsContext& graphicsContext = GraphicsResources::GetInstance().GetGraphicsContext();
	FrameResources& frameResources = GraphicsResources::GetInstance().GetFrameResources();

	const auto renderChunks = [this, &graphicsContext]
	{
		for (const auto& chunk : m_chunks)
		{
			if (!chunk.IsVisible())
				continue;

			const SubMesh& subMesh = chunk.GetSubMesh();

			graphicsContext.DrawIndexed(
				(UINT)subMesh.indexCount, (UINT)subMesh.startIndexLocation, (UINT)subMesh.baseVertexLocation);
		}
	};

	switch (renderLayer)
	{
	case RenderLayer::Base:
		graphicsContext.SetRootSignature(m_basePSO->GetRootSiganture());
		graphicsContext.SetPipelineState(*m_basePSO);

		graphicsContext.SetPrimitiveTopology(m_baseToplogy);

		graphicsContext.SetVertexBuffer(0, m_vertexBuffer->GetVertexBufferView());
		graphicsContext.SetIndexBuffer(m_indexBuffer->GetIndexBufferView());

		graphicsContext.BindDescriptorHeaps();

		graphicsContext.SetConstantBuffer(WaterRSParams::PassCB, frameResources.m_perPassCB.GetGpuVirtualAdress(0));
		graphicsContext.SetConstantBuffer(
			WaterRSParams::ObjectCB, frameResources.m_perObjectCB.GetGpuVirtualAdress(GetObjectCB_ID()));
		graphicsContext.SetConstantBuffer(
			WaterRSParams::MaterialCB, frameResources.m_perMaterialCB.GetGpuVirtualAdress(GetMaterialCB_ID()));
		graphicsContext.SetConstantBuffer(WaterRSParams::WaterCB, frameResources.m_waterCB.GetGpuVirtualAdress());

		graphicsContext.SetDescriptorTable(WaterRSParams::StaticPixelTextures, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		graphicsContext.SetDescriptorTable(
			WaterRSParams::StaticNonPixelTextures, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		graphicsContext.SetDescriptorTable(WaterRSParams::DynamicTextures, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		renderChunks();

		break;
	case RenderLayer::CubeMap:
	case RenderLayer::ShadowMap:
	case RenderLayer::DebugShadowMap: throw engine::core::CustomException("Tip de randare nesuportat");
	default: throw engine::core::CustomException("Tip de randare inexistent!!");
	}
}

void WaterRenderer::BuildAccelerationStructures()
{
	GraphicsContext& graphicsContext = GraphicsResources::GetInstance().GetGraphicsContext();
	ID3D12Device10* pDevice = GraphicsResources::GetDevice();

	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDescs;

	{
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
		geometryDesc.Flags =
			D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE | D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;
		geometryDesc.AABBs.AABBCount = (UINT)m_AABBs.size();
		geometryDesc.AABBs.AABBs.StartAddress = m_AABBsResource->GetGPUVirtualAddress();
		geometryDesc.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);

		geomDescs.push_back(geometryDesc);
	}

	m_bottomLevelAccelerationStructure.Build(
		geomDescs, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);
}

}  // namespace engine::gfx
