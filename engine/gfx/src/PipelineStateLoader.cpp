#include "PipelineStateLoader.hpp"

#include "RootSignatureManager.hpp"
#include "ShadersManager.hpp"
#include "RayTracingGraphics.hpp"

#define GET_SHADER_DATA(name) shadersManager.GetBufferPointer(name), shadersManager.GetBufferSize(name)
#define GET_RT_SHADER_DATA(name) (void*)shadersManager.GetBufferPointer(name), shadersManager.GetBufferSize(name)

GraphicsPSO::Ptr PipelineStateLoader::LoadDefaultPipelineState(const ShadersManager& shadersManager)
{
	GraphicsPSO::Ptr pso = GraphicsPSO::CreateEmptyPSO();

	pso->SetVertexShader(GET_SHADER_DATA("DefaultVS"));
	pso->SetPixelShader(GET_SHADER_DATA("DefaultPS"));

	// Setare input layout
	pso->SetInputLayout(
		5,
		{D3D12_INPUT_ELEMENT_DESC{
			 "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "Color",
			 0,
			 DXGI_FORMAT_R32G32B32A32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, color),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "Normal",
			 0,
			 DXGI_FORMAT_R32G32B32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, normal),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "Tangent",
			 0,
			 DXGI_FORMAT_R32G32B32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, tangent),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "TextureC",
			 0,
			 DXGI_FORMAT_R32G32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, texC),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0}});

	// Setare rasterizare
	{
		CD3DX12_RASTERIZER_DESC desc(D3D12_DEFAULT);
		// desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		// desc.CullMode = D3D12_CULL_MODE_NONE;
		pso->SetRasterizerState(desc);
	}

	// Setare blend state (alpha blending in principiu)
	{
		CD3DX12_BLEND_DESC desc(D3D12_DEFAULT);
		pso->SetBlendState(desc);
	}

	// Setare multisampleing
	{
		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count = engine::core::Settings::GetGraphicsSettings().GetMSAASampleCount();
		sampleDesc.Quality = engine::core::Settings::GetGraphicsSettings().GetMSAAQuality();

		pso->SetSampleState(sampleDesc);
	}

	// Setare depth stencil si format DSV
	{
		CD3DX12_DEPTH_STENCIL_DESC desc(D3D12_DEFAULT);
		pso->SetDepthStencilState(desc);
		pso->SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
	}

	// Setare chestii
	{
		pso->SetSampleMask(UINT_MAX);
		pso->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		pso->SetRTVs(1, {DXGI_FORMAT_R8G8B8A8_UNORM});

		pso->SetNodeMask(0);
		pso->SetFlags(D3D12_PIPELINE_STATE_FLAG_NONE);
	}

	return pso;
}

GraphicsPSO::Ptr PipelineStateLoader::LoadSkyBoxPipelineState(const ShadersManager& shadersManager)
{
	GraphicsPSO::Ptr pso = LoadDefaultPipelineState(shadersManager);

	pso->SetVertexShader(GET_SHADER_DATA("SkyBoxVS"));
	pso->SetPixelShader(GET_SHADER_DATA("SkyBoxPS"));

	pso->SetInputLayout(
		2,
		{D3D12_INPUT_ELEMENT_DESC{
			 "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "TextureC",
			 0,
			 DXGI_FORMAT_R32G32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, texC),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0}});

	{
		CD3DX12_RASTERIZER_DESC desc(D3D12_DEFAULT);
		desc.CullMode = D3D12_CULL_MODE_NONE;
		pso->SetRasterizerState(desc);
	}

	{
		CD3DX12_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = false;
		pso->SetDepthStencilState(desc);
	}

	return pso;
}

GraphicsPSO::Ptr PipelineStateLoader::LoadTerrainPipelineState(const ShadersManager& shadersManager)
{
	GraphicsPSO::Ptr pso = LoadDefaultPipelineState(shadersManager);

	pso->SetVertexShader(GET_SHADER_DATA("TerrainVS"));
	pso->SetHullShader(GET_SHADER_DATA("TerrainHS"));
	pso->SetDomainShader(GET_SHADER_DATA("TerrainDS"));
	pso->SetPixelShader(GET_SHADER_DATA("TerrainPS"));

	pso->SetInputLayout(
		4,
		{D3D12_INPUT_ELEMENT_DESC{
			 "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "Normal",
			 0,
			 DXGI_FORMAT_R32G32B32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, normal),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "Tangent",
			 0,
			 DXGI_FORMAT_R32G32B32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, tangent),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "TextureC",
			 0,
			 DXGI_FORMAT_R32G32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, texC),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0}});

	{
		CD3DX12_RASTERIZER_DESC desc(D3D12_DEFAULT);
		// desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		// desc.CullMode = D3D12_CULL_MODE_NONE;
		pso->SetRasterizerState(desc);
	}

	pso->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);

	return pso;
}

