#include "Utilities.hpp"

#include "engine/core/DxgiInfoManager.hpp"
#include "engine/core/Settings.hpp"

#include <iomanip>
#include <sstream>

using namespace Microsoft::WRL;

namespace engine::gfx
{

void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;
	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);
	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);
	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text = L"Width = " + std::to_wstring(x.Width) + L" " + L"Height = " + std::to_wstring(x.Height)
			+ L" " + L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) + L" => "
			+ std::to_wstring(n * 1. / d) + L"\n";
		::OutputDebugString(text.c_str());
	}
}
void LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;

	DXGI_ADAPTER_DESC desc;
	adapter->GetDesc(&desc);
	std::wstring text = L"***Adapter: ";
	text += desc.Description;
	text += L"\n";
	OutputDebugString(text.c_str());

	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);
		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());
		LogOutputDisplayModes(output, DXGI_FORMAT_B8G8R8A8_UNORM);
		output->Release();
		++i;
	}
}
void LogAdapters()
{
	HRESULT hr;
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	ComPtr<IDXGIFactory4> dxgiFactory;
	GFX_THROW_INFO(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory)));
	while (dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);
		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";
		OutputDebugString(text.c_str());
		adapterList.push_back(adapter);
		++i;
	}
	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		adapterList[i]->Release();
	}
}

UINT CalculateConstantBufferSize(UINT byteSize)
{
	return (byteSize + 255) & ~255;
}

void GetHardwareAdapter(
	IDXGIFactory4* pFactory,
	IDXGIAdapter1** ppAdapter,
	D3D_FEATURE_LEVEL& outMaxFeatureLevel,
	D3D_FEATURE_LEVEL minimalFeatureLevel)
{
	*ppAdapter = nullptr;
	D3D_FEATURE_LEVEL maxSupportedFeatureLevel = D3D_FEATURE_LEVEL_9_1;

	for (UINT adapterIndex = 0;; ++adapterIndex)
	{
		IDXGIAdapter1* pAdapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter))
		{
			// No more adapters to enumerate.
			break;
		}

		for (auto featureLevel : featureLevels)
		{
			if (SUCCEEDED(D3D12CreateDevice(pAdapter, featureLevel, _uuidof(ID3D12Device), nullptr)))
			{
				if (featureLevel > maxSupportedFeatureLevel)
				{
					maxSupportedFeatureLevel = featureLevel;
					if (*ppAdapter)
					{
						(*ppAdapter)->Release();
					}
					*ppAdapter = pAdapter;
				}
				else
				{
					pAdapter->Release();
				}
				break;
			}
		}
	}

	outMaxFeatureLevel = maxSupportedFeatureLevel;

	if (outMaxFeatureLevel < minimalFeatureLevel)
		throw engine::core::CustomException("Incomaptible feature level");
}

