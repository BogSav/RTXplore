#include "RasterizationGraphics.hpp"

#include "engine/math/Frustum.hpp"

using namespace Microsoft::WRL;
using namespace engine::gfx::RSBinding;
using namespace engine::gfx::render_descriptors;
using namespace engine::gfx::rasterization;

RasterizationGraphics::RasterizationGraphics(GraphicsResources& graphicsResorurces) : Graphics(graphicsResorurces)
{
	GraphicsContext& graphicsContext = m_graphicsResources.GetGraphicsContext();
	graphicsContext.Reset();

	// Compilare shadere, setare RS-uri si PSO-uri, creare materiale
	m_graphicsPSOsManager.LoadPSOs(m_RSManager, m_shaderManager, GraphicsResources::GetDevice());
	m_materialManager.LoadDefaultMaterials();

	// Incarcare texturi
	m_textureManager.LoadTexturesFormFiles(GraphicsResources::GetDevice(), GraphicsResources::GetCommandQueue());
	m_textureManager.CreateSRVHandles();
	m_textureManager.SwapNonPixelShaderTextures(graphicsContext);

	if (engine::core::Settings::UseAdvancedReflections())
	{
		m_dynamicCubeMap.reset(new DynamicCubeMap(1024u, 1024u));
		m_dynamicCubeMap->Create();
	}

	if (engine::core::Settings::UseShadows())
	{
		m_shadowMap.reset(new ShadowMap(8192u, 8192u));
		m_shadowMap->Create();
	}

	{
		DX_TERRAIN_DESCRIPTOR desc;
		DX_OBJECT_DESCRIPTOR& objectDesc = desc.objectDescriptor;

		objectDesc.objectCB_ID = FrameResources::GetObjectCB_ID();
		objectDesc.textureTransform = engine::math::Matrix4::MakeScale(engine::math::Vector3(40.f, 40.f, 1));
		objectDesc.materialCB_ID = m_materialManager.GetMaterialCB_ID("Default");
		objectDesc.isStatic = true;

		desc.basePSO = m_graphicsPSOsManager.GetPSO("Terrain");
		desc.baseToplogy = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

		desc.shadowPSO = m_graphicsPSOsManager.GetPSO("ShadowMap");
		desc.cubePSO = m_graphicsPSOsManager.GetPSO("TerrainCubeMap");
		desc.cubeShadowToplogy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		desc.length = 400.f;
		desc.width = 400.f;
		desc.chunkCountPerSide = 16;
		desc.chunkKernelSize = 10;

		desc.simplexProperties.frequency = 0.006f;
		desc.simplexProperties.amplitude = 10.f;
		desc.simplexProperties.lacunarity = 2.2f;
		desc.simplexProperties.persistence = 0.5f;
		desc.simplexProperties.octaveCount = 5;
		desc.simplexProperties.amplitudeFactor = 30.f;

		m_terrainRender = TerrainRenderer::CreateTerrainRenderer(desc);
	}

	{
		DX_WATER_DESCRIPTOR desc;
		DX_OBJECT_DESCRIPTOR& objectDesc = desc.objectDescriptor;

		objectDesc.objectCB_ID = FrameResources::GetObjectCB_ID();
		objectDesc.materialCB_ID = m_materialManager.GetMaterialCB_ID("Water");
		objectDesc.color = engine::math::Vector4(50 / 255.f, 77 / 255.f, 80 / 255.f, 0.6f);
		objectDesc.isStatic = true;

		desc.basePSO = m_graphicsPSOsManager.GetPSO("Water");
		desc.baseToplogy = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

		desc.waveProperties[0].amplitude = 0.7f;  // 0.7
		desc.waveProperties[0].speed = 0.3f;
		desc.waveProperties[0].wavelength = 8.f;
		desc.waveProperties[0].direction = DirectX::XMFLOAT2(-1.f, 0.4f);
		desc.waveProperties[0].steepness = 0.4f;

		desc.waveProperties[1].amplitude = 1.4f;  // 1.4
		desc.waveProperties[1].speed = 0.9f;
		desc.waveProperties[1].wavelength = 10.f;
		desc.waveProperties[1].direction = DirectX::XMFLOAT2(0.3f, -1.f);
		desc.waveProperties[1].steepness = 0.9f;

		desc.length = 400.f;
		desc.width = 400.f;
		desc.chunkKernelSize = 10;
		desc.chunkCountPerSide = 20;

		desc.textureScale = engine::math::Vector3(10, 10, 10);
		desc.textureDirection = engine::math::Vector3(0.3f, -1.f, 0.45f);
		desc.textureMoveSpeed = 0.02f;
		desc.cubeMapSphereRadius = 400.f;

		m_waterRenderer = WaterRenderer::CreateWaterRenderer(desc);
	}

	{
		DX_SKYBOX_DESCRIPTOR desc;
		DX_OBJECT_DESCRIPTOR& objectDesc = desc.objectDescriptor;

		objectDesc.objectCB_ID = FrameResources::GetObjectCB_ID();
		objectDesc.isStatic = false;

		desc.basePSO = m_graphicsPSOsManager.GetPSO("SkyBox");
		desc.baseToplogy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		desc.dynamicCubeMapPSO = m_graphicsPSOsManager.GetPSO("SkyBox");
		desc.dynamicCubeMapToplogy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		desc.rotationAxis = engine::math::Vector3(0, -1, 0);
		desc.rotationSpeed = 0.02f;

		constexpr float angle = DirectX::XMConvertToRadians(55.f);
		desc.lightDirection = engine::math::Vector3(-cosf(angle), -sinf(angle), 0.f);

		m_skyBoxRenderer = SkyBoxRenderer::CreateSkyBoxRenderer(desc);
	}

	{
		DX_TEXTURE_DESCRIPTOR desc;

		desc.basePSO = m_graphicsPSOsManager.GetPSO("Texture");
		desc.baseTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		desc.textureSRVHandle = m_shadowMap->GetTextureSRVHandle();

		m_textureRenderer = TextureRenderer::CreateTextureRenderer(desc);
	}

	{
		DX_OBJECTS_RENDERER_DESCRIPTOR desc;

		for (size_t i = 0; i < 4; i++)
		{
			DX_OBJECT_DESCRIPTOR objectDesc;
			objectDesc.subGeometryName = "Sphere";
			objectDesc.scale = engine::math::Vector3(2.f, 2.f, 2.f);
			objectDesc.position = engine::math::Vector3(20.f, 10.f, 20.f);
			objectDesc.objectCB_ID = FrameResources::GetObjectCB_ID();
			objectDesc.color = engine::math::Vector4(1, 0, 0, 1);
			objectDesc.isStatic = false;

			desc.objectDescriptors.push_back(objectDesc);
		}

		{
			DX_OBJECT_DESCRIPTOR objectDesc;
			objectDesc.subGeometryName = "Quad";
			objectDesc.objectCB_ID = FrameResources::GetObjectCB_ID();
			objectDesc.isStatic = true;

			desc.objectDescriptors.push_back(objectDesc);
		}

		desc.basePSO = m_graphicsPSOsManager.GetPSO("Default");
		desc.baseToplogy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		desc.dynamicCubeMapPSO = m_graphicsPSOsManager.GetPSO("Default");
		desc.dynamicCubeMapToplogy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		desc.shadowPSO = m_graphicsPSOsManager.GetPSO("ShadowMap");
		desc.shadowTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// desc.debugShadowPSO = m_graphicsPSOsManager.GetPSO("Debug");

		m_objectRenderer = ObjectRenderer::CreateObjectRenderer(desc);
	}

	m_dynamicCubeMap->SetCenter(m_waterRenderer->GetCubeMapCenter());
	m_shadowMap->SetDirection(m_skyBoxRenderer->GetLightDirection());

	LoadAssets();

	m_shaderManager.ClearShaders();

	GraphicsResources::GetContextManager().Flush(true);
}

