#pragma once

#include "PSO.hpp"
#include "RS_Manager.hpp"
#include "ShadersManager.hpp"
#include "PSO_Loader.hpp"

#include <unordered_map>

template <class PSOType>
class PSO_Manager
{
public:
	PSO_Manager() = default;
	~PSO_Manager();

	PSOType::Ptr GetPSO(std::string);
	bool AddPSO(std::string name, PSOType::Ptr PSO);

	void LoadPSOs(const RS_Manager& rs_manager, const ShadersManager& shadersManager, ID3D12Device10* pDevice);

private:
	std::unordered_map<std::string, typename PSOType::Ptr> PSOs;
	bool firstLoad = true;
};

//////////////////////////////////////////////////////////////////////////////

template <class PSOType>
PSO_Manager<PSOType>::~PSO_Manager()
{
}

template <class PSOType>
inline PSOType::Ptr PSO_Manager<PSOType>::GetPSO(std::string PSOname)
{
	auto it = PSOs.find(PSOname);
	if (it == PSOs.end())
		throw misc::customException("PSO-ul nu exista sau nu a fost creat");

	return it->second;
}

template <class PSOType>
inline bool PSO_Manager<PSOType>::AddPSO(std::string PSOname, PSOType::Ptr PSO)
{
	auto it = PSOs.find(PSOname);
	if (it != PSOs.end())
		return false;

	PSOs[PSOname] = PSO;

	return true;
}

template <>
inline void PSO_Manager<GraphicsPSO>::LoadPSOs(
	const RS_Manager& rsManager, const ShadersManager& shadersManager, ID3D12Device10* pDevice)
{
	if (!firstLoad)
		return;

	{
		GraphicsPSO::Ptr pso = PSO_Loader::LoadDefaultPSO(shadersManager);
		pso->SetRootSignature(rsManager.GetRootSignature("Default"));
		pso->Finalize(pDevice);

		AddPSO("Default", pso);
	}

	{
		GraphicsPSO::Ptr pso = PSO_Loader::LoadTerrainPSO(shadersManager);
		pso->SetRootSignature(rsManager.GetRootSignature("Default"));
		pso->Finalize(pDevice);

		AddPSO("Terrain", pso);
	}

	{
		GraphicsPSO::Ptr pso = PSO_Loader::LoadSkyBoxPSO(shadersManager);
		pso->SetRootSignature(rsManager.GetRootSignature("Default"));
		pso->Finalize(pDevice);

		AddPSO("SkyBox", pso);
	}

	{
		GraphicsPSO::Ptr pso = PSO_Loader::LoadWaterPSO(shadersManager);
		pso->SetRootSignature(rsManager.GetRootSignature("Water"));
		pso->Finalize(pDevice);

		AddPSO("Water", pso);
	}

	{
		GraphicsPSO::Ptr pso = PSO_Loader::LoadCubeMapTerrainPSO(shadersManager);
		pso->SetRootSignature(rsManager.GetRootSignature("Default"));
		pso->Finalize(pDevice);

		AddPSO("TerrainCubeMap", pso);
	}

	{
		GraphicsPSO::Ptr pso = PSO_Loader::LoadShadowMapPSO(shadersManager);
		pso->SetRootSignature(rsManager.GetRootSignature("Default"));
		pso->Finalize(pDevice);

		AddPSO("ShadowMap", pso);
	}

	{
		GraphicsPSO::Ptr pso = PSO_Loader::LoadTexturePSO(shadersManager);
		pso->SetRootSignature(rsManager.GetRootSignature("Texture"));
		pso->Finalize(pDevice);

		AddPSO("Texture", pso);
	}

	firstLoad = false;
}

template <>
inline void PSO_Manager<RayTracingPSO>::LoadPSOs(
	const RS_Manager& rsManager, const ShadersManager& shadersManager, ID3D12Device10* pDevice)
{
	if (!firstLoad)
		return;

	{
		RayTracingPSO::Ptr pso = PSO_Loader::LoadDefaultRTPSO(pDevice, rsManager, shadersManager);

		AddPSO("DefaultRTPSO", pso);
	}

	firstLoad = false;
}

template <>
inline void PSO_Manager<ComputePSO>::LoadPSOs(
	const RS_Manager& rsManager, const ShadersManager& shadersManager, ID3D12Device10* pDevice)
{
	if (!firstLoad)
		return;
	return;
	{
		ComputePSO::Ptr pso = PSO_Loader::LoadWavesCPSO(shadersManager);
		pso->SetRootSignature(rsManager.GetRootSignature("Waves"));
		pso->Finalize(pDevice);

		AddPSO("Waves", pso);
	}

	firstLoad = false;
}