void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc)
{
	std::wstringstream wstr;
	wstr << L"\n";
	wstr << L"--------------------------------------------------------------------\n";
	wstr << L"| D3D12 State Object 0x" << static_cast<const void*>(desc) << L": ";
	if (desc->Type == D3D12_STATE_OBJECT_TYPE_COLLECTION)
		wstr << L"Collection\n";
	if (desc->Type == D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE)
		wstr << L"Raytracing Pipeline\n";

	auto ExportTree = [](UINT depth, UINT numExports, const D3D12_EXPORT_DESC* exports)
	{
		std::wostringstream woss;
		for (UINT i = 0; i < numExports; i++)
		{
			woss << L"|";
			if (depth > 0)
			{
				for (UINT j = 0; j < 2 * depth - 1; j++)
					woss << L" ";
			}
			woss << L" [" << i << L"]: ";
			if (exports[i].ExportToRename)
				woss << exports[i].ExportToRename << L" --> ";
			woss << exports[i].Name << L"\n";
		}
		return woss.str();
	};

	for (UINT i = 0; i < desc->NumSubobjects; i++)
	{
		wstr << L"| [" << i << L"]: ";
		switch (desc->pSubobjects[i].Type)
		{
		case D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG:
		{
			wstr << L"State Object Config\n";
			auto config = static_cast<const D3D12_STATE_OBJECT_CONFIG*>(desc->pSubobjects[i].pDesc);
			wstr << L"|  [0]: Flags: 0x" << std::hex << static_cast<UINT>(config->Flags) << std::dec << L"\n";
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE:
			wstr << L"Global Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
			break;
		case D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE:
			wstr << L"Local Root Signature 0x" << desc->pSubobjects[i].pDesc << L"\n";
			break;
		case D3D12_STATE_SUBOBJECT_TYPE_NODE_MASK:
			wstr << L"Node Mask: 0x" << std::hex << std::setfill(L'0') << std::setw(8)
				 << *static_cast<const UINT*>(desc->pSubobjects[i].pDesc) << std::setw(0) << std::dec << L"\n";
			break;
		case D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY:
		{
			wstr << L"DXIL Library 0x";
			auto lib = static_cast<const D3D12_DXIL_LIBRARY_DESC*>(desc->pSubobjects[i].pDesc);
			wstr << lib->DXILLibrary.pShaderBytecode << L", " << lib->DXILLibrary.BytecodeLength << L" bytes\n";
			wstr << ExportTree(1, lib->NumExports, lib->pExports);
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION:
		{
			wstr << L"Existing Library 0x";
			auto collection = static_cast<const D3D12_EXISTING_COLLECTION_DESC*>(desc->pSubobjects[i].pDesc);
			wstr << collection->pExistingCollection << L"\n";
			wstr << ExportTree(1, collection->NumExports, collection->pExports);
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
		{
			wstr << L"Subobject to Exports Association (Subobject [";
			auto association = static_cast<const D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
			UINT index = static_cast<UINT>(association->pSubobjectToAssociate - desc->pSubobjects);
			wstr << index << L"])\n";
			for (UINT j = 0; j < association->NumExports; j++)
			{
				wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
			}
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION:
		{
			wstr << L"DXIL Subobjects to Exports Association (";
			auto association =
				static_cast<const D3D12_DXIL_SUBOBJECT_TO_EXPORTS_ASSOCIATION*>(desc->pSubobjects[i].pDesc);
			wstr << association->SubobjectToAssociate << L")\n";
			for (UINT j = 0; j < association->NumExports; j++)
			{
				wstr << L"|  [" << j << L"]: " << association->pExports[j] << L"\n";
			}
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG:
		{
			wstr << L"Raytracing Shader Config\n";
			auto config = static_cast<const D3D12_RAYTRACING_SHADER_CONFIG*>(desc->pSubobjects[i].pDesc);
			wstr << L"|  [0]: Max Payload Size: " << config->MaxPayloadSizeInBytes << L" bytes\n";
			wstr << L"|  [1]: Max Attribute Size: " << config->MaxAttributeSizeInBytes << L" bytes\n";
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG:
		{
			wstr << L"Raytracing Pipeline Config\n";
			auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG*>(desc->pSubobjects[i].pDesc);
			wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1:
		{
			wstr << L"Raytracing Pipeline Config1\n";
			auto config = static_cast<const D3D12_RAYTRACING_PIPELINE_CONFIG1*>(desc->pSubobjects[i].pDesc);
			wstr << L"|  [0]: Max Recursion Depth: " << config->MaxTraceRecursionDepth << L"\n";
			wstr << L"|  [1]: Flags: 0x" << std::hex << static_cast<UINT>(config->Flags) << std::dec << L"\n";
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP:
		{
			wstr << L"Hit Group (";
			auto hitGroup = static_cast<const D3D12_HIT_GROUP_DESC*>(desc->pSubobjects[i].pDesc);
			wstr << (hitGroup->HitGroupExport ? hitGroup->HitGroupExport : L"[none]") << L")\n";
			wstr << L"|  [0]: Any Hit Import: "
				 << (hitGroup->AnyHitShaderImport ? hitGroup->AnyHitShaderImport : L"[none]") << L"\n";
			wstr << L"|  [1]: Closest Hit Import: "
				 << (hitGroup->ClosestHitShaderImport ? hitGroup->ClosestHitShaderImport : L"[none]") << L"\n";
			wstr << L"|  [2]: Intersection Import: "
				 << (hitGroup->IntersectionShaderImport ? hitGroup->IntersectionShaderImport : L"[none]") << L"\n";
			break;
		}
		case D3D12_STATE_SUBOBJECT_TYPE_MAX_VALID:
			wstr << L"State Subobject MaxValid (sentinel)\n";
			break;
		default:
			wstr << L"Unknown subobject type (" << static_cast<UINT>(desc->pSubobjects[i].Type) << L")\n";
			break;
		}
		wstr << L"|--------------------------------------------------------------------\n";
	}
	wstr << L"\n";
	OutputDebugStringW(wstr.str().c_str());
}

bool IsRayTracingSupported(IDXGIAdapter1* adapter, D3D_FEATURE_LEVEL featureLevel)
{
	ComPtr<ID3D12Device> testDevice;
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};

	return SUCCEEDED(D3D12CreateDevice(adapter, featureLevel, IID_PPV_ARGS(&testDevice)))
		&& SUCCEEDED(testDevice->CheckFeatureSupport(
			D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData)))
		&& featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

void SetOptimalMSAALevel(ID3D12Device10* pDevice, DXGI_FORMAT formmat)
{
	UINT maxSampleCountSupported = 0;
	UINT maxQualityLevels = 0;

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels = {};
	msQualityLevels.Format = formmat;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

	for (UINT sampleCount = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; sampleCount > 1; sampleCount--)
	{
		msQualityLevels.SampleCount = sampleCount;
		msQualityLevels.NumQualityLevels = 0;

		if (SUCCEEDED(pDevice->CheckFeatureSupport(
				D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)))
			&& msQualityLevels.NumQualityLevels > 0)
		{
			if (maxSampleCountSupported < sampleCount)
			{
				maxSampleCountSupported = msQualityLevels.SampleCount;
				maxQualityLevels = msQualityLevels.NumQualityLevels;
			}

			if (sampleCount <= engine::core::Settings::GetGraphicsSettings().GetDesiredMSAASampleCount())
			{
				engine::core::Settings::GetGraphicsSettings().SetMSAAQuality(msQualityLevels.NumQualityLevels - 1);
				engine::core::Settings::GetGraphicsSettings().SetMSAASampleCount(sampleCount);
				break;
			}
		}
	}

	if (maxSampleCountSupported > 1)
	{
		engine::core::Settings::GetGraphicsSettings().SetIsMSAASupported(true);
		engine::core::Settings::GetGraphicsSettings().SetMSAAMaximumSampleCount(maxSampleCountSupported);
	}
	else
	{
		engine::core::Settings::GetGraphicsSettings().SetIsMSAASupported(false);
	}
}

void SetTearing(Microsoft::WRL::ComPtr<IDXGIFactory4> pFactory)
{
	if (!engine::core::Settings::GetGraphicsSettings().IsTearingAllowed())
		return;

	HRESULT hr;

	BOOL allowTearing = FALSE;

	ComPtr<IDXGIFactory5> pFactory5;
	GFX_THROW_INFO(pFactory.As(&pFactory5));

	hr = pFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

	if (FAILED(hr) || !allowTearing)
	{
		engine::core::Settings::GetGraphicsSettings().SetTearingEnabled(false, 1);
		OutputDebugStringA("WARNING: Variable refresh rate displays not supported");
	}
}


void DescriptorHeap::Create(
	ID3D12Device10* pDevice, const std::wstring& Name, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount)
{
	HRESULT hr;

	m_HeapDesc.Type = Type;
	m_HeapDesc.NumDescriptors = MaxCount;
	m_HeapDesc.Flags = Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
																	  : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_HeapDesc.NodeMask = 1;

	GFX_THROW_INFO(pDevice->CreateDescriptorHeap(&m_HeapDesc, IID_PPV_ARGS(m_Heap.ReleaseAndGetAddressOf())));

	m_Heap->SetName(Name.c_str());

	m_DescriptorSize = pDevice->GetDescriptorHandleIncrementSize(m_HeapDesc.Type);
	m_NumFreeDescriptors = m_HeapDesc.NumDescriptors;
	m_FirstHandle = DescriptorHandle(
		m_Heap->GetCPUDescriptorHandleForHeapStart(),
		Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			? m_Heap->GetGPUDescriptorHandleForHeapStart()
			: D3D12_GPU_DESCRIPTOR_HANDLE(D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN));
	m_NextFreeHandle = m_FirstHandle;
}

DescriptorHandle DescriptorHeap::Alloc(uint32_t Count)
{
	assert(HasAvailableSpace(Count));

	DescriptorHandle ret = m_NextFreeHandle;
	m_NextFreeHandle += Count * m_DescriptorSize;
	m_NumFreeDescriptors -= Count;

	return ret;
}

bool DescriptorHeap::ValidateHandle(const DescriptorHandle& DHandle) const
{
	if (DHandle.GetCpuPtr() < m_FirstHandle.GetCpuPtr()
		|| DHandle.GetCpuPtr() >= m_FirstHandle.GetCpuPtr() + m_HeapDesc.NumDescriptors * m_DescriptorSize)
		return false;

	if (DHandle.GetGpuPtr() - m_FirstHandle.GetGpuPtr() != DHandle.GetCpuPtr() - m_FirstHandle.GetCpuPtr())
		return false;

	return true;
}


};  // namespace engine::gfx
