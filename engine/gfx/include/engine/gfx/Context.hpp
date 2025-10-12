#pragma once

#include "FrameResources.hpp"
#include "GPUBuffers.hpp"
#include "PipelineState.hpp"
#include "RootSignature.hpp"
#include "Texture.hpp"
#include "d3dx12.h"

#include "engine/core/Settings.hpp"

class ComputeContext;
class GraphicsContext;

class CommandContext
{
public:
	friend class ContextManager;

	using Ptr = std::unique_ptr<CommandContext>;

	CommandContext() = default;

	ID3D12GraphicsCommandList7* GetCommandList() { return pCommandList.Get(); }
	ID3D12CommandAllocator* GetCommandAllocator() { return pCommandAllocator.Get(); }

	GraphicsContext& GetGraphicsContext() { return reinterpret_cast<GraphicsContext&>(*this); }
	ComputeContext& GetComputeContext() { return reinterpret_cast<ComputeContext&>(*this); }

	void Create(D3D12_COMMAND_LIST_TYPE type);
	bool IsReadyOrWait();

	void CopyBuffer(GpuResource& Dest, GpuResource& Src);
	void CopyBufferRegion(GpuResource& Dest, size_t DestOffset, GpuResource& Src, size_t SrcOffset, size_t NumBytes);

	void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void BeginResourceTransition(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);
	void InsertUAVBarrier(GpuResource& Resource, bool FlushImmediate = false);
	void InsertAliasBarrier(GpuResource& Before, GpuResource& After, bool FlushImmediate = false);

	void BindDescriptorHeaps();
	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr);
	void SetDescriptorHeaps(UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[]);
	void SetPipelineState(const PipelineState& pipelineState);

	void Finish(ID3D12CommandQueue* pCommandQueue);
	void End(ID3D12CommandQueue* pCommandQueue);
	void Reset();

	void FlushResourceBarriers(void);

protected:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> pCommandList;
	Microsoft::WRL::ComPtr<ID3D12Fence> pFence;
	HANDLE m_fenceEvent;

	ID3D12RootSignature* m_CurGraphicsRootSignature;
	ID3D12RootSignature* m_CurComputeRootSignature;
	ID3D12PipelineState* m_CurPipelineState;

	D3D12_COMMAND_LIST_TYPE m_Type;

	D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
	UINT m_NumBarriersToFlush;

	ID3D12DescriptorHeap* m_CurrentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	UINT64 m_fenceValue;
};

class GraphicsContext : public CommandContext
{
public:
	using Ptr = std::unique_ptr<GraphicsContext>;

	void SetRootSignature(const RootSignature& RootSig);

	void SetRenderTargetAndDepthStencil(const engine::gfx::DescriptorHandle& RTV, const engine::gfx::DescriptorHandle& DSV);
	void SetRenderTarget(const engine::gfx::DescriptorHandle& RTV);
	void SetDepthStencil(const engine::gfx::DescriptorHandle& DSV);

	void ClearColor(const ColorTexture& colorTexture, D3D12_RECT* rect = nullptr);
	void ClearCubeMapFace(const ColorTexture& cubeMap, UINT index, D3D12_RECT* rect = nullptr);
	void ClearDepth(const DepthTexture& depthTexture);
	void ClearStencil(const DepthTexture& depthTexture);
	void ClearDepthAndStencil(const DepthTexture& depthTexture);

	void SetViewport(const D3D12_VIEWPORT& vp);
	void SetViewport(FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth = 0.0f, FLOAT maxDepth = 1.0f);
	void SetScissor(const D3D12_RECT& rect);
	void SetScissor(UINT left, UINT top, UINT right, UINT bottom);
	void SetViewportAndScissor(const D3D12_VIEWPORT& vp, const D3D12_RECT& rect);
	void SetViewportAndScissor(UINT x, UINT y, UINT w, UINT h);

