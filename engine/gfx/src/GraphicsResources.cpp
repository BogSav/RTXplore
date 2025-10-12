#include "GraphicsResources.hpp"

#include "FrameResources.hpp"
#include "engine/core/DxgiInfoManager.hpp"
#include "engine/core/Utilities.hpp"

using namespace Microsoft::WRL;

std::unique_ptr<GraphicsResources> GraphicsResources::instance = nullptr;

GraphicsResources::~GraphicsResources()
{
	pContextManager->End();
}

void GraphicsResources::BeginFrame()
{
	GraphicsContext& graphicsContext = GetGraphicsContext();
	// ComputeContext& computeContext = GetComputeContext();

	graphicsContext.Reset();
	// computeContext.Reset();
}

void GraphicsResources::SetRenderTarget()
{
	GraphicsContext& graphicsContext = GetGraphicsContext();

	if (Settings::UseRayTracing())
	{
		graphicsContext.TransitionResource(*m_renderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
	}
	else if (Settings::GetGraphicsSettings().GetIsMSAAEnabled())
	{
		graphicsContext.TransitionResource(*m_renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	else
	{
		graphicsContext.TransitionResource(
			*m_renderTargetTextures[m_backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, true);
	}

	if (Settings::UseRayTracing())
		return;

	graphicsContext.SetViewportAndScissor(m_viewport, m_scissorRect);
	graphicsContext.SetRenderTargetAndDepthStencil(
		m_renderTargetTextures[m_backBufferIndex]->GetRtvHandle(), m_depthTexture->GetDsvHandle());

	if (Settings::GetGraphicsSettings().GetIsMSAAEnabled())
	{
		graphicsContext.ClearColor(*m_renderTexture);
	}
	else
	{
		graphicsContext.ClearColor(*m_renderTargetTextures[m_backBufferIndex]);
	}

	graphicsContext.ClearDepthAndStencil(*m_depthTexture);
}

void GraphicsResources::EndFrame()
{
	HRESULT hr;

	GraphicsContext& graphicsContext = GetGraphicsContext();
	// ComputeContext& computeContext = GetComputeContext();

	if (Settings::UseRayTracing())
	{
		graphicsContext.CopyBuffer(*m_renderTargetTextures[m_backBufferIndex], *m_renderTexture);
		graphicsContext.TransitionResource(
			*m_renderTargetTextures[m_backBufferIndex], D3D12_RESOURCE_STATE_PRESENT, true);
	}
	else if (Settings::GetGraphicsSettings().GetIsMSAAEnabled())
	{
		/*ResourceBarriers(
			m_renderTexture->GetResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
			GetRenderTarget(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RESOLVE_DEST);

		pCommandList->ResolveSubresource(GetRenderTarget(), 0, m_renderTexture->GetResource(), 0, m_backBufferFormat);

		ResourceBarriers(GetRenderTarget(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PRESENT);*/
	}
	else
	{
		graphicsContext.TransitionResource(*m_renderTargetTextures[m_backBufferIndex], D3D12_RESOURCE_STATE_PRESENT);
	}

	pContextManager->SwapContext();

#if _DEBUG
	DxgiInfoManager::GetInstance().Set();
#endif

	if (FAILED(
			hr = pSwapChain->Present(
				Settings::GetGraphicsSettings().GetSyncInterval(),
				Settings::GetGraphicsSettings().IsTearingAllowed() ? DXGI_PRESENT_ALLOW_TEARING : 0u)))
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}
		else
		{
			throw GFX_EXCEPT(hr);
		}
	}

	m_backBufferIndex = pSwapChain->GetCurrentBackBufferIndex();
}

//////////////////////////////////////////////////////////////////////////
// Functii pentru crearea resurselor
//////////////////////////////////////////////////////////////////////////
void GraphicsResources::CreateDeviceResources()
{
	HRESULT hr;

	//////////////////////////////////////////////////////////////////////////////////////////////
	// Creare factory cu flag-uri de debug/release
#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}

	UINT compileFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
	UINT compileFlags = 0;
#endif
	GFX_THROW_INFO(CreateDXGIFactory2(compileFlags, IID_PPV_ARGS(&pFactory)));

	//////////////////////////////////////////////////////////////////////////////////////////////
	// Creare componente de baza
	// - setare tearing
	// - identificare adaptor hardware
	// - creare device
	// - verificare suport ray tracing
	// - setare MSAA
	misc::SetTearing(pFactory);

	if (Settings::GetGraphicsSettings().GetUseWarpAdapter())
	{
		ComPtr<IDXGIAdapter1> warpAdapter;
		GFX_THROW_INFO(pFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		GFX_THROW_INFO(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice)));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		misc::GetHardwareAdapter(pFactory.Get(), &hardwareAdapter, m_featureLevel, m_minimalFeatureLevel);

		if (!misc::IsRayTracingSupported(hardwareAdapter.Get(), m_featureLevel) && Settings::UseRayTracing())
			throw misc::customException("RayTracing is not supported");

		GFX_THROW_INFO(D3D12CreateDevice(hardwareAdapter.Get(), m_featureLevel, IID_PPV_ARGS(&pDevice)));
	}

	misc::SetOptimalMSAALevel(pDevice.Get(), m_backBufferFormat);

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Creare descriptor heaps
	m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Create(
		GetDevice(), L"CbvSrvUavHeap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 30);
	m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].Create(
		GetDevice(), L"RtvHeap", D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 10);
	m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].Create(
		GetDevice(), L"DsvHeap", D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 5);

	// Creare command queue si fence events
	pContextManager = std::make_unique<ContextManager>();
	pContextManager->Create();
}

