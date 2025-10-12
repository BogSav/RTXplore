#include "GPUBuffers.hpp"

#include "Context.hpp"
#include "GraphicsResources.hpp"

void GpuResource::AllocateUAVBuffer(
	UINT64 bufferSize, GpuResource& resource, D3D12_RESOURCE_STATES initialResourceState, std::wstring resourceName)
{
	HRESULT hr;

	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	GFX_THROW_INFO(GraphicsResources::GetDevice()->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(resource.GetAddressOf())));

	if (!resourceName.empty())
	{
		resource->SetName(resourceName.c_str());
	}
}

void GpuResource::AllocateUploadBuffer(
	const void* pData, UINT64 datasize, GpuResource& resource, std::wstring resourceName)
{
	HRESULT hr;

	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);

	GFX_THROW_INFO(GraphicsResources::GetDevice()->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(resource.GetAddressOf())));

	resource.m_UsageState = D3D12_RESOURCE_STATE_GENERIC_READ;
	resource.m_GpuVirtualAddress = resource.GetResource()->GetGPUVirtualAddress();

	if (!resourceName.empty())
	{
		resource->SetName(resourceName.c_str());
	}

	if (pData)
	{
		void* pMappedData;
		resource->Map(0, nullptr, &pMappedData);
		memcpy(pMappedData, pData, datasize);
		resource->Unmap(0, nullptr);
	}
}

void GpuResource::AllocateDefaultBuffer(
	const void* pData,
	UINT64 datasize,
	GpuResource& defaultResource,
	GpuResource& uploadResource,
	D3D12_RESOURCE_STATES resourceState,
	std::wstring defaultResourceName,
	std::wstring uploadResourceName)
{
	HRESULT hr;

	AllocateUploadBuffer(pData, datasize, uploadResource, uploadResourceName);

	CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	auto defaultDesc = CD3DX12_RESOURCE_DESC::Buffer(datasize);

	GFX_THROW_INFO(GraphicsResources::GetDevice()->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&defaultDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultResource.GetAddressOf())));

	defaultResource.m_UsageState = D3D12_RESOURCE_STATE_COMMON;

	if (!uploadResourceName.empty())
	{
		uploadResource->SetName(uploadResourceName.c_str());
	}

	GraphicsResources::GetContextManager().GetGraphicsContext().CopyBuffer(defaultResource, uploadResource);
	GraphicsResources::GetContextManager().GetGraphicsContext().TransitionResource(defaultResource, resourceState);

	// GraphicsResources::GetContextManager().End();
	GraphicsResources::GetContextManager().Flush(true);
	GraphicsResources::GetContextManager().GetGraphicsContext().Reset();
}

misc::DescriptorHandle GpuResource::CreateTextureView(GpuResource& texture, D3D12_SHADER_RESOURCE_VIEW_DESC* descriptor)
{
	assert(descriptor != nullptr);

	misc::DescriptorHandle handle = GraphicsResources::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	GraphicsResources::GetDevice()->CreateShaderResourceView(texture.GetResource(), descriptor, handle);

	return handle;
}

misc::DescriptorHandle GpuResource::CreateTextureView(
	GpuResource& texture, D3D12_UNORDERED_ACCESS_VIEW_DESC* descriptor)
{
	assert(descriptor != nullptr);

	misc::DescriptorHandle handle = GraphicsResources::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	GraphicsResources::GetDevice()->CreateUnorderedAccessView(texture.GetResource(), nullptr, descriptor, handle);

	return handle;
}

misc::DescriptorHandle GpuResource::CreateTextureView(GpuResource& texture, D3D12_RENDER_TARGET_VIEW_DESC* descriptor)
{
	assert(descriptor != nullptr);

	misc::DescriptorHandle handle = GraphicsResources::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	GraphicsResources::GetDevice()->CreateRenderTargetView(texture.GetResource(), descriptor, handle);

	return handle;
}

misc::DescriptorHandle GpuResource::CreateTextureView(GpuResource& texture, D3D12_DEPTH_STENCIL_VIEW_DESC* descriptor)
{
	assert(descriptor != nullptr);

	misc::DescriptorHandle handle = GraphicsResources::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	GraphicsResources::GetDevice()->CreateDepthStencilView(texture.GetResource(), descriptor, handle);

	return handle;
}

misc::DescriptorHandle GpuResource::CreateBufferSRV(GpuResource& buffer, UINT numElements, UINT elementSize)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
	if (elementSize == 0)
	{
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else
	{
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = elementSize;
	}

	misc::DescriptorHandle handle = GraphicsResources::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	GraphicsResources::GetDevice()->CreateShaderResourceView(buffer.GetResource(), &srvDesc, handle);

	return handle;
}

misc::DescriptorHandle GpuResource::CreateAccelerationStructureSRV(GpuResource& accelerationStructure)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = accelerationStructure->GetGPUVirtualAddress();

	misc::DescriptorHandle handle = GraphicsResources::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	GraphicsResources::GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, handle);

	return handle;
}

void VertexBuffer::Create(const Mesh& mesh)
{
	m_vertexBufferSize = (UINT)mesh.GetVerticesDataSize();
	m_vertexCount = (UINT)mesh.GetVertexCount();

	GpuResource::AllocateDefaultBuffer(
		reinterpret_cast<const void*>(mesh.GetVerticesData()),
		mesh.GetVerticesDataSize(),
		*this,
		m_uploadResource,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	m_vertexBufferView.BufferLocation = m_pResource->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = (UINT)Mesh::GetSizeOfVertex();
	m_vertexBufferView.SizeInBytes = m_vertexBufferSize;
}

void VertexBuffer::AllocateSRV()
{
	m_SRVHandle = GpuResource::CreateBufferSRV(*this, m_vertexCount, m_vertexBufferView.StrideInBytes);
}

void IndexBuffer::Create(const Mesh& mesh)
{
	m_indexBufferSize = (UINT)mesh.GetIndicesDataSize();
	m_indexCount = (UINT)mesh.GetIndexCount();

	GpuResource::AllocateDefaultBuffer(
		reinterpret_cast<const void*>(mesh.GetIndicesData()),
		mesh.GetIndicesDataSize(),
		*this,
		m_uploadResource,
		D3D12_RESOURCE_STATE_INDEX_BUFFER);

	m_indexBufferView.BufferLocation = m_pResource->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = m_indexBufferSize;
}

void IndexBuffer::AllocateSRV()
{
	m_SRVHandle = GpuResource::CreateBufferSRV(*this, m_indexCount / 4, 0);
}
