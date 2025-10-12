#include "RS_Manager.hpp"

#include "Utilities.hpp"
#include "engine/core/CustomException.hpp"
#include "engine/core/Settings.hpp"

// Ocuparea registrilor:
// t0, space0 - texture environmental (skybox/reflectii)
// t1, space0 - textura de shadow (shadow map)
// t2, space0 - textura/texturi difuze
// t3, space0 - textura/texturi de normale
// t0, space1 - texturi displacement

// b0, space0 - pass CB
// b1, space0 - object CB
// b2, space0 - material CB
// b0, space1 - water CB

using namespace engine::gfx::RSBinding;

RS_Manager::RS_Manager()
{
	if (engine::core::Settings::UseRayTracing())
	{
		m_loadFunctions["Global"] = &RS_Manager::CreateGlobalRTRS;
		m_loadFunctions["Triangle_Hit"] = &RS_Manager::CreateTriangleHitRS;
		m_loadFunctions["AABB_Hit"] = &RS_Manager::CreateAABBHitRS;
		m_loadFunctions["Miss"] = &RS_Manager::CreateMissRS;
		m_loadFunctions["RayGen"] = &RS_Manager::CreateRayGenRS;
	}
	else
	{
		m_loadFunctions["Default"] = &RS_Manager::CreateDefaultRS;
		m_loadFunctions["Water"] = &RS_Manager::CreateWaterRS;
		m_loadFunctions["Texture"] = &RS_Manager::CreateTextureRS;
	}

	m_loadFunctions["Waves"] = &RS_Manager::CreateWavesRS;
}

bool RS_Manager::AddRootSiganture(std::string name, RootSignature::Ptr rootSignture)
{
	auto it = m_rootSignatures.find(name);
	if (it != m_rootSignatures.end())
		return false;

	m_rootSignatures[name] = rootSignture;

	return true;
}

RootSignature::Ptr RS_Manager::GetRootSignature(std::string name) const
{
	auto it = m_rootSignatures.find(name);
	if (it == m_rootSignatures.end())
		throw engine::core::CustomException("Root siganture nu exista sau nu a fost creat inca");

	return it->second;
}

ID3D12RootSignature* RS_Manager::GetID3D12RootSignature(std::string name) const
{
	return GetRootSignature(name)->GetID3D12RootSignature();
}

void RS_Manager::LoadRootSignatures(ID3D12Device10* pDevice)
{
	if (!firstLoad)
		return;

	for (const auto& [name, func] : m_loadFunctions)
	{
		RootSignature::Ptr rs = func(pDevice);
		AddRootSiganture(name, rs);
	}

	m_loadFunctions.clear();

	firstLoad = false;
}


////////////////////////////////////////////////////////////////////////////////////
// Root sigantures pentru rasterizare
////////////////////////////////////////////////////////////////////////////////////
RootSignature::Ptr RS_Manager::CreateDefaultRS(ID3D12Device10* pDevice)
{
	RootSignature::Ptr rs = RootSignature::CreateEmptyRootSignature(DefaultRSBindings::Count);

	rs->AddDefaultStaticSamplers();
	rs->AddDefaultShadowMapSampler();

	rs->GetRootParameter(DefaultRSBindings::PassCB)
		.InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rs->GetRootParameter(DefaultRSBindings::ObjectCB)
		.InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rs->GetRootParameter(DefaultRSBindings::MaterialCB)
		.InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

	std::array<CD3DX12_DESCRIPTOR_RANGE1, 3> SRVRange;
	SRVRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0);
	rs->GetRootParameter(DefaultRSBindings::StaticPixelTextures)
		.InitAsDescriptorTable(1, &SRVRange[0], D3D12_SHADER_VISIBILITY_PIXEL);

	SRVRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 4);
	rs->GetRootParameter(DefaultRSBindings::StaticNonPixelTextures)
		.InitAsDescriptorTable(1, &SRVRange[1], D3D12_SHADER_VISIBILITY_ALL);

	SRVRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 6);
	rs->GetRootParameter(DefaultRSBindings::DynamicTextures)
		.InitAsDescriptorTable(1, &SRVRange[2], D3D12_SHADER_VISIBILITY_PIXEL);

	rs->SetRootSignatureFlags(
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	rs->Finalize(pDevice);

	return rs;
}

RootSignature::Ptr RS_Manager::CreateWaterRS(ID3D12Device10* pDevice)
{
	RootSignature::Ptr rs = RootSignature::CreateEmptyRootSignature(WaterRSParams::Count);

	rs->GetRootParameter(WaterRSParams::PassCB)
		.InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rs->GetRootParameter(WaterRSParams::ObjectCB)
		.InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rs->GetRootParameter(WaterRSParams::MaterialCB)
		.InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rs->GetRootParameter(WaterRSParams::WaterCB)
		.InitAsConstantBufferView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

	std::array<CD3DX12_DESCRIPTOR_RANGE1, 3> SRVRange;

	SRVRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0);
	rs->GetRootParameter(WaterRSParams::StaticPixelTextures)
		.InitAsDescriptorTable(1, &SRVRange[0], D3D12_SHADER_VISIBILITY_PIXEL);

	SRVRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 4);
	rs->GetRootParameter(WaterRSParams::StaticNonPixelTextures)
		.InitAsDescriptorTable(1, &SRVRange[1], D3D12_SHADER_VISIBILITY_ALL);

	SRVRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 2, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 6);
	rs->GetRootParameter(WaterRSParams::DynamicTextures)
		.InitAsDescriptorTable(1, &SRVRange[2], D3D12_SHADER_VISIBILITY_PIXEL);

	rs->AddDefaultStaticSamplers();
	rs->AddDefaultShadowMapSampler();

	rs->SetRootSignatureFlags(
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	rs->Finalize(pDevice);

	return rs;
}

