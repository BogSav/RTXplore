#pragma once

#include "ShaderTable.hpp"
#include "Graphics.hpp"

namespace engine::gfx
{

class RayTracingGraphics : public Graphics
{
public:
	using Ptr = std::unique_ptr<RayTracingGraphics>;

	RayTracingGraphics(GraphicsResources& resources);
	~RayTracingGraphics();

	void Update(float deltaTime, float totalTime) override;

private:
	void PopulateCommandList() override;
	void LoadAssets() override;

	void CreateTopLevelAccelerationStructure();
	void CreateShaderTable();

private:
	PipelineStateManager<RayTracingPSO> m_rayTracingPipelineStateManager;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_missShaderTable;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_hitGroupShaderTable;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_rayGenShaderTable;

	TopLevelAccelerationStructure m_topLevelAccelerationStructure;

	RayTracingPSO::Ptr m_rayTracingPipelineState;
	RootSignature::Ptr pGlobalRTRS;

	UINT64 m_missShaderTableStride;
	UINT64 m_hitGroupShaderTableStride;
};

}  // namespace engine::gfx
