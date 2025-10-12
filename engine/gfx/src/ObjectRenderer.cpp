#include "ObjectRenderer.hpp"

#include "FrameResources.hpp"
#include "GeometryGenerator.hpp"

#include <map>

using namespace misc::rasterization;
using namespace misc::render_descriptors;

ObjectRenderer::Ptr ObjectRenderer::CreateObjectRenderer(DX_OBJECTS_RENDERER_DESCRIPTOR descriptor)
{
	ObjectRenderer::Ptr objectRenderer = Ptr(new ObjectRenderer());

	objectRenderer->m_basePSO = descriptor.basePSO;
	objectRenderer->m_baseToplogy = descriptor.baseToplogy;

	objectRenderer->m_shadowPSO = descriptor.shadowPSO;
	objectRenderer->m_shadowTopology = descriptor.shadowTopology;

	objectRenderer->m_dynamicCubeMapPSO = descriptor.dynamicCubeMapPSO;
	objectRenderer->m_dynamicCubeMapTopology = descriptor.dynamicCubeMapToplogy;

	objectRenderer->m_shadowDebugPSO = descriptor.debugShadowPSO;

	objectRenderer->LoadGeometry(descriptor);
	objectRenderer->CreateObjects(descriptor);

	return objectRenderer;
}

ObjectRenderer::~ObjectRenderer()
{
	OutputDebugStringA("Object Renderer Destroyed");
}

void ObjectRenderer::LoadGeometry(DescriptorVariant descriptor)
{
	std::map<std::string, Mesh::Ptr> meshes;
	meshes["6FacesCube"] = GeometryGenerator::GenerateCube(2.f);
	meshes["Cube"] = GeometryGenerator::GenerateSimpleCube(2.f);
	meshes["Cylinder"] = GeometryGenerator::GenerateCylinder(1.f, 1.f, 2.f, 5, 20);
	meshes["Grid"] = GeometryGenerator::GenerateGrid(2.f, 2.f, 2, 2);
	meshes["Sphere"] = GeometryGenerator::GenerateGeoSphere(2.f, {0.f, 0.f, 0.f}, 3);
	meshes["Quad"] = GeometryGenerator::GenerateSimpleQuad(2.f);

	UINT vertexOffset = 0;
	UINT indexOffset = 0;

	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	for (auto const& [name, meshPtr] : meshes)
	{
		SubMesh subMesh;
		subMesh.baseVertexLocation = vertexOffset;
		subMesh.indexCount = meshPtr->GetIndexCount();
		subMesh.startIndexLocation = indexOffset;

		m_subMeshes[name] = subMesh;
		m_boundingBoxes[name] = meshPtr->GetAABB();

		vertexOffset += (UINT)meshPtr->GetVertexCount();
		indexOffset += (UINT)meshPtr->GetIndexCount();

		vertices.insert(vertices.end(), meshPtr->GetVertexVector().cbegin(), meshPtr->GetVertexVector().cend());
		indices.insert(indices.end(), meshPtr->GetIndexVector().cbegin(), meshPtr->GetIndexVector().cend());
	}

	m_mesh = std::make_shared<Mesh>(vertices, indices);

	CreateVertexAndIndexBuffer();
}

void ObjectRenderer::CreateObjects(const DX_OBJECTS_RENDERER_DESCRIPTOR& desc)
{
	for (size_t i = 0; i < 4; i++)
	{
		const DX_OBJECT_DESCRIPTOR& objectDesc = desc.objectDescriptors[i];
		Object::Ptr aux = std::make_shared<Object>(
			objectDesc, m_subMeshes[objectDesc.subGeometryName], m_boundingBoxes[objectDesc.subGeometryName]);

		m_objects.push_back(aux);
	}

	// Obiect pentru shadow debug - un quad simplu pe ccare este randata textura
	{
		const DX_OBJECT_DESCRIPTOR& objectDesc = desc.objectDescriptors[desc.objectDescriptors.size() - 1];
		Object::Ptr aux = std::make_shared<Object>(
			objectDesc, m_subMeshes[objectDesc.subGeometryName], m_boundingBoxes[objectDesc.subGeometryName]);
		aux->SetVisible(false);

		m_objects.push_back(aux);
	}
}

void ObjectRenderer::BuildAccelerationStructures()
{
}

void ObjectRenderer::Update(float deltaTime)
{
	for (auto& object : m_objects)
	{
		object->Update(deltaTime);
	}
}

void ObjectRenderer::Render(misc::rasterization::RenderLayer::Value renderLayer) const
{
	GraphicsContext& graphicsContext = GraphicsResources::GetInstance().GetGraphicsContext();
	FrameResources& frameResources = GraphicsResources::GetInstance().GetFrameResources();

	graphicsContext.SetVertexBuffer(0, m_vertexBuffer->GetVertexBufferView());
	graphicsContext.SetIndexBuffer(m_indexBuffer->GetIndexBufferView());

	const auto renderObjects = [this, &renderLayer](const bool performVisbilityTest = true)
	{
		for (auto& object : m_objects)
		{
			if (performVisbilityTest && !object->IsVisible())
				continue;

			object->Render(renderLayer);
		}
	};

	switch (renderLayer)
	{
	case misc::rasterization::RenderLayer::Base:
		graphicsContext.SetPipelineState(*m_basePSO);

		renderObjects();

		break;
	case RenderLayer::CubeMap:
		graphicsContext.SetPipelineState(*m_dynamicCubeMapPSO);

		renderObjects();

		break;
	case misc::rasterization::RenderLayer::ShadowMap:
		graphicsContext.SetPipelineState(*m_shadowPSO);

		renderObjects();

		break;
	// case misc::rasterization::RenderLayer::DebugShadowMap:
	//	pCommandList->SetGraphicsRootSignature(m_shadowDebugPSO->GetID3D12RootSignature());
	//	pCommandList->SetPipelineState(m_shadowDebugPSO->GetD3D12PipelineState());

	//	pCommandList->IASetPrimitiveTopology(m_baseToplogy);
	//	pCommandList->IASetVertexBuffers(0, 1, &m_vertexBuffer->GetVertexBufferView());
	//	pCommandList->IASetIndexBuffer(&m_indexBuffer->GetIndexBufferView());

	//	pCommandList->SetGraphicsRootConstantBufferView(
	//		DefaultRSBindings::PassCB, pFrameResources->m_perPassCB.GetGpuVirtualAdress(0));

	//	(*std::prev(m_objects.end()))->Render(pGraphicsResources, renderLayer);

	//	break;
	default: throw misc::CustomException("Tip de randare inexistent!!");
	}
}

void ObjectRenderer::FrustumCulling(const CameraController& cameraController)
{
	for (auto& object : m_objects)
	{
		object->SetVisible(cameraController.GetWorldSpaceFrustum().IntersectBoundingBox(object->GetWorldSpaceAABB()));
	}
}