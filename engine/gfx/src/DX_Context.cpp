#include "DX_Context.hpp"

#include "engine/core/Settings.hpp"
#include "engine/core/DxgiInfoManager.hpp"
#include "GraphicsResources.hpp"

void ContextManager::Create()
{
	HRESULT hr;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.NodeMask = 0;
	GFX_THROW_INFO(GraphicsResources::GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pCommandQueue)));

	frameIndex = 0;

	for (size_t i = 0; i < Settings::GetFrameResourcesCount(); i++)
	{
		m_frameResources[i] = std::make_unique<FrameResources>();
		m_frameResources[i]->Create();

		m_graphicsContexts[i] = std::make_unique<GraphicsContext>();
		m_graphicsContexts[i]->Create(D3D12_COMMAND_LIST_TYPE_DIRECT);

		std::wstring name = L"CommandList_" + std::to_wstring(i);
		m_graphicsContexts[i]->GetCommandList()->SetName(name.c_str());

		m_computeContexts[i] = std::make_unique<ComputeContext>();
		m_computeContexts[i]->Create(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	}
}

void CommandContext::Create(D3D12_COMMAND_LIST_TYPE type)
{
	HRESULT hr;

	m_fenceValue = 0;

	GFX_THROW_INFO(GraphicsResources::GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&pCommandAllocator)));
	GFX_THROW_INFO(GraphicsResources::GetDevice()->CreateCommandList(
		0, type, pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&pCommandList)));
	pCommandList->Close();
	GFX_THROW_INFO(GraphicsResources::GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		GFX_THROW_INFO(HRESULT_FROM_WIN32(GetLastError()));
	}

	m_fenceValue++;
}