GraphicsPSO::Ptr PipelineStateLoader::LoadWaterPipelineState(const ShadersManager& shadersManager)
{
	GraphicsPSO::Ptr pso = LoadDefaultPipelineState(shadersManager);

	pso->SetVertexShader(GET_SHADER_DATA("WaterVS"));
	pso->SetHullShader(GET_SHADER_DATA("WaterHS"));
	pso->SetDomainShader(GET_SHADER_DATA("WaterDS"));
	pso->SetPixelShader(GET_SHADER_DATA("WaterPS"));

	pso->SetInputLayout(
		2,
		{D3D12_INPUT_ELEMENT_DESC{
			 "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "TextureC",
			 0,
			 DXGI_FORMAT_R32G32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, texC),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0}});

	{
		CD3DX12_RASTERIZER_DESC desc(D3D12_DEFAULT);
		//desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		//desc.CullMode = D3D12_CULL_MODE_NONE;
		pso->SetRasterizerState(desc);
	}

	// Setare blend state (alpha blending in principiu)
	{
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		pso->SetBlendState(blendDesc);
	}

	pso->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);

	return pso;
}

GraphicsPSO::Ptr PipelineStateLoader::LoadCubeMapTerrainPipelineState(const ShadersManager& shadersManager)
{
	GraphicsPSO::Ptr pso = LoadDefaultPipelineState(shadersManager);

	pso->SetInputLayout(
		4,
		{D3D12_INPUT_ELEMENT_DESC{
			 "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "Normal",
			 0,
			 DXGI_FORMAT_R32G32B32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, normal),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "Tangent",
			 0,
			 DXGI_FORMAT_R32G32B32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, tangent),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "TextureC",
			 0,
			 DXGI_FORMAT_R32G32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, texC),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0}});

	pso->SetVertexShader(GET_SHADER_DATA("TerrainCubeMapVS"));
	pso->SetPixelShader(GET_SHADER_DATA("TerrainPS"));


	{
		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count = 1;
		pso->SetSampleState(sampleDesc);
	}

	pso->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	return pso;
}

GraphicsPSO::Ptr PipelineStateLoader::LoadShadowMapPipelineState(const ShadersManager& shadersManager)
{
	GraphicsPSO::Ptr pso = LoadDefaultPipelineState(shadersManager);

	pso->SetVertexShader(GET_SHADER_DATA("ShadowRenderVS"));
	pso->SetPixelShader(GET_SHADER_DATA("ShadowRenderPS"));

	{
		CD3DX12_DEPTH_STENCIL_DESC desc(D3D12_DEFAULT);
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		pso->SetDepthStencilState(desc);
		pso->SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
	}

	{
		CD3DX12_RASTERIZER_DESC desc(D3D12_DEFAULT);
		desc.DepthClipEnable = TRUE;
		desc.DepthBias = 100000;
		desc.DepthBiasClamp = 0.0f;
		desc.SlopeScaledDepthBias = 100.0f;
		pso->SetRasterizerState(desc);
	}

	{
		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count = 1;
		pso->SetSampleState(sampleDesc);
	}

	pso->SetRTVs(0, {});

	return pso;
}

GraphicsPSO::Ptr PipelineStateLoader::LoadTexturePipelineState(const ShadersManager& shadersManager)
{
	GraphicsPSO::Ptr pso = LoadDefaultPipelineState(shadersManager);

	pso->SetInputLayout(
		2,
		{D3D12_INPUT_ELEMENT_DESC{
			 "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		 D3D12_INPUT_ELEMENT_DESC{
			 "TextureC",
			 0,
			 DXGI_FORMAT_R32G32_FLOAT,
			 0,
			 offsetof(Mesh::Vertex, texC),
			 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			 0}});

	{
		pso->SetVertexShader(GET_SHADER_DATA("TextureRenderVS"));
		pso->SetPixelShader(GET_SHADER_DATA("TextureRenderPS"));
	}

	{
		CD3DX12_RASTERIZER_DESC desc(D3D12_DEFAULT);
		// desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		desc.CullMode = D3D12_CULL_MODE_NONE;
		pso->SetRasterizerState(desc);
	}

	return pso;
}

ComputePSO::Ptr PipelineStateLoader::LoadWavesComputePipelineState(const ShadersManager& shadersManager)
{
	ComputePSO::Ptr pso = ComputePSO::CreateEmptyPSO();

	pso->SetComputeShader(GET_SHADER_DATA("WavesCS"));
	pso->SetNodeMask(0);
	pso->SetFlags(D3D12_PIPELINE_STATE_FLAG_NONE);

	return pso;
}