RasterizationGraphics::~RasterizationGraphics()
{
	OutputDebugString(L"Rasterization graphics object fully destroyed");
}

void RasterizationGraphics::LoadAssets()
{
	// Setare CB static pentru obiectele static din scena: teren, apa
	for (UINT i = 0; i < engine::core::Settings::GetFrameResourcesCount(); i++)
	{
		m_graphicsResources.GetFrameResources(i).UpdateTerrainCB(*m_terrainRender);
	}

	m_dynamicGameComponents = m_objectRenderer->GetObjects();

	/*for (size_t i = 0; i < 4; i++)
	{
		LightSource lightSource(LightSource::Type::SPOT_LIGHT);

		lightSource.m_lightProperties.Position = m_dynamicGameComponents[i]->GetPosition();
		lightSource.m_lightProperties.constantAttenuation = 1.f;
		lightSource.m_lightProperties.linearAttenuation = 0.4f;
		lightSource.m_lightProperties.quadraticAttenuation = 0.0005f;
		lightSource.m_lightProperties.Strength = engine::math::Vector3(1.0f, 1.0f, 1.0f) * 10;
		lightSource.m_lightProperties.SpotAngle = DirectX::XMConvertToRadians(70.0);
		lightSource.m_lightProperties.Direction = engine::math::Vector3(0.0f, -1.0f, 0.0f);

		m_lightSources.push_back(lightSource);
	}*/

	LightSource lightSource(LightSource::Type::DIRECTIONAL_LIGHT);
	lightSource.m_lightProperties.Strength = engine::math::Vector3(1.0f, 1.0f, 1.0f);
	lightSource.m_lightProperties.Direction = engine::math::Vector3(0.0f, -1.0f, 0.0f);
	m_lightSources.push_back(lightSource);

	m_inspectionCamera = std::make_unique<OrthograficCamera>(
		engine::math::Vector3(0, 100, 0), engine::math::Vector3(0, 0, 0), engine::math::Vector3(0, 0, 1), -200.f, 200.f, -200.f, 200.f);

	m_timer.StartClock();
}

