#pragma once

#include "PipelineState.hpp"
#include "RootSignatureManager.hpp"
#include "ShadersManager.hpp"
#include "PipelineStateLoader.hpp"

#include <unordered_map>

namespace engine::gfx
{

template <class PipelineStateType>
class PipelineStateManager
{
public:
	PipelineStateManager() = default;
	~PipelineStateManager();

	PipelineStateType::Ptr GetPipelineState(std::string name);
	bool AddPipelineState(std::string name, PipelineStateType::Ptr pipelineState);

	void LoadPipelineStates(
		const RootSignatureManager& rsManager, const ShadersManager& shadersManager, ID3D12Device10* pDevice);

private:
	std::unordered_map<std::string, typename PipelineStateType::Ptr> m_pipelineStates;
	bool firstLoad = true;
};

//////////////////////////////////////////////////////////////////////////////

template <class PipelineStateType>
PipelineStateManager<PipelineStateType>::~PipelineStateManager()
{
}

template <class PipelineStateType>
inline PipelineStateType::Ptr PipelineStateManager<PipelineStateType>::GetPipelineState(std::string name)
{
	auto it = m_pipelineStates.find(name);
	if (it == m_pipelineStates.end())
		throw engine::core::CustomException("Pipeline state-ul nu exista sau nu a fost creat");

	return it->second;
}

template <class PipelineStateType>
inline bool PipelineStateManager<PipelineStateType>::AddPipelineState(std::string name, PipelineStateType::Ptr pipelineState)
{
	auto it = m_pipelineStates.find(name);
	if (it != m_pipelineStates.end())
		return false;

	m_pipelineStates[name] = pipelineState;

	return true;
}

template <>
inline void PipelineStateManager<GraphicsPSO>::LoadPipelineStates(
	const RootSignatureManager& rsManager, const ShadersManager& shadersManager, ID3D12Device10* pDevice)
{
	if (!firstLoad)
		return;

	{
		GraphicsPSO::Ptr pipelineState = PipelineStateLoader::LoadDefaultPipelineState(shadersManager);
		pipelineState->SetRootSignature(rsManager.GetRootSignature("Default"));
		pipelineState->Finalize(pDevice);

		AddPipelineState("Default", pipelineState);
	}

	{
		GraphicsPSO::Ptr pipelineState = PipelineStateLoader::LoadTerrainPipelineState(shadersManager);
		pipelineState->SetRootSignature(rsManager.GetRootSignature("Default"));
		pipelineState->Finalize(pDevice);

		AddPipelineState("Terrain", pipelineState);
	}

	{
		GraphicsPSO::Ptr pipelineState = PipelineStateLoader::LoadSkyBoxPipelineState(shadersManager);
		pipelineState->SetRootSignature(rsManager.GetRootSignature("Default"));
		pipelineState->Finalize(pDevice);

		AddPipelineState("SkyBox", pipelineState);
	}

	{
		GraphicsPSO::Ptr pipelineState = PipelineStateLoader::LoadWaterPipelineState(shadersManager);
		pipelineState->SetRootSignature(rsManager.GetRootSignature("Water"));
		pipelineState->Finalize(pDevice);

		AddPipelineState("Water", pipelineState);
	}

	{
		GraphicsPSO::Ptr pipelineState = PipelineStateLoader::LoadCubeMapTerrainPipelineState(shadersManager);
		pipelineState->SetRootSignature(rsManager.GetRootSignature("Default"));
		pipelineState->Finalize(pDevice);

		AddPipelineState("TerrainCubeMap", pipelineState);
	}

	{
		GraphicsPSO::Ptr pipelineState = PipelineStateLoader::LoadShadowMapPipelineState(shadersManager);
		pipelineState->SetRootSignature(rsManager.GetRootSignature("Default"));
		pipelineState->Finalize(pDevice);

		AddPipelineState("ShadowMap", pipelineState);
	}

	{
		GraphicsPSO::Ptr pipelineState = PipelineStateLoader::LoadTexturePipelineState(shadersManager);
		pipelineState->SetRootSignature(rsManager.GetRootSignature("Texture"));
		pipelineState->Finalize(pDevice);

		AddPipelineState("Texture", pipelineState);
	}

	firstLoad = false;
}

template <>
inline void PipelineStateManager<RayTracingPSO>::LoadPipelineStates(
	const RootSignatureManager& rsManager, const ShadersManager& shadersManager, ID3D12Device10* pDevice)
{
	if (!firstLoad)
		return;

	{
		RayTracingPSO::Ptr pipelineState = PipelineStateLoader::LoadDefaultRayTracingPipelineState(pDevice, rsManager, shadersManager);

		AddPipelineState("DefaultRTPSO", pipelineState);
	}

	firstLoad = false;
}

template <>
inline void PipelineStateManager<ComputePSO>::LoadPipelineStates(
	const RootSignatureManager& rsManager, const ShadersManager& shadersManager, ID3D12Device10* pDevice)
{
	if (!firstLoad)
		return;
	return;
	{
		ComputePSO::Ptr pipelineState = PipelineStateLoader::LoadWavesComputePipelineState(shadersManager);
		pipelineState->SetRootSignature(rsManager.GetRootSignature("Waves"));
		pipelineState->Finalize(pDevice);

		AddPipelineState("Waves", pipelineState);
	}

	firstLoad = false;
}

}  // namespace engine::gfx
