#include "AccelerationStructures.hpp"

#include "GraphicsResources.hpp"

namespace engine::gfx
{

void BottomLevelAccelerationStructure::Build(
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescriptors,
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
	GraphicsContext& graphicsContext = GraphicsResources::GetInstance().GetGraphicsContext();
	ID3D12Device10* pDevice = GraphicsResources::GetDevice();

	AccelerationStructureBuffers buffers;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	bottomLevelInputs.Flags = buildFlags;
	bottomLevelInputs.NumDescs = (UINT)geometryDescriptors.size();
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = geometryDescriptors.data();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	assert(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

	GpuResource::AllocateUAVBuffer(
		bottomLevelPrebuildInfo.ScratchDataSizeInBytes,
		buffers.scratch,
		D3D12_RESOURCE_STATE_COMMON,
		L"ScratchResource");

	{
		m_UsageState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

		GpuResource::AllocateUAVBuffer(
			bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, *this, m_UsageState, L"BottomLevelAccelerationStructure");

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
	}

	bottomLevelBuildDesc.DestAccelerationStructureData = m_pResource->GetGPUVirtualAddress();
	bottomLevelBuildDesc.ScratchAccelerationStructureData = buffers.scratch->GetGPUVirtualAddress();

	graphicsContext.BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
	graphicsContext.InsertUAVBarrier(*this, true);

	GraphicsResources::GetContextManager().Flush(true);
	GraphicsResources::GetContextManager().GetGraphicsContext().Reset();
}

void TopLevelAccelerationStructure::Build(
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC>& instanceDescriptors,
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
	GraphicsContext& graphicsContext = GraphicsResources::GetInstance().GetGraphicsContext();

	AccelerationStructureBuffers buffers;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = buildFlags;
	topLevelInputs.NumDescs = (UINT)instanceDescriptors.size();
	topLevelInputs.pGeometryDescs = nullptr;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	GraphicsResources::GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(
		&topLevelInputs, &topLevelPrebuildInfo);
	assert(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

	GpuResource::AllocateUAVBuffer(
		topLevelPrebuildInfo.ScratchDataSizeInBytes, buffers.scratch, D3D12_RESOURCE_STATE_COMMON, L"ScratchBuffer");

	{
		m_UsageState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

		GpuResource::AllocateUAVBuffer(
			topLevelPrebuildInfo.ResultDataMaxSizeInBytes, *this, m_UsageState, L"TopLevelAccelerationStructure");

		m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
	}

	GpuResource::AllocateUploadBuffer(
		reinterpret_cast<const void*>(instanceDescriptors.data()),
		sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instanceDescriptors.size(),
		buffers.instanceDesc,
		L"InstanceDescs");

	topLevelBuildDesc.DestAccelerationStructureData = m_pResource->GetGPUVirtualAddress();
	topLevelBuildDesc.ScratchAccelerationStructureData = buffers.scratch->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs.InstanceDescs = buffers.instanceDesc->GetGPUVirtualAddress();

	graphicsContext.BuildRaytracingAccelerationStructure(&topLevelBuildDesc);
	graphicsContext.InsertUAVBarrier(*this, true);

	GraphicsResources::GetContextManager().Flush(true);
	GraphicsResources::GetContextManager().GetGraphicsContext().Reset();
}

void TopLevelAccelerationStructure::CreateSRV()
{
	m_SRVHandle = GpuResource::CreateAccelerationStructureSRV(*this);
}

}  // namespace engine::gfx