void GraphicsResources::CreateWindowSizeDependentResources()
{
	HRESULT hr;

	// CREARE SWAP CHAIN====================================================================
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = Settings::GetBackBufferCount();
	swapChainDesc.Width = Settings::GetGraphicsSettings().GetWidth();
	swapChainDesc.Height = Settings::GetGraphicsSettings().GetHeight();
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.Format = m_backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Flags = Settings::GetGraphicsSettings().IsTearingAllowed() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

	ComPtr<IDXGISwapChain1> tmpSwapChain;
	GFX_THROW_INFO(pFactory->CreateSwapChainForHwnd(
		pContextManager->GetCommandQueue(), m_hWnd, &swapChainDesc, nullptr, nullptr, &tmpSwapChain));
	GFX_THROW_INFO(tmpSwapChain.As(&pSwapChain));


	m_backBufferIndex = pSwapChain->GetCurrentBackBufferIndex();
	pContextManager->GetFrameIndex() = 0;

	GFX_THROW_INFO(pFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));

	LoadSizeDependentResoruces(Settings::GetGraphicsSettings().GetWidth(), Settings::GetGraphicsSettings().GetHeight());
}

GraphicsResources& GraphicsResources::GetInstance()
{
	if (!instance)
	{
		instance = GraphicsResources::Ptr(new GraphicsResources());
	}

	return *instance;
}

void GraphicsResources::DestroyInstance()
{
	if (!instance)
		delete instance.release();

	instance = nullptr;
}

void GraphicsResources::OnSizeChanged(UINT width, UINT height)
{
	HRESULT hr;

	if (Settings::GetGraphicsSettings().GetWidth() == width && Settings::GetGraphicsSettings().GetHeight() == height)
		return;

	pContextManager->Flush(true);

	DXGI_SWAP_CHAIN_DESC desc = {};
	pSwapChain->GetDesc(&desc);
	GFX_THROW_INFO(
		pSwapChain->ResizeBuffers(Settings::GetBackBufferCount(), width, height, desc.BufferDesc.Format, desc.Flags));

	pContextManager->GetFrameIndex() = pSwapChain->GetCurrentBackBufferIndex();

	Settings::GetGraphicsSettings().SetWidthAndHeight(width, height);

	UpdateViewportAndScissor(width, height);
	LoadSizeDependentResoruces(width, height);

	m_sizeChanged = true;
}

void GraphicsResources::LoadSizeDependentResoruces(UINT width, UINT height)
{
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

	for (UINT i = 0; i < Settings::GetBackBufferCount(); i++)
	{
		m_renderTargetTextures[i].reset(new ColorTexture());
		m_renderTargetTextures[i]->CreateFromSwapChain(pSwapChain.Get(), i);
		m_renderTargetTextures[i]->AllocateRtvHandle();
	}

	if (Settings::UseRayTracing())
	{
		Math::Color clearColor(0.25f, 0.25f, 0.25f, 1.f);

		m_renderTexture.reset(new ColorTexture(clearColor));
		m_renderTexture->Create(
			width,
			height,
			m_backBufferFormat,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
			L"RenderTexture");
		m_renderTexture->AllocateUavHandle();
	}
	else
	{
		if (Settings::GetGraphicsSettings().GetIsZBufferingEnabled())
		{
			m_depthTexture.reset(new DepthTexture(1.f, 0));
			m_depthTexture->Create(width, height, m_depthBufferFormat, L"DepthTexture");
			m_depthTexture->AllocateDsvHandle();
		}

		if (Settings::GetGraphicsSettings().GetIsMSAAEnabled())
		{
			Math::Color clearColor = Math::Color(0x87, 0xCE, 0xEB, 1.f);

			m_renderTexture.reset(new ColorTexture(clearColor));
			m_renderTexture->Create(
				width, height, m_backBufferFormat, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, L"RenderTexture");
			m_renderTexture->AllocateRtvHandle();
		}
	}
}

misc::DescriptorHandle GraphicsResources::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return GetInstance().m_descriptorHeaps[heapType].Alloc();
}

void GraphicsResources::LoadResources(
	HWND hWnd, D3D_FEATURE_LEVEL minimalFeatureLevel, DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat)
{
	m_hWnd = hWnd;
	m_minimalFeatureLevel = minimalFeatureLevel;
	m_backBufferFormat = backBufferFormat;
	m_depthBufferFormat = depthBufferFormat;

	this->UpdateViewportAndScissor(
		Settings::GetGraphicsSettings().GetWidth(), Settings::GetGraphicsSettings().GetHeight());
	this->CreateDeviceResources();
	this->CreateWindowSizeDependentResources();
}

void GraphicsResources::UpdateViewportAndScissor(UINT width, UINT height)
{
	m_viewport.Height = static_cast<float>(height);
	m_viewport.Width = static_cast<float>(width);
	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;

	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = width;
	m_scissorRect.bottom = height;
}