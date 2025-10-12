#pragma once

#include "PipelineState.hpp"
#include "HlslUtils.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dxgi1_6.h>

#include "Mesh.hpp"

#include <array>

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

namespace engine::gfx
{

////////////////////////////////////////////////////////////////////////////////
// Chestii ajutataoare rayTracing (rt)
////////////////////////////////////////////////////////////////////////////////
namespace rt
{
static const wchar_t* c_raygenShaderName = L"rayGen";

static const wchar_t* HitGroupName_TriangleGeometry = L"TriangleGeometry_HitGroup";
static const wchar_t* HitGroupName_AABBGeometry = L"AABBGeometry_HitGroup";

static const wchar_t* ClosestHit_TriangleGeometry = L"CHS_TriangleGeometry";
static const wchar_t* ClosestHit_AABBGeometry = L"CHS_AABBGeometry";

static const wchar_t* IntersectionShader = L"IntersectionShader";

static const wchar_t* Miss_RadianceShader = L"DefaultMiss";
static const wchar_t* Miss_ShadowShader = L"ShadowMiss";
}  // namespace rt


////////////////////////////////////////////////////////////////////////////////
// Chestii ajutatoare rasterizare
////////////////////////////////////////////////////////////////////////////////
namespace rasterization
{
namespace RenderLayer
{
enum Value : int
{
	Base = 0,
	CubeMap = 1,
	ShadowMap = 2,
	DebugShadowMap = 3,
	OcclusionQuery = 4
};
}
}  // namespace rasterization

namespace RSBinding
{
namespace DefaultRSBindings
{
enum Value : int
{
	PassCB,
	ObjectCB,
	MaterialCB,
	StaticPixelTextures,
	StaticNonPixelTextures,
	DynamicTextures,
	Count
};
}

namespace WaterRSParams
{
enum Value : int
{
	PassCB = 0,
	ObjectCB,
	MaterialCB,
	WaterCB,
	StaticPixelTextures,
	StaticNonPixelTextures,
	DynamicTextures,
	Count
};
}

namespace GlobalRTRSBinding
{
enum Value : int
{
	PassCB = 0,
	OutputSlot,
	AccelerationStrcuture,
	Count
};
}
}  // namespace RSBinding


////////////////////////////////////////////////////////////////////////////////
// Wrapper classes
////////////////////////////////////////////////////////////////////////////////
class DescriptorHandle
{
public:
	DescriptorHandle()
	{
		m_CpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_GpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle)
		: m_CpuHandle(CpuHandle), m_GpuHandle(GpuHandle)
	{
	}

	DescriptorHandle operator+(INT OffsetScaledByDescriptorSize) const
	{
		DescriptorHandle ret = *this;
		ret += OffsetScaledByDescriptorSize;
		return ret;
	}

	void operator+=(INT OffsetScaledByDescriptorSize)
	{
		if (m_CpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_CpuHandle.ptr += OffsetScaledByDescriptorSize;
		if (m_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_GpuHandle.ptr += OffsetScaledByDescriptorSize;
	}

	bool operator==(const DescriptorHandle& rhs) const
	{
		return (this->GetCpuPtr() == rhs.GetCpuPtr()) && (this->GetGpuPtr() == rhs.GetGpuPtr());
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &m_CpuHandle; }
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return m_CpuHandle; }
	operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return m_GpuHandle; }

