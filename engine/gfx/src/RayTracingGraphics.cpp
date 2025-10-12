#include "RayTracingGraphics.hpp"

#include "engine/core/DxgiInfoManager.hpp"
#include "GeometryGenerator.hpp"

#include <algorithm>

using namespace misc::render_descriptors;
using namespace misc::RSBinding;

RayTracingGraphics::RayTracingGraphics(GraphicsResources& resources) : Graphics(resources)
{
	GraphicsContext& graphicsContext = m_graphicsResources.GetGraphicsContext();
	graphicsContext.Reset();

	// Load PSO si cache
	m_rayTracingPSOManager.LoadPSOs(m_RSManager, m_shaderManager, GraphicsResources::GetDevice());
	pGlobalRTRS = m_rayTracingPSOManager.GetPSO("DefaultRTPSO")->GetRootSignaturePtr();
	pRTPSOStateObject = m_rayTracingPSOManager.GetPSO("DefaultRTPSO");

	m_materialManager.LoadDefaultMaterials();

	m_textureManager.LoadTexturesFormFiles(GraphicsResources::GetDevice(), GraphicsResources::GetCommandQueue());
	m_textureManager.CreateSRVHandles();
	m_textureManager.SwapNonPixelShaderTextures(graphicsContext);

	m_graphicsResources.GetContextManager().Flush(false);
	m_graphicsResources.GetContextManager().GetGraphicsContext().Reset();

	{
		DX_TERRAIN_DESCRIPTOR desc;
		DX_OBJECT_DESCRIPTOR& objectDesc = desc.objectDescriptor;

		objectDesc.objectCB_ID = FrameResources::GetObjectCB_ID();
		objectDesc.materialCB_ID = m_materialManager.GetMaterialCB_ID("Default");

		objectDesc.textureTransform = engine::math::Matrix4::MakeScale(engine::math::Vector3(40.f, 40.f, 1));
		objectDesc.isStatic = true;

		desc.length = 400.f;
		desc.width = 400.f;
		desc.chunkCountPerSide = 10;
		desc.chunkKernelSize = 15;

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
		objectDesc.isStatic = false;

		desc.waveProperties[0].amplitude = 0.7f;  // 0.7
		desc.waveProperties[0].speed = 0.3f;
		desc.waveProperties[0].wavelength = 8.f;
		desc.waveProperties[0].direction = DirectX::XMFLOAT2(-1.f, 0.4f);
		desc.waveProperties[0].steepness = 0.4f;

		desc.waveProperties[1].amplitude = 1.4f;  // 1.4
		desc.waveProperties[1].speed = 0.4f;
		desc.waveProperties[1].wavelength = 10.f;
		desc.waveProperties[1].direction = DirectX::XMFLOAT2(0.3f, -1.f);
		desc.waveProperties[1].steepness = 0.9f;

		desc.length = 400.f;
		desc.width = 400.f;
		desc.chunkKernelSize = 10;
		desc.chunkCountPerSide = 20;

		desc.textureScale = engine::math::Vector3(10, 10, 10);
		desc.textureDirection = engine::math::Vector3(-0.3f, 1.f, 0.45f);
		desc.textureMoveSpeed = 0.02f;
		desc.cubeMapSphereRadius = 200.f;

		m_waterRenderer = WaterRenderer::CreateWaterRenderer(desc);
	}

	{
		DX_SKYBOX_DESCRIPTOR desc;
		DX_OBJECT_DESCRIPTOR& objectDesc = desc.objectDescriptor;

		objectDesc.objectCB_ID = FrameResources::GetObjectCB_ID();
		objectDesc.isStatic = false;

		desc.rotationAxis = engine::math::Vector3(0, -1, 0);
		desc.rotationSpeed = 0.06f;

		constexpr float angle = DirectX::XMConvertToRadians(55.f);
		desc.lightDirection = engine::math::Vector3(-cosf(angle), -sinf(angle), 0.f);

		m_skyBoxRenderer = SkyBoxRenderer::CreateSkyBoxRenderer(desc);
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
			objectDesc.isStatic = true;

			desc.objectDescriptors.push_back(objectDesc);
		}

		{
			DX_OBJECT_DESCRIPTOR objectDesc;
			objectDesc.subGeometryName = "Quad";
			objectDesc.objectCB_ID = FrameResources::GetObjectCB_ID();
			objectDesc.isStatic = true;

			desc.objectDescriptors.push_back(objectDesc);
		}

		m_objectRenderer = ObjectRenderer::CreateObjectRenderer(desc);
	}

	// Creare structuri de acccelerare
	m_terrainRender->BuildAccelerationStructures();
	m_waterRenderer->BuildAccelerationStructures();
	m_skyBoxRenderer->BuildAccelerationStructures();
	m_objectRenderer->BuildAccelerationStructures();

	CreateTopLevelAccelerationStructure();

	CreateShaderTable();

	LoadAssets();

	m_shaderManager.ClearShaders();

	GraphicsResources::GetContextManager().Flush(true);
}

