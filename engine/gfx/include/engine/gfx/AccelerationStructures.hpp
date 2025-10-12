#pragma once

#include "GPUBuffers.hpp"

struct AccelerationStructureBuffers
{
	GpuResource scratch;
	GpuResource instanceDesc;  // Used only for top-level AS
};

class BottomLevelAccelerationStructure : public GpuResource
{
public:
	BottomLevelAccelerationStructure() = default;

	void Build(
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescriptors,
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags =
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE);

private:
};

class TopLevelAccelerationStructure : public GpuResource
{
public:
	TopLevelAccelerationStructure() = default;

	void Build(
		std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instanceDescriptors,
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags =
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE);
	void CreateSRV();

	inline const engine::gfx::DescriptorHandle& GetSRVHandle() const { return m_SRVHandle; }
	inline engine::gfx::DescriptorHandle& GetSRVHandle() { return m_SRVHandle; }

private:
	engine::gfx::DescriptorHandle m_SRVHandle;
};