	void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);
	void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV);
	void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle);
	void SetDescriptorTable(UINT RootIndex, D3D12_DESCRIPTOR_HEAP_TYPE type);
	void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView);
	void SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView);
	void SetVertexBuffers(UINT StartSlot, UINT Count, const D3D12_VERTEX_BUFFER_VIEW VBViews[]);

	void Draw(UINT VertexCount, UINT VertexStartOffset = 0);
	void DrawIndexed(UINT IndexCount, UINT StartIndexLocation = 0, INT BaseVertexLocation = 0);
	void DrawInstanced(
		UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation = 0, UINT StartInstanceLocation = 0);
	void DrawIndexedInstanced(
		UINT IndexCountPerInstance,
		UINT InstanceCount,
		UINT StartIndexLocation,
		INT BaseVertexLocation,
		UINT StartInstanceLocation);

	void BuildRaytracingAccelerationStructure(
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc,
		UINT NumPostbuildInfoDescs = 0,
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs = nullptr);
};

class ComputeContext : public CommandContext
{
public:
	using Ptr = std::unique_ptr<ComputeContext>;

	void SetRootSignature(const RootSignature& RootSig);
	void SetPipelineStateObject(const PipelineState& pipelineState);

	void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle);
	void SetDescriptorTable(UINT RootIndex, D3D12_DESCRIPTOR_HEAP_TYPE type);
	void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV);
	void SetShaderResourceView(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS SRV);

	void DispatchRays(D3D12_DISPATCH_RAYS_DESC* desc);
};

class ContextManager
{
public:
	ContextManager() = default;

	using Ptr = std::unique_ptr<ContextManager>;

	void Create();

	inline GraphicsContext& GetGraphicsContext() { return *m_graphicsContexts[frameIndex]; }
	inline const GraphicsContext& GetGraphicsContext() const { return *m_graphicsContexts[frameIndex]; }
	inline ComputeContext& GetComputeContext() { return *m_computeContexts[frameIndex]; }
	inline const ComputeContext& GetComputeContext() const { return *m_computeContexts[frameIndex]; }

	inline FrameResources& GetFrameResources() { return *m_frameResources[frameIndex]; }
	inline const FrameResources& GetFrameResources() const { return *m_frameResources[frameIndex]; }
	inline FrameResources& GetFrameResources(UINT index) { return *m_frameResources[index]; }
	inline const FrameResources& GetFrameResources(UINT index) const { return *m_frameResources[index]; }

	ID3D12CommandQueue* GetCommandQueue() { return pCommandQueue.Get(); }
	UINT& GetFrameIndex() { return frameIndex; }

	void Flush(bool waitForCompletition);
	void End();
	void SwapContext();

private:
	std::array<GraphicsContext::Ptr, engine::core::Settings::GetFrameResourcesCount()> m_graphicsContexts;
	std::array<ComputeContext::Ptr, engine::core::Settings::GetFrameResourcesCount()> m_computeContexts;
	std::array<FrameResources::Ptr, engine::core::Settings::GetFrameResourcesCount()> m_frameResources;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	UINT frameIndex;
};

////////////////////////////////////////////////////////////////////////////////
// Definire inline-uri
inline void CommandContext::SetPipelineState(const PipelineState& pipelineState)
{
	ID3D12PipelineState* nativeState = pipelineState.GetID3D12PipelineState();
	if (nativeState == m_CurPipelineState)
		return;

	pCommandList->SetPipelineState(nativeState);
	m_CurPipelineState = nativeState;
}

inline void CommandContext::FlushResourceBarriers(void)
{
	if (m_NumBarriersToFlush > 0)
	{
		pCommandList->ResourceBarrier(m_NumBarriersToFlush, m_ResourceBarrierBuffer);
		m_NumBarriersToFlush = 0;
	}
}

inline void GraphicsContext::SetRootSignature(const RootSignature& rs)
{
	if (rs.GetID3D12RootSignature() == m_CurGraphicsRootSignature)
		return;

	pCommandList->SetGraphicsRootSignature(m_CurGraphicsRootSignature = rs.GetID3D12RootSignature());
}

inline void ComputeContext::SetRootSignature(const RootSignature& rs)
{
	if (rs.GetID3D12RootSignature() == m_CurComputeRootSignature)
		return;

	pCommandList->SetComputeRootSignature(m_CurComputeRootSignature = rs.GetID3D12RootSignature());
}

inline void ComputeContext::SetPipelineStateObject(const PipelineState& pipelineState)
{
	pCommandList->SetPipelineState1(pipelineState.GetID3D12StateObject());
}