void RayTracingGraphics::LoadAssets()
{
	// Setare CB static pentru obiectele static din scena: teren, apa
	for (UINT i = 0; i < Settings::GetFrameResourcesCount(); i++)
	{
		m_graphicsResources.GetFrameResources(i).UpdateTerrainCB(*m_terrainRender);
	}

	LightSource lightSource(LightSource::Type::DIRECTIONAL_LIGHT);
	lightSource.m_lightProperties.Strength = engine::math::Vector3(1.0f, 1.0f, 1.0f) * 1.5;
	lightSource.m_lightProperties.Direction = engine::math::Vector3(0.0f, -1.0f, 0.0f);
	m_lightSources.push_back(lightSource);
}

RayTracingGraphics::~RayTracingGraphics()
{
}

void RayTracingGraphics::Update(float deltaTime, float totalTime)
{
	FrameResources& frameResources = m_graphicsResources.GetFrameResources();

	m_skyBoxRenderer->Update(deltaTime);
	m_waterRenderer->Update(deltaTime);
	m_terrainRender->Update(deltaTime);
	m_objectRenderer->Update(deltaTime);

	m_lightSources[0].m_lightProperties.Direction = m_skyBoxRenderer->GetLightDirection();

	if (m_cameraController.IsDirty())
	{
		m_cameraController.Update();

		m_cameraController.DecreaseDirtyCount();
	}

	frameResources.UpdateMainPassCB(m_camera, deltaTime, 0.3, m_renderMode, m_lightSources);
	frameResources.UpdateObjectRendererCB(*m_objectRenderer);
	frameResources.UpdatePerMaterialCB(m_materialManager);
	frameResources.UpdateSkyBoxCB(*m_skyBoxRenderer);
	frameResources.UpdateWaterCB(*m_waterRenderer);
}

void RayTracingGraphics::PopulateCommandList()
{
	ComputeContext& commandContext = m_graphicsResources.GetGraphicsContext().GetComputeContext();
	FrameResources& frameResources = m_graphicsResources.GetFrameResources();

	D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
	{
		raytraceDesc.Width = Settings::GetGraphicsSettings().GetWidth();
		raytraceDesc.Height = Settings::GetGraphicsSettings().GetHeight();
		raytraceDesc.Depth = 1;

		raytraceDesc.HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
		raytraceDesc.HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
		raytraceDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStride;

		raytraceDesc.MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
		raytraceDesc.MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
		raytraceDesc.MissShaderTable.StrideInBytes = m_missShaderTableStride;

		raytraceDesc.RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress();
		raytraceDesc.RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable->GetDesc().Width;
	}

	commandContext.SetRootSignature(*pGlobalRTRS);

	commandContext.SetDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		m_graphicsResources.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetHeapPointer());
	commandContext.BindDescriptorHeaps();

	commandContext.SetDescriptorTable(GlobalRTRSBinding::OutputSlot, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	commandContext.SetShaderResourceView(
		GlobalRTRSBinding::AccelerationStrcuture, m_topLevelAccelerationStructure.GetGpuVirtualAddress());
	commandContext.SetConstantBuffer(GlobalRTRSBinding::PassCB, frameResources.m_perPassCB.GetGpuVirtualAdress());

	commandContext.SetPipelineStateObject(*pRTPSOStateObject);

	m_graphicsResources.SetRenderTarget();

	commandContext.DispatchRays(&raytraceDesc);
}