void RasterizationGraphics::Update(float deltaTime, float totalTime)
{
	FrameResources& frameResources = m_graphicsResources.GetFrameResources();

	/////////////////////////////////////////////////////////////////////////
	// Actualizam mai intai obiectele scenei
	m_skyBoxRenderer->Update(deltaTime);
	m_waterRenderer->Update(deltaTime);
	m_terrainRender->Update(deltaTime);
	m_objectRenderer->Update(deltaTime);

	for (size_t i = 0; i < m_dynamicGameComponents.size() - 1; i++)
	{
		engine::math::Quaternion rot(engine::math::Vector3(0, 1, 0), engine::math::Scalar(4.f * i));

		m_dynamicGameComponents[i]->SetPosition(rot * engine::math::Vector3(10, 0, 0) + engine::math::Vector3(-70, 10, 30));
		m_dynamicGameComponents[i]->Update(deltaTime);

		// m_lightSources[i].m_lightProperties.Position = m_dynamicGameComponents[i]->GetPosition();
	}

	// Randam iar cube-mapul din cauza ca se misca skybox-ul
	// m_dynamicCubeMap->SetRenderDirty(true);
	// if (m_timer.GetTimeSinceStart() > 0.1f)
	//{
	m_dynamicCubeMap->SetCenter(engine::math::Vector3(-92.930351, 0, 0.121590));
	m_waterRenderer->SetCubeMapCenter(m_dynamicCubeMap->GetCubeMapCenter());
	m_shadowMap->SetDirection(m_skyBoxRenderer->GetLightDirection());

	m_timer.ResetStartTime();
	//}

	/////////////////////////////////////////////////////////////////////////
	// Actaulizam camerele - main, cele de cubemap si cea pentru shadow map
	if (m_cameraController.IsDirty())
	{
		m_cameraController.Update();

		m_terrainRender->FrustumCulling(m_cameraController);
		m_waterRenderer->FrustumCulling(m_cameraController);
		m_objectRenderer->FrustumCulling(m_cameraController);

		m_cameraController.DecreaseDirtyCount();
	}

	if (engine::core::Settings::UseAdvancedReflections() && m_dynamicCubeMap->IsDirty())
	{
		m_dynamicCubeMap->Update();

		m_dynamicCubeMap->DecreaseDirtyCount();
	}

	if (engine::core::Settings::UseShadows() && m_shadowMap->IsDirty())
	{
		m_shadowMap->Update();

		m_lightSources[0].m_lightProperties.Direction = m_shadowMap->GetLightDirection();
		m_lightSources[0].m_lightProperties.lightTransformMatrix =
			engine::math::Matrix4::Transpose(m_shadowMap->GetCamera().GetViewProjMatrix());

		m_shadowMap->DecreaseDirtyCount();
	}

	/////////////////////////////////////////////////////////////////////////
	// Actualizam bufferele CB
	// Ordinea la pass este importanta!!!!
	frameResources.UpdateMainPassCB(m_camera, deltaTime, 0.3, m_renderMode, m_lightSources);
	frameResources.UpdateDyanmicCubeMapPassCB(*m_dynamicCubeMap);
	frameResources.UpdateShadowMapPassCB(*m_shadowMap);
	frameResources.UpdateObjectRendererCB(*m_objectRenderer);
	frameResources.UpdatePerMaterialCB(m_materialManager);
	frameResources.UpdateSkyBoxCB(*m_skyBoxRenderer);
	frameResources.UpdateWaterCB(*m_waterRenderer);
}

