#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Context.hpp"
#include "FrameResources.hpp"
#include "engine/core/DxgiInfoManager.hpp"
#include "Texture.hpp"

class GraphicsResources
{
public:
	static GraphicsResources& GetInstance();
	static void DestroyInstance();

	void OnSizeChanged(UINT width, UINT height);
	void CreateWindowSizeDependentResources();
	void CreateDeviceResources();

	void BeginFrame();
	void EndFrame();
	void SetRenderTarget();

	void LoadResources(
		HWND hWnd,
		D3D_FEATURE_LEVEL minimalFeatureLevel = D3D_FEATURE_LEVEL_12_2,
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);

	void UpdateViewportAndScissor(UINT width, UINT height);
	void LoadSizeDependentResoruces(UINT width, UINT height);

	inline GraphicsContext& GetGraphicsContext() { return pContextManager->GetGraphicsContext(); }
	inline ComputeContext& GetComputeContext() { return pContextManager->GetComputeContext(); }
	inline FrameResources& GetFrameResources(UINT index) { return pContextManager->GetFrameResources(index); }
	inline FrameResources& GetFrameResources() { return pContextManager->GetFrameResources(); }

	static inline ID3D12CommandQueue* GetCommandQueue() { return GetInstance().pContextManager->GetCommandQueue(); }
	static inline ContextManager& GetContextManager() { return *GetInstance().pContextManager; }
	static inline ID3D12Device10* GetDevice() { return GetInstance().pDevice.Get(); }

	static misc::DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	inline const misc::DescriptorHeap& GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	inline UINT GetDescriptorIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

	void SetSizeChnaged(bool toSet) { m_sizeChanged = toSet; }
	bool GetSizeChnaged() const { return m_sizeChanged; }

	~GraphicsResources();

private:
	using Ptr = std::unique_ptr<GraphicsResources>;

	GraphicsResources() = default;

private:
	ContextManager::Ptr pContextManager;

	HWND m_hWnd;

	bool m_sizeChanged = false;

	D3D_FEATURE_LEVEL m_minimalFeatureLevel;
	D3D_FEATURE_LEVEL m_featureLevel;

	DXGI_FORMAT m_backBufferFormat;
	DXGI_FORMAT m_depthBufferFormat;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	// Obiecte basic Directx12
	Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain;
	Microsoft::WRL::ComPtr<IDXGIFactory4> pFactory;
	Microsoft::WRL::ComPtr<ID3D12Device10> pDevice;

	// Obiecte Swap-Chain, MSAA, RT
	std::array<ColorTexture::Ptr, Settings::GetBackBufferCount()> m_renderTargetTextures;
	ColorTexture::Ptr m_renderTexture;
	DepthTexture::Ptr m_depthTexture;

	std::array<misc::DescriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_descriptorHeaps;

	UINT m_backBufferIndex;

	static Ptr instance;
};

inline UINT GraphicsResources::GetDescriptorIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return m_descriptorHeaps[heapType].GetDescriptorSize();
}

inline const misc::DescriptorHeap& GraphicsResources::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return m_descriptorHeaps[heapType];
}