inline void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, ID3D12DescriptorHeap* HeapPtr)
{
	if (m_CurrentDescriptorHeaps[Type] != HeapPtr)
	{
		m_CurrentDescriptorHeaps[Type] = HeapPtr;
		BindDescriptorHeaps();
	}
}

inline void CommandContext::SetDescriptorHeaps(
	UINT HeapCount, D3D12_DESCRIPTOR_HEAP_TYPE Type[], ID3D12DescriptorHeap* HeapPtrs[])
{
	bool AnyChanged = false;

	for (UINT i = 0; i < HeapCount; ++i)
	{
		if (m_CurrentDescriptorHeaps[Type[i]] != HeapPtrs[i])
		{
			m_CurrentDescriptorHeaps[Type[i]] = HeapPtrs[i];
			AnyChanged = true;
		}
	}

	if (AnyChanged)
		BindDescriptorHeaps();
}

inline void GraphicsContext::SetViewportAndScissor(UINT x, UINT y, UINT w, UINT h)
{
	SetViewport((float)x, (float)y, (float)w, (float)h);
	SetScissor(x, y, x + w, y + h);
}

inline void GraphicsContext::SetScissor(UINT left, UINT top, UINT right, UINT bottom)
{
	SetScissor(CD3DX12_RECT(left, top, right, bottom));
}

inline void GraphicsContext::SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV)
{
	pCommandList->SetGraphicsRootConstantBufferView(RootIndex, CBV);
}

inline void GraphicsContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle)
{
	pCommandList->SetGraphicsRootDescriptorTable(RootIndex, FirstHandle);
}

inline void GraphicsContext::SetDescriptorTable(UINT RootIndex, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	this->SetDescriptorTable(RootIndex, m_CurrentDescriptorHeaps[type]->GetGPUDescriptorHandleForHeapStart());
}

inline void GraphicsContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView)
{
	pCommandList->IASetIndexBuffer(&IBView);
}

inline void GraphicsContext::SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView)
{
	SetVertexBuffers(Slot, 1, &VBView);
}

inline void GraphicsContext::SetVertexBuffers(UINT StartSlot, UINT Count, const D3D12_VERTEX_BUFFER_VIEW VBViews[])
{
	pCommandList->IASetVertexBuffers(StartSlot, Count, VBViews);
}

inline void GraphicsContext::Draw(UINT VertexCount, UINT VertexStartOffset)
{
	DrawInstanced(VertexCount, 1, VertexStartOffset, 0);
}

inline void GraphicsContext::DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
}

inline void GraphicsContext::DrawInstanced(
	UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
	FlushResourceBarriers();
	pCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

inline void GraphicsContext::DrawIndexedInstanced(
	UINT IndexCountPerInstance,
	UINT InstanceCount,
	UINT StartIndexLocation,
	INT BaseVertexLocation,
	UINT StartInstanceLocation)
{
	FlushResourceBarriers();
	pCommandList->DrawIndexedInstanced(
		IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

inline void GraphicsContext::BuildRaytracingAccelerationStructure(
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc,
	UINT NumPostbuildInfoDescs,
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs)
{
	pCommandList->BuildRaytracingAccelerationStructure(pDesc, NumPostbuildInfoDescs, pPostbuildInfoDescs);
}

inline void GraphicsContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology)
{
	pCommandList->IASetPrimitiveTopology(Topology);
}

inline void ComputeContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle)
{
	pCommandList->SetComputeRootDescriptorTable(RootIndex, FirstHandle);
}

inline void ComputeContext::SetDescriptorTable(UINT RootIndex, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	this->SetDescriptorTable(RootIndex, m_CurrentDescriptorHeaps[type]->GetGPUDescriptorHandleForHeapStart());
}

inline void ComputeContext::SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV)
{
	pCommandList->SetComputeRootConstantBufferView(RootIndex, CBV);
}

inline void ComputeContext::SetShaderResourceView(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS SRV)
{
	pCommandList->SetComputeRootShaderResourceView(RootIndex, SRV);
}

inline void ComputeContext::DispatchRays(D3D12_DISPATCH_RAYS_DESC* desc)
{
	pCommandList->DispatchRays(desc);
}
