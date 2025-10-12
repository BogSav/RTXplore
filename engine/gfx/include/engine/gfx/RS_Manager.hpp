#pragma once

#include "RS.hpp"

#include <unordered_map>
#include <functional>

class RS_Manager
{
public:
	RS_Manager();

	bool AddRootSiganture(std::string name, RootSignature::Ptr rootSignture);
	RootSignature::Ptr GetRootSignature(std::string name) const;
	ID3D12RootSignature* GetID3D12RootSignature(std::string name) const;

	void LoadRootSignatures(ID3D12Device10* pDevice);

private:
	// Rasterization root signatures
	static RootSignature::Ptr CreateDefaultRS(ID3D12Device10* pDevice);
	static RootSignature::Ptr CreateWaterRS(ID3D12Device10* pDevice);
	static RootSignature::Ptr CreateTextureRS(ID3D12Device10* pDevice);

	// RayTracing root sigantures
	static RootSignature::Ptr CreateGlobalRTRS(ID3D12Device10* pDevice);
	static RootSignature::Ptr CreateRayGenRS(ID3D12Device10* pDevice);
	static RootSignature::Ptr CreateMissRS(ID3D12Device10* pDevice);
	
	static RootSignature::Ptr CreateTriangleHitRS(ID3D12Device10* pDevice);
	static RootSignature::Ptr CreateAABBHitRS(ID3D12Device10* pDevice);

	// Compute root signatures
	static RootSignature::Ptr CreateWavesRS(ID3D12Device10* pDevice);

private:
	std::unordered_map<std::string, std::function<RootSignature::Ptr(ID3D12Device10* pDevice)>> m_loadFunctions;
	std::unordered_map<std::string, RootSignature::Ptr> m_rootSignatures;

	bool firstLoad = true;
};