RootSignature::Ptr RS_Manager::CreateTextureRS(ID3D12Device10* pDevice)
{
	RootSignature::Ptr rs = RootSignature::CreateEmptyRootSignature(1);

	std::array<CD3DX12_DESCRIPTOR_RANGE1, 1> SRVRange;

	SRVRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0);
	rs->GetRootParameter(0).InitAsDescriptorTable(1, &SRVRange[0], D3D12_SHADER_VISIBILITY_PIXEL);

	rs->AddDefaultStaticSamplers();

	rs->SetRootSignatureFlags(
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	rs->Finalize(pDevice);

	return rs;
}


////////////////////////////////////////////////////////////////////////////////////
// Root sigantures pentru ray tracing
////////////////////////////////////////////////////////////////////////////////////
RootSignature::Ptr RS_Manager::CreateGlobalRTRS(ID3D12Device10* pDevice)
{
	RootSignature::Ptr rs = RootSignature::CreateEmptyRootSignature(GlobalRTRSBinding::Count);

	rs->SetRootSignatureFlags(D3D12_ROOT_SIGNATURE_FLAG_NONE);

	std::array<CD3DX12_DESCRIPTOR_RANGE1, 2> ranges;
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);

	rs->GetRootParameter(GlobalRTRSBinding::OutputSlot)
		.InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
	rs->GetRootParameter(GlobalRTRSBinding::AccelerationStrcuture).InitAsShaderResourceView(0);
	rs->GetRootParameter(GlobalRTRSBinding::PassCB).InitAsConstantBufferView(0);

	rs->Finalize(pDevice);

	return rs;
}

RootSignature::Ptr RS_Manager::CreateTriangleHitRS(ID3D12Device10* pDevice)
{
	RootSignature::Ptr rs = RootSignature::CreateEmptyRootSignature(4);

	rs->SetRootSignatureFlags(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
	rs->AddDefaultStaticSamplers();

	rs->GetRootParameter(0).InitAsConstantBufferView(1);
	rs->GetRootParameter(1).InitAsConstantBufferView(2);

	std::array<CD3DX12_DESCRIPTOR_RANGE1, 2> ranges;
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0);

	rs->GetRootParameter(2).InitAsDescriptorTable(1, &ranges[0]);
	rs->GetRootParameter(3).InitAsDescriptorTable(1, &ranges[1]);

	rs->Finalize(pDevice);

	return rs;
}

RootSignature::Ptr RS_Manager::CreateAABBHitRS(ID3D12Device10* pDevice)
{
	RootSignature::Ptr rs = RootSignature::CreateEmptyRootSignature(5);

	rs->SetRootSignatureFlags(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
	rs->AddDefaultStaticSamplers();

	rs->GetRootParameter(0).InitAsConstantBufferView(1);
	rs->GetRootParameter(1).InitAsConstantBufferView(2);
	rs->GetRootParameter(2).InitAsConstantBufferView(3);

	std::array<CD3DX12_DESCRIPTOR_RANGE1, 1> ranges;
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0);

	rs->GetRootParameter(3).InitAsDescriptorTable(1, &ranges[0]);

	rs->Finalize(pDevice);

	return rs;
}


RootSignature::Ptr RS_Manager::CreateMissRS(ID3D12Device10* pDevice)
{
	RootSignature::Ptr rs = RootSignature::CreateEmptyRootSignature(1);

	rs->SetRootSignatureFlags(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
	rs->AddDefaultStaticSamplers();

	std::array<CD3DX12_DESCRIPTOR_RANGE1, 1> ranges;
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0);

	rs->GetRootParameter(0).InitAsDescriptorTable(1, &ranges[0]);

	rs->Finalize(pDevice);

	return rs;
}

RootSignature::Ptr RS_Manager::CreateRayGenRS(ID3D12Device10* pDevice)
{
	RootSignature::Ptr rs = RootSignature::CreateEmptyRootSignature(0);

	rs->SetRootSignatureFlags(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
	rs->AddDefaultStaticSamplers();
	rs->Finalize(pDevice);

	return rs;
}


////////////////////////////////////////////////////////////////////////////////////
// Root sigantures pentru compute
////////////////////////////////////////////////////////////////////////////////////
RootSignature::Ptr RS_Manager::CreateWavesRS(ID3D12Device10* pDevice)
{
	RootSignature::Ptr rs = RootSignature::CreateEmptyRootSignature(1);

	rs->GetRootParameter(0).InitAsUnorderedAccessView(0);

	rs->SetRootSignatureFlags(D3D12_ROOT_SIGNATURE_FLAG_NONE);
	rs->Finalize(pDevice);

	return rs;
}
