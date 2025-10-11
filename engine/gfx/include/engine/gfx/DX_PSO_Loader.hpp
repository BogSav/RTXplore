#pragma once

#include "DX_PSO.hpp"

class ShadersManager;
class RS_Manager;

struct PSO_Loader
{
	PSO_Loader() = delete;

	// Graphics PSOs
	static GraphicsPSO::Ptr LoadDefaultPSO(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadTerrainPSO(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadSkyBoxPSO(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadWaterPSO(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadCubeMapTerrainPSO(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadShadowMapPSO(const ShadersManager& shadersManager);
	static GraphicsPSO::Ptr LoadTexturePSO(const ShadersManager& shadersManager);

	// Compute PSOs
	static ComputePSO::Ptr LoadWavesCPSO(const ShadersManager& shadersManager);

	// PSO-urile de ray tracing sunt modulare, trebuie finalizate direct in functiile de load
	static RayTracingPSO::Ptr LoadDefaultRTPSO(
		ID3D12Device10* pDevice, const RS_Manager& rs_manager, const ShadersManager& shadersManageer);
};