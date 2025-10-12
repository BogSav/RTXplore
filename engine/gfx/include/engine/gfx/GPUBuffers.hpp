#pragma once

#include "engine/core/Exceptions.hpp"
#include "Utilities.hpp"
#include "engine/core/DxgiInfoManager.hpp"
#include "engine/core/GraphicsThrowMacros.hpp"

#include <type_traits>
#include <variant>

class GpuResource
{
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;

public:
	static void AllocateUAVBuffer(
		UINT64 bufferSize,
		GpuResource& resource,
		D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON,
		std::wstring resourceName = L"");
	static void AllocateUploadBuffer(
		const void* pData, UINT64 datasize, GpuResource& resource, std::wstring resourceName = L"");
	static void AllocateDefaultBuffer(
		const void* pData,
		UINT64 datasize,
		GpuResource& defaultResource,
		GpuResource& uploadResource,
		D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON,
		std::wstring defaultResourceName = L"",
		std::wstring uploadResourceName = L"");

	static engine::gfx::DescriptorHandle CreateTextureView(GpuResource& texture, D3D12_SHADER_RESOURCE_VIEW_DESC* descriptor);
	static engine::gfx::DescriptorHandle CreateTextureView(GpuResource& texture, D3D12_UNORDERED_ACCESS_VIEW_DESC* descriptor);
	static engine::gfx::DescriptorHandle CreateTextureView(GpuResource& texture, D3D12_RENDER_TARGET_VIEW_DESC* descriptor);
	static engine::gfx::DescriptorHandle CreateTextureView(GpuResource& texture, D3D12_DEPTH_STENCIL_VIEW_DESC* descriptor);

	static engine::gfx::DescriptorHandle CreateBufferSRV(GpuResource& buffer, UINT numElements, UINT elementSize);

	static engine::gfx::DescriptorHandle CreateAccelerationStructureSRV(GpuResource& accelerationStructure);

public:
	GpuResource()
		: m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		  m_UsageState(D3D12_RESOURCE_STATE_COMMON),
		  m_TransitioningState((D3D12_RESOURCE_STATES)-1)
	{
	}

	GpuResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES CurrentState)
		: m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
		  m_pResource(pResource),
		  m_UsageState(CurrentState),
		  m_TransitioningState((D3D12_RESOURCE_STATES)-1)
	{
	}

	~GpuResource() { Destroy(); }

	virtual void Destroy()
	{
		m_pResource = nullptr;
		m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	}

	ID3D12Resource* operator->() { return m_pResource.Get(); }
	const ID3D12Resource* operator->() const { return m_pResource.Get(); }

	ID3D12Resource* GetResource() { return m_pResource.Get(); }
	const ID3D12Resource* GetResource() const { return m_pResource.Get(); }

	ID3D12Resource** GetAddressOf() { return m_pResource.GetAddressOf(); }

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
	D3D12_RESOURCE_STATES m_UsageState;
	D3D12_RESOURCE_STATES m_TransitioningState;
	D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
};


class GpuUploadBuffer
{
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() { return m_resource; }

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;

	GpuUploadBuffer() {}
	~GpuUploadBuffer()
	{
		if (m_resource.Get())
		{
			m_resource->Unmap(0, nullptr);
		}
	}

	void Allocate(ID3D12Device* device, UINT bufferSize, LPCWSTR resourceName = nullptr)
	{
		HRESULT hr;

		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
		GFX_THROW_INFO(device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_resource)));
		m_resource->SetName(resourceName);
	}

	uint8_t* MapCpuWriteOnly()
	{
		HRESULT hr;

		uint8_t* mappedData;
		CD3DX12_RANGE readRange(0, 0);
		GFX_THROW_INFO(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));

		return mappedData;
	}
};