	size_t GetCpuPtr() const { return m_CpuHandle.ptr; }
	uint64_t GetGpuPtr() const { return m_GpuHandle.ptr; }
	bool IsNull() const { return m_CpuHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
	bool IsShaderVisible() const { return m_GpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;
};


class DescriptorHeap
{
public:
	DescriptorHeap(void) {}
	~DescriptorHeap(void) { Destroy(); }

	void Create(
		ID3D12Device10* pDevice, const std::wstring& DebugHeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxCount);
	void Destroy(void) { m_Heap = nullptr; }

	bool HasAvailableSpace(uint32_t Count) const { return Count <= m_NumFreeDescriptors; }
	DescriptorHandle Alloc(uint32_t Count = 1);

	DescriptorHandle operator[](uint32_t arrayIdx) const { return m_FirstHandle + arrayIdx * m_DescriptorSize; }

	uint32_t GetOffsetOfHandle(const DescriptorHandle& DHandle)
	{
		return (uint32_t)(DHandle.GetCpuPtr() - m_FirstHandle.GetCpuPtr()) / m_DescriptorSize;
	}

	bool ValidateHandle(const DescriptorHandle& DHandle) const;

	ID3D12DescriptorHeap* GetHeapPointer() const { return m_Heap.Get(); }

	uint32_t GetDescriptorSize(void) const { return m_DescriptorSize; }

	const DescriptorHandle& GetFirstDescriptorHandle() const { return m_FirstHandle; }

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
	D3D12_DESCRIPTOR_HEAP_DESC m_HeapDesc = {};
	uint32_t m_DescriptorSize = 0;
	uint32_t m_NumFreeDescriptors = 0;
	DescriptorHandle m_FirstHandle;
	DescriptorHandle m_NextFreeHandle;
};


////////////////////////////////////////////////////////////////////////////////
// Descriptori pentru rendereri (render_descriptors)
////////////////////////////////////////////////////////////////////////////////
namespace render_descriptors
{
struct DX_OBJECT_DESCRIPTOR
{
	UINT objectCB_ID;
	UINT materialCB_ID;

	std::string subGeometryName;

	bool isStatic = true;

	engine::math::Quaternion rotation = engine::math::Quaternion(engine::math::kIdentity);
	engine::math::Vector3 position = engine::math::Vector3(engine::math::kOrigin);
	engine::math::Vector3 scale = engine::math::Vector3(engine::math::kIdentity);
	engine::math::Vector4 color = engine::math::Vector4(1, 0, 0, 1);

	engine::math::Matrix4 textureTransform = engine::math::Matrix4(engine::math::kIdentity);
};
struct DX_TERRAIN_DESCRIPTOR
{
	DX_OBJECT_DESCRIPTOR objectDescriptor;

	GraphicsPSO::Ptr basePSO;
	D3D12_PRIMITIVE_TOPOLOGY baseToplogy;

	GraphicsPSO::Ptr cubePSO;
	GraphicsPSO::Ptr shadowPSO;
	D3D12_PRIMITIVE_TOPOLOGY cubeShadowToplogy;

	float width;
	float length;
	int chunkKernelSize;
	int chunkCountPerSide;

	// Proprietati fractal noise
	struct DX_SIMPLEX_PROPERTIES
	{
		float frequency = 1.0f;
		float amplitude = 1.0f;
		float lacunarity = 2.0f;
		float persistence = 0.5f;
		float amplitudeFactor = 30.f;

		size_t octaveCount = 1;
	} simplexProperties;
};
struct DX_WATER_DESCRIPTOR
{
	DX_OBJECT_DESCRIPTOR objectDescriptor;

	GraphicsPSO::Ptr basePSO;
	D3D12_PRIMITIVE_TOPOLOGY baseToplogy;

	engine::math::Vector3 textureDirection = {};
	engine::math::Vector3 textureScale = {};
	float textureMoveSpeed = 0.f;
	float cubeMapSphereRadius = 0.f;

	float width = 0.f;
	float length = 0.f;
	int chunkKernelSize = 0;
	int chunkCountPerSide = 0;

	std::array<WaveProperties, WAVE_PROPERTIES_COUNT> waveProperties;
};
struct DX_SKYBOX_DESCRIPTOR
{
	DX_OBJECT_DESCRIPTOR objectDescriptor;

	GraphicsPSO::Ptr basePSO;
	D3D12_PRIMITIVE_TOPOLOGY baseToplogy;

	GraphicsPSO::Ptr dynamicCubeMapPSO;
	D3D12_PRIMITIVE_TOPOLOGY dynamicCubeMapToplogy;

	engine::math::Vector3 rotationAxis;
	engine::math::Vector3 lightDirection;
	float rotationSpeed;
};
struct DX_TEXTURE_DESCRIPTOR
{
	DescriptorHandle textureSRVHandle;

	GraphicsPSO::Ptr basePSO;
	D3D12_PRIMITIVE_TOPOLOGY baseTopology;
};
struct DX_OBJECTS_RENDERER_DESCRIPTOR
{
	std::vector<DX_OBJECT_DESCRIPTOR> objectDescriptors;

	GraphicsPSO::Ptr basePSO;
	D3D12_PRIMITIVE_TOPOLOGY baseToplogy;

	GraphicsPSO::Ptr dynamicCubeMapPSO;
	D3D12_PRIMITIVE_TOPOLOGY dynamicCubeMapToplogy;

	GraphicsPSO::Ptr shadowPSO;
	D3D12_PRIMITIVE_TOPOLOGY shadowTopology;

	GraphicsPSO::Ptr debugShadowPSO;
};
}  // namespace render_descriptors



const D3D_FEATURE_LEVEL featureLevels[] = {
	D3D_FEATURE_LEVEL_12_2,
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0};



////////////////////////////////////////////////////////////////////////////////
// Functii auxiliare
////////////////////////////////////////////////////////////////////////////////
bool IsRayTracingSupported(IDXGIAdapter1* adapter, D3D_FEATURE_LEVEL featureLevel);
void SetOptimalMSAALevel(ID3D12Device10* pDevice, DXGI_FORMAT format);
void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
void SetTearing(Microsoft::WRL::ComPtr<IDXGIFactory4> pFactory);
void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc);
void LogAdapterOutputs(IDXGIAdapter* adapter);
void LogAdapters();

void GetHardwareAdapter(
	IDXGIFactory4* pFactory,
	IDXGIAdapter1** ppAdapter,
	D3D_FEATURE_LEVEL& outMaxFeatureLevel,
	D3D_FEATURE_LEVEL minimalFeatureLevel);

inline UINT Align(UINT size, UINT alignment)
{
	return (size + (alignment - 1)) & ~(alignment - 1);
}

inline UINT CalculateConstantBufferByteSize(UINT byteSize)
{
	return Align(byteSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
}

};  // namespace misc
