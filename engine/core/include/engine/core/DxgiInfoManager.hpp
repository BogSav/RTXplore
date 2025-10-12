#pragma once

#include "CustomException.hpp"

#include <dxgidebug.h>
#include <wrl/client.h>
#include <vector>

// Singleton global, se creaza static direct in .cpp
#ifdef _DEBUG
namespace engine::core
{

class DxgiInfoManager
{
private:
	using Ptr = std::unique_ptr<DxgiInfoManager>;

public:
	static DxgiInfoManager& GetInstance();
	static void DestroyInstance();

	void Set() noexcept;
	std::vector<std::string> GetMessages() const;

	~DxgiInfoManager() = default;

private:
	DxgiInfoManager();
	DxgiInfoManager(const DxgiInfoManager&) = delete;
	DxgiInfoManager& operator=(const DxgiInfoManager&) = delete;

private:
	unsigned long long next = 0u;
	Microsoft::WRL::ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;

	static Ptr instance;
};
}  // namespace engine::core

#endif

#include "Exceptions.hpp"
#include "GraphicsThrowMacros.hpp"