void RayTracingGraphics::CreateShaderTable()
{
	FrameResources& frameResources = m_graphicsResources.GetFrameResources();

	void* rayGenShaderIdentifier;
	void* defaultMissIdentifier;
	void* shadowMissIdentifier;
	void* triangleGeometryHitGroupShaderIdentifier;
	void* AABBGeometryHitGroupShaderIndentifier;

	auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
	{
		rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(misc::rt::c_raygenShaderName);

		defaultMissIdentifier = stateObjectProperties->GetShaderIdentifier(misc::rt::Miss_RadianceShader);
		shadowMissIdentifier = stateObjectProperties->GetShaderIdentifier(misc::rt::Miss_ShadowShader);


		triangleGeometryHitGroupShaderIdentifier =
			stateObjectProperties->GetShaderIdentifier(misc::rt::HitGroupName_TriangleGeometry);
		AABBGeometryHitGroupShaderIndentifier =
			stateObjectProperties->GetShaderIdentifier(misc::rt::HitGroupName_AABBGeometry);
	};

	Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
	m_rayTracingPSOManager.GetPSO("DefaultRTPSO")
		->GetID3D12StateObject()
		->QueryInterface(IID_PPV_ARGS(&stateObjectProperties));

	UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	GetShaderIdentifiers(stateObjectProperties.Get());

	// Ray gen shader table
	{
		struct RootArguments
		{
			D3D12_GPU_DESCRIPTOR_HANDLE heapStartHandle;
		} rootArguments;

		rootArguments.heapStartHandle =
			m_graphicsResources.GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetFirstDescriptorHandle();

		UINT numShaderRecords = 1;
		UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);
		ShaderTable rayGenShaderTable(
			GraphicsResources::GetDevice(), numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
		rayGenShaderTable.push_back(
			ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));

		m_rayGenShaderTable = rayGenShaderTable.GetResource();
	}

	// Miss shader table
	{
		struct RootArguments
		{
			D3D12_GPU_DESCRIPTOR_HANDLE cubeMapHandle;
		} rootArguments;
		rootArguments.cubeMapHandle = m_textureManager.GetTexture(L"SkyBox").GetSrvHandle();

		UINT numShaderRecords = 2;
		UINT shaderRecordSize = shaderIdentifierSize + sizeof(RootArguments);
		ShaderTable missShaderTable(
			GraphicsResources::GetDevice(), numShaderRecords, shaderRecordSize, L"MissShaderTable");

		missShaderTable.push_back(
			ShaderRecord(defaultMissIdentifier, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
		missShaderTable.push_back(ShaderRecord(shadowMissIdentifier, shaderIdentifierSize));

		m_missShaderTable = missShaderTable.GetResource();
		m_missShaderTableStride = missShaderTable.GetShaderRecordSize();
	}

	// Hit group shader table
	{
		struct TriangleRootArguments
		{
			D3D12_GPU_VIRTUAL_ADDRESS objectCBHandle;
			D3D12_GPU_VIRTUAL_ADDRESS materialCBHnadle;
			D3D12_GPU_DESCRIPTOR_HANDLE vertexIndexHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE textureSRVHnadle;
		};

		struct AABBRootArguments
		{
			D3D12_GPU_VIRTUAL_ADDRESS objectCBHandle;
			D3D12_GPU_VIRTUAL_ADDRESS materialCBHnadle;
			D3D12_GPU_VIRTUAL_ADDRESS waterCBHandle;
			D3D12_GPU_DESCRIPTOR_HANDLE textureSRVHnadle;
		};

		TriangleRootArguments terrainRootArguments;
		{
			terrainRootArguments.objectCBHandle =
				frameResources.m_perObjectCB.GetGpuVirtualAdress(m_terrainRender->GetObjectCB_ID());
			terrainRootArguments.materialCBHnadle =
				frameResources.m_perMaterialCB.GetGpuVirtualAdress(m_terrainRender->GetMaterialCB_ID());
			terrainRootArguments.vertexIndexHandle = m_terrainRender->GetVertexBuffer().GetSRVHandle();
			terrainRootArguments.textureSRVHnadle = m_textureManager.GetTexture(L"Terrain.Diffuse").GetSrvHandle();
		}

		AABBRootArguments waterRootArguments;
		{
			waterRootArguments.objectCBHandle =
				frameResources.m_perObjectCB.GetGpuVirtualAdress(m_waterRenderer->GetObjectCB_ID());
			waterRootArguments.materialCBHnadle =
				frameResources.m_perMaterialCB.GetGpuVirtualAdress(m_waterRenderer->GetMaterialCB_ID());
			waterRootArguments.waterCBHandle = frameResources.m_waterCB.GetGpuVirtualAdress();
			waterRootArguments.textureSRVHnadle = m_textureManager.GetTexture(L"Terrain.Diffuse").GetSrvHandle();
		}

		UINT numShaderRecords = 2;
		UINT shaderRecordSize =
			shaderIdentifierSize + max(sizeof(TriangleRootArguments), sizeof(AABBRootArguments));
		ShaderTable hitGroupShaderTable(
			GraphicsResources::GetDevice(), numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

		hitGroupShaderTable.push_back(ShaderRecord(
			triangleGeometryHitGroupShaderIdentifier,
			shaderIdentifierSize,
			&terrainRootArguments,
			sizeof(terrainRootArguments)));
		hitGroupShaderTable.push_back(ShaderRecord(
			AABBGeometryHitGroupShaderIndentifier,
			shaderIdentifierSize,
			&waterRootArguments,
			sizeof(waterRootArguments)));

		m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
		m_hitGroupShaderTableStride = hitGroupShaderTable.GetShaderRecordSize();
	}
}

void RayTracingGraphics::CreateTopLevelAccelerationStructure()
{
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescriptorVector;
	// Adaugare BLAS pentru teren
	{
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
		instanceDesc.InstanceMask = 0b00000001;
		instanceDesc.InstanceID = 0;
		instanceDesc.InstanceContributionToHitGroupIndex = 0;
		instanceDesc.AccelerationStructure =
			m_terrainRender->GetBottomLevelAccelerationStructure()->GetGPUVirtualAddress();

		instanceDescriptorVector.push_back(instanceDesc);
	}

	// Adaugare BLAS pentru apa
	{
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
		instanceDesc.InstanceMask = 0b00000010;
		instanceDesc.InstanceContributionToHitGroupIndex = 1;
		instanceDesc.InstanceID = 1;
		instanceDesc.AccelerationStructure =
			m_waterRenderer->GetBottomLevelAccelerationStructure()->GetGPUVirtualAddress();

		instanceDescriptorVector.push_back(instanceDesc);
	}

	m_topLevelAccelerationStructure.Build(
		instanceDescriptorVector, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE);

	// m_topLevelAccelerationStructure.CreateSRV();
}
