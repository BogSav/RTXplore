#pragma once

#include "DX_GPUBuffers.hpp"
#include "DX_Utilities.hpp"

class ObjectRenderer;
class Object;
class WaterRenderer;
class SkyBoxRenderer;
class TerrainRenderer;
class BaseCamera;
class CameraController;
class GraphicsResources;
class DynamicCubeMap;
class MaterialManager;
class ShadowMap;
class LightSource;
class PerspectiveCamera;
class RasterizationGraphics;
class RayTracingGraphics;

class FrameResources
{
public:
	using Ptr = std::unique_ptr<FrameResources>;

public:
	FrameResources() = default;

	void Create();

	static UINT GetObjectCB_ID() { return objectCB_ID++; }
	static UINT GetMaterialCB_ID() { return materialCB_ID++; }

	// Functii de actualizare CB
	void UpdateObjectRendererCB(const ObjectRenderer& gameComponentRenderer);
	void UpdatePerObjectCB(const Object& object);
	void UpdatePerMaterialCB(const MaterialManager& materialManager);

	void UpdateTerrainCB(const TerrainRenderer& terrain);
	void UpdateSkyBoxCB(const SkyBoxRenderer& skyBox);
	void UpdateWaterCB(const WaterRenderer& water);

	void UpdateShadowMapPassCB(const ShadowMap& shadowMap);
	void UpdateDyanmicCubeMapPassCB(const DynamicCubeMap& dynamicCubeMap);
	void UpdateMainPassCB(
		const BaseCamera& camera,
		const float& deltaTime,
		const float& totalTime,
		const UINT& renderMode,
		std::vector<LightSource>& lightSources);

private:
	friend TerrainRenderer;
	friend ObjectRenderer;
	friend Object;
	friend SkyBoxRenderer;
	friend WaterRenderer;
	friend RasterizationGraphics;
	friend RayTracingGraphics;

	void UpdateCameraAndTransferToGPUPassCB(const BaseCamera& camera, int i = 0);

	ConstantBuffer<MaterialProperties> m_perMaterialCB;
	ConstantBuffer<ObjectConstantBuffer> m_perObjectCB;
	ConstantBuffer<PassConstantBuffer> m_perPassCB;
	ConstantBuffer<WaterConstantBuffer> m_waterCB;

private:
	static UINT objectCB_ID;
	static UINT materialCB_ID;
};