void RasterizationGraphics::PopulateCommandList()
{
	GraphicsContext& graphicsContext = m_graphicsResources.GetGraphicsContext();
	FrameResources& frameResources = m_graphicsResources.GetFrameResources();

	graphicsContext.SetRootSignature(*m_RSManager.GetRootSignature("Default"));
	graphicsContext.SetDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		m_graphicsResources.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetHeapPointer());
	graphicsContext.BindDescriptorHeaps();
	graphicsContext.SetDescriptorTable(DefaultRSBindings::StaticPixelTextures, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	graphicsContext.SetDescriptorTable(
		DefaultRSBindings::StaticNonPixelTextures, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	graphicsContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (m_shadowMap->IsRenderDirty())
	{
		m_shadowMap->Setup(graphicsContext);

		graphicsContext.SetConstantBuffer(DefaultRSBindings::PassCB, frameResources.m_perPassCB.GetGpuVirtualAdress(7));

		//m_objectRenderer->Render(RenderLayer::ShadowMap);
		m_terrainRender->Render(RenderLayer::ShadowMap);

		m_shadowMap->Finish(graphicsContext);
		m_shadowMap->SetRenderDirty(false);
	}

	graphicsContext.SetDescriptorTable(DefaultRSBindings::DynamicTextures, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (m_dynamicCubeMap->IsRenderDirty())
	{
		m_dynamicCubeMap->Setup(graphicsContext);

		for (int i = 0; i < 6; i++)
		{
			m_dynamicCubeMap->SetRenderTarget(graphicsContext, i);

			graphicsContext.SetConstantBuffer(
				DefaultRSBindings::PassCB, frameResources.m_perPassCB.GetGpuVirtualAdress(i + 1));

			m_skyBoxRenderer->Render(RenderLayer::CubeMap);
			m_terrainRender->Render(RenderLayer::CubeMap);
			// m_objectRenderer->Render(RenderLayer::CubeMap);
		}

		m_dynamicCubeMap->Finish(graphicsContext);
		m_dynamicCubeMap->SetRenderDirty(false);
	}

	m_graphicsResources.SetRenderTarget();

	graphicsContext.SetConstantBuffer(DefaultRSBindings::PassCB, frameResources.m_perPassCB.GetGpuVirtualAdress(0));

	// m_objectRenderer->Render(pResources, RenderLayer::DebugShadowMap);
	// return;
	graphicsContext.SetDescriptorTable(DefaultRSBindings::DynamicTextures, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_skyBoxRenderer->Render(RenderLayer::Base);
	//m_objectRenderer->Render(RenderLayer::Base);

	graphicsContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	m_terrainRender->Render(RenderLayer::Base);
	m_waterRenderer->Render(RenderLayer::Base);

	//m_textureRenderer->Render(RenderLayer::Base);
}