RayTracingPSO::Ptr PipelineStateLoader::LoadDefaultRayTracingPipelineState(
	ID3D12Device10* pDevice, const RootSignatureManager& rsManager, const ShadersManager& shadersManager)
{
	RayTracingPSO::Ptr pso = RayTracingPSO::CreateEmptyPSO();

	pso->desc = CD3DX12_STATE_OBJECT_DESC{D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE};

	auto lib = pso->desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(GET_RT_SHADER_DATA("DefaultRT"));
	lib->SetDXILLibrary(&libdxil);
	{
		lib->DefineExport(engine::gfx::rt::c_raygenShaderName);

		lib->DefineExport(engine::gfx::rt::ClosestHit_TriangleGeometry);
		lib->DefineExport(engine::gfx::rt::ClosestHit_AABBGeometry);

		lib->DefineExport(engine::gfx::rt::IntersectionShader);

		lib->DefineExport(engine::gfx::rt::Miss_RadianceShader);
		lib->DefineExport(engine::gfx::rt::Miss_ShadowShader);
	}

	{
		auto hitGroup = pso->desc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetHitGroupExport(engine::gfx::rt::HitGroupName_TriangleGeometry);
		hitGroup->SetClosestHitShaderImport(engine::gfx::rt::ClosestHit_TriangleGeometry);
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	}

	{
		auto hitGroup = pso->desc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetHitGroupExport(engine::gfx::rt::HitGroupName_AABBGeometry);
		hitGroup->SetClosestHitShaderImport(engine::gfx::rt::ClosestHit_AABBGeometry);
		hitGroup->SetIntersectionShaderImport(engine::gfx::rt::IntersectionShader);
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
	}


	auto shaderConfig = pso->desc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = 4 * sizeof(float) + sizeof(UINT);
	UINT attributeSize = 3 * sizeof(float);
	shaderConfig->Config(payloadSize, attributeSize);

	// RayGen shader association
	{
		auto localRootSignature = pso->desc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(rsManager.GetID3D12RootSignature("RayGen"));

		// Shader association
		auto rootSignatureAssociation = pso->desc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		rootSignatureAssociation->AddExport(engine::gfx::rt::c_raygenShaderName);
	}

	// HitGroup triangle geometry shader association
	{
		auto localRootSignature = pso->desc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(rsManager.GetID3D12RootSignature("Triangle_Hit"));

		// Shader association
		auto rootSignatureAssociation = pso->desc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		rootSignatureAssociation->AddExport(engine::gfx::rt::HitGroupName_TriangleGeometry);
	}

	// HitGroup AABB geometry shader association
	{
		auto localRootSignature = pso->desc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(rsManager.GetID3D12RootSignature("AABB_Hit"));

		// Shader association
		auto rootSignatureAssociation = pso->desc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		rootSignatureAssociation->AddExport(engine::gfx::rt::HitGroupName_AABBGeometry);
	}

	// Intersection shader association
	//{
	//	auto localRootSignature = pso->desc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	//	localRootSignature->SetRootSignature(rsManager.GetID3D12RootSignature("RayGen"));

	//	// Shader association
	//	auto rootSignatureAssociation = pso->desc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	//	rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
	//	rootSignatureAssociation->AddExport(engine::gfx::rt::IntersectionShader);
	//}

	// Default miss shader associtaion
	{
		auto localRootSignature = pso->desc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(rsManager.GetID3D12RootSignature("Miss"));

		// Shader association
		auto rootSignatureAssociation = pso->desc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		rootSignatureAssociation->AddExport(engine::gfx::rt::Miss_RadianceShader);
	}

	// Shadow miss shader association
	// !!!! Important: foloses aceeasi RS ca la ray gen pentru ca este goala!!!!
	// !!!! Daca se schimba, trebuie creata un RS local pentru shadow miss shader !!!
	{
		auto localRootSignature = pso->desc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(rsManager.GetID3D12RootSignature("RayGen"));

		// Shader association
		auto rootSignatureAssociation = pso->desc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		rootSignatureAssociation->AddExport(engine::gfx::rt::Miss_ShadowShader);
	}

	auto globalRootSignature = pso->desc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(rsManager.GetID3D12RootSignature("Global"));

	auto pipelineConfig = pso->desc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	UINT maxRecursionDepth = 2;
	pipelineConfig->Config(maxRecursionDepth);

	pso->SetRootSignature(rsManager.GetRootSignature("Global"));
	pso->Finalize(pDevice);

	return pso;
}