bool CommandContext::IsReadyOrWait()
{
	if (pFence->GetCompletedValue() < (m_fenceValue - 1))
	{
		HRESULT hr;

		GFX_THROW_INFO(pFence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

		return false;
	}

	return true;
}

void CommandContext::CopyBuffer(GpuResource& Dest, GpuResource& Src)
{
	TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();
	pCommandList->CopyResource(Dest.GetResource(), Src.GetResource());
}

void CommandContext::CopyBufferRegion(
	GpuResource& Dest, size_t DestOffset, GpuResource& Src, size_t SrcOffset, size_t NumBytes)
{
	TransitionResource(Dest, D3D12_RESOURCE_STATE_COPY_DEST);
	// TransitionResource(Src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();
	pCommandList->CopyBufferRegion(Dest.GetResource(), DestOffset, Src.GetResource(), SrcOffset, NumBytes);
}

void CommandContext::TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate)
{
	D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

	// if (m_Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
	//{
	// assert((OldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == OldState);
	// assert((NewState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == NewState);
	//}

	if (OldState != NewState)
	{
		// assert(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
		D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Transition.pResource = Resource.GetResource();
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = OldState;
		BarrierDesc.Transition.StateAfter = NewState;

		// Check to see if we already started the transition
		if (NewState == Resource.m_TransitioningState)
		{
			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
			Resource.m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
		}
		else
			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		Resource.m_UsageState = NewState;
	}
	else if (NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		InsertUAVBarrier(Resource, FlushImmediate);

	if (FlushImmediate || m_NumBarriersToFlush == 16)
		FlushResourceBarriers();
}

void CommandContext::InsertUAVBarrier(GpuResource& Resource, bool FlushImmediate)
{
	// assert(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
	D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.UAV.pResource = Resource.GetResource();

	if (FlushImmediate)
		FlushResourceBarriers();
}

void CommandContext::InsertAliasBarrier(GpuResource& Before, GpuResource& After, bool FlushImmediate)
{
	// assert(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
	D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Aliasing.pResourceBefore = Before.GetResource();
	BarrierDesc.Aliasing.pResourceAfter = After.GetResource();

	if (FlushImmediate)
		FlushResourceBarriers();
}

void CommandContext::BeginResourceTransition(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate)
{
	// If it's already transitioning, finish that transition
	if (Resource.m_TransitioningState != (D3D12_RESOURCE_STATES)-1)
		TransitionResource(Resource, Resource.m_TransitioningState);

	D3D12_RESOURCE_STATES OldState = Resource.m_UsageState;

	if (OldState != NewState)
	{
		// assert(m_NumBarriersToFlush < 16, "Exceeded arbitrary limit on buffered barriers");
		D3D12_RESOURCE_BARRIER& BarrierDesc = m_ResourceBarrierBuffer[m_NumBarriersToFlush++];

		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Transition.pResource = Resource.GetResource();
		BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		BarrierDesc.Transition.StateBefore = OldState;
		BarrierDesc.Transition.StateAfter = NewState;

		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

		Resource.m_TransitioningState = NewState;
	}

	if (FlushImmediate || m_NumBarriersToFlush == 16)
		FlushResourceBarriers();
}

void CommandContext::BindDescriptorHeaps(void)
{
	UINT NonNullHeaps = 0;
	ID3D12DescriptorHeap* HeapsToBind[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	for (UINT i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		ID3D12DescriptorHeap* HeapIter = m_CurrentDescriptorHeaps[i];
		if (HeapIter != nullptr)
			HeapsToBind[NonNullHeaps++] = HeapIter;
	}

	if (NonNullHeaps > 0)
		pCommandList->SetDescriptorHeaps(NonNullHeaps, HeapsToBind);
}

void CommandContext::Reset()
{
	HRESULT hr;

	GFX_THROW_INFO(pCommandAllocator->Reset());
	GFX_THROW_INFO(pCommandList->Reset(pCommandAllocator.Get(), nullptr));

	m_CurGraphicsRootSignature = nullptr;
	m_CurComputeRootSignature = nullptr;
	m_CurPipelineState = nullptr;
}

void GraphicsContext::SetRenderTargetAndDepthStencil(
	const misc::DescriptorHandle& RTV, const misc::DescriptorHandle& DSV)
{
	pCommandList->OMSetRenderTargets(1, &RTV, TRUE, &DSV);
}

void GraphicsContext::SetRenderTarget(const misc::DescriptorHandle& RTV)
{
	pCommandList->OMSetRenderTargets(1, &RTV, TRUE, nullptr);
}

void GraphicsContext::SetDepthStencil(const misc::DescriptorHandle& DSV)
{
	pCommandList->OMSetRenderTargets(0, nullptr, TRUE, &DSV);
}

void GraphicsContext::ClearColor(const ColorTexture& colorTexture, D3D12_RECT* rect)
{
	FlushResourceBarriers();
	pCommandList->ClearRenderTargetView(
		colorTexture.GetRtvHandle(), colorTexture.GetClearColor().GetPtr(), (rect == nullptr) ? 0 : 1, rect);
}

void GraphicsContext::ClearCubeMapFace(const ColorTexture& cubeMap, UINT index, D3D12_RECT* rect)
{
	FlushResourceBarriers();
	pCommandList->ClearRenderTargetView(
		cubeMap.GetRtvHandle(index), cubeMap.GetClearColor().GetPtr(), (rect == nullptr) ? 0 : 1, rect);
}

void GraphicsContext::ClearDepth(const DepthTexture& depthTexture)
{
	FlushResourceBarriers();
	pCommandList->ClearDepthStencilView(
		depthTexture.GetDsvHandle(), D3D12_CLEAR_FLAG_DEPTH, depthTexture.GetClearDepth(), 0, 0, nullptr);
}

void GraphicsContext::ClearStencil(const DepthTexture& depthTexture)
{
	FlushResourceBarriers();
	pCommandList->ClearDepthStencilView(
		depthTexture.GetDsvHandle(), D3D12_CLEAR_FLAG_STENCIL, 1.f, depthTexture.GetClearStencil(), 0, nullptr);
}

void GraphicsContext::ClearDepthAndStencil(const DepthTexture& depthTexture)
{
	FlushResourceBarriers();
	pCommandList->ClearDepthStencilView(
		depthTexture.GetDsvHandle(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		depthTexture.GetClearDepth(),
		depthTexture.GetClearStencil(),
		0,
		nullptr);
}

void GraphicsContext::SetViewportAndScissor(const D3D12_VIEWPORT& vp, const D3D12_RECT& rect)
{
	assert(rect.left < rect.right && rect.top < rect.bottom);
	pCommandList->RSSetViewports(1, &vp);
	pCommandList->RSSetScissorRects(1, &rect);
}

void GraphicsContext::SetViewport(const D3D12_VIEWPORT& vp)
{
	pCommandList->RSSetViewports(1, &vp);
}

void GraphicsContext::SetViewport(FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth, FLOAT maxDepth)
{
	D3D12_VIEWPORT vp;
	vp.Width = w;
	vp.Height = h;
	vp.MinDepth = minDepth;
	vp.MaxDepth = maxDepth;
	vp.TopLeftX = x;
	vp.TopLeftY = y;
	pCommandList->RSSetViewports(1, &vp);
}

void GraphicsContext::SetScissor(const D3D12_RECT& rect)
{
	assert(rect.left < rect.right && rect.top < rect.bottom);
	pCommandList->RSSetScissorRects(1, &rect);
}

void CommandContext::Finish(ID3D12CommandQueue* pCommandQueue)
{
	HRESULT hr;

	FlushResourceBarriers();

	GFX_THROW_INFO(pCommandList->Close());
	ID3D12CommandList* ppCommandLists[] = {pCommandList.Get()};
	pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	pCommandQueue->Signal(pFence.Get(), m_fenceValue);

	m_fenceValue++;
}

void CommandContext::End(ID3D12CommandQueue* pCommandQueue)
{
	HRESULT hr;

	FlushResourceBarriers();

	GFX_THROW_INFO(pCommandList->Close());
	ID3D12CommandList* ppCommandLists[] = {pCommandList.Get()};
	pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	pCommandQueue->Signal(pFence.Get(), m_fenceValue);

	if (pFence->GetCompletedValue() < m_fenceValue)
	{
		HRESULT hr;

		GFX_THROW_INFO(pFence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	m_fenceValue++;
}

void ContextManager::Flush(bool waitForCompletition)
{
	GetGraphicsContext().Finish(pCommandQueue.Get());

	if (waitForCompletition)
		GetGraphicsContext().IsReadyOrWait();
}

void ContextManager::End()
{
	for (size_t i = 0; i < Settings::GetFrameResourcesCount(); i++)
	{
		m_graphicsContexts[i]->End(pCommandQueue.Get());
	}
}

void ContextManager::SwapContext()
{
	GetGraphicsContext().Finish(pCommandQueue.Get());

	frameIndex = (frameIndex + 1) % Settings::GetFrameResourcesCount();

	GetGraphicsContext().IsReadyOrWait();
}
