#pragma once

#include "PipelineState.hpp"

namespace engine::gfx
{

class ShadersManager;
class RootSignatureManager;

struct PipelineStateLoader
{
	PipelineStateLoader() = delete;

	// Graphics PSOs
	static GraphicsPSO::Ptr LoadDefaultPipelineState(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadTerrainPipelineState(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadSkyBoxPipelineState(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadWaterPipelineState(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadCubeMapTerrainPipelineState(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadShadowMapPipelineState(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadTexturePipelineState(const ShadersManager& shadersManager);

	// Compute PSOs
	static ComputePSO::Ptr LoadWavesComputePipelineState(const ShadersManager& shadersManager);

	// PSO-urile de ray tracing sunt modulare, trebuie finalizate direct in functiile de load
	static RayTracingPSO::Ptr LoadDefaultRayTracingPipelineState(
		ID3D12Device10* pDevice, const RootSignatureManager& rs_manager, const ShadersManager& shadersManageer);
};

}  // namespace engine::gfx