template <class T>
class ConstantBuffer : public GpuUploadBuffer
{
	uint8_t* m_mappedConstantData;
	UINT m_alignedInstanceSize;
	UINT m_numInstances;

public:
	ConstantBuffer() : m_alignedInstanceSize(), m_numInstances(0), m_mappedConstantData(nullptr) {}

	void Create(ID3D12Device* device, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
	{
		m_numInstances = numInstances;
		m_alignedInstanceSize = engine::gfx::Align(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		UINT bufferSize = numInstances * m_alignedInstanceSize;
		Allocate(device, bufferSize, resourceName);
		m_mappedConstantData = MapCpuWriteOnly();
	}

	void CopyStagingToGpu(UINT instanceIndex = 0)
	{
		memcpy(m_mappedConstantData + instanceIndex * m_alignedInstanceSize, &staging, sizeof(T));
	}

	void CopyData(UINT instanceIndex, const T& data)
	{
		memcpy(m_mappedConstantData + instanceIndex * m_alignedInstanceSize, &data, sizeof(T));
	}

	// Accessors
	T staging;
	T* operator->() { return &staging; }
	UINT NumInstances() { return m_numInstances; }

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAdress(UINT instanceIndex = 0) const
	{
		return m_resource->GetGPUVirtualAddress() + instanceIndex * m_alignedInstanceSize;
	}
};

// Helper class to create and update a structured buffer.
// Usage: ToDo
//    ConstantBuffer<...> cb;
//    cb.Create(...);
//    cb.staging.var = ...; | cb->var = ... ;
//    cb.CopyStagingToGPU(...);
template <class T>
class StructuredBuffer : public GpuUploadBuffer
{
	T* m_mappedBuffers;
	std::vector<T> m_staging;
	UINT m_numInstances;

public:
	static_assert(sizeof(T) % 16 == 0, "Align structure buffers on 16 byte boundary for performance reasons.");

	StructuredBuffer() : m_mappedBuffers(nullptr), m_numInstances(0) {}

	void Create(ID3D12Device* device, UINT numElements, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
	{
		m_staging.resize(numElements);
		UINT bufferSize = numInstances * numElements * sizeof(T);
		Allocate(device, bufferSize, resourceName);
		m_mappedBuffers = reinterpret_cast<T*>(MapCpuWriteOnly());
	}

	void CopyStagingToGpu(UINT instanceIndex = 0)
	{
		memcpy(m_mappedBuffers + instanceIndex, &m_staging[0], InstanceSize());
	}

	// Accessors
	T& operator[](UINT elementIndex) { return m_staging[elementIndex]; }
	size_t NumElementsPerInstance() { return m_staging.size(); }
	UINT NumInstances() { return m_staging.size(); }
	size_t InstanceSize() { return NumElementsPerInstance() * sizeof(T); }
	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
	{
		return m_resource->GetGPUVirtualAddress() + instanceIndex * InstanceSize();
	}
};

class VertexBuffer : public GpuResource
{
public:
	using Ptr = std::unique_ptr<VertexBuffer>;

	VertexBuffer() = default;

	void Create(const Mesh& mesh);
	inline const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() noexcept { return m_vertexBufferView; }

	void AllocateSRV();
	inline const engine::gfx::DescriptorHandle& GetSRVHandle() const { return m_SRVHandle; }

private:
	GpuResource m_uploadResource;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	UINT m_vertexBufferSize;
	UINT m_vertexCount;

	engine::gfx::DescriptorHandle m_SRVHandle;
};

class IndexBuffer : public GpuResource
{
public:
	using Ptr = std::unique_ptr<IndexBuffer>;

	IndexBuffer() = default;

	void Create(const Mesh& mesh);
	inline const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() noexcept { return m_indexBufferView; }

	void AllocateSRV();
	inline const engine::gfx::DescriptorHandle& GetSRVHandle() const { return m_SRVHandle; }

private:
	GpuResource m_uploadResource;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	UINT m_indexBufferSize;
	UINT m_indexCount;

	engine::gfx::DescriptorHandle m_SRVHandle;
};