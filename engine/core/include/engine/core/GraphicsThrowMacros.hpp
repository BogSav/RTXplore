#pragma once

// HRESULT hr should exist in the local scope for these macros to work

#define GFX_EXCEPT_NOINFO(hr) engine::core::HrException(__LINE__, __FILE__, (hr))

#define GFX_THROW_NOINFO(hrcall) \
	if (FAILED(hr = (hrcall)))   \
	throw engine::core::HrException(__LINE__, __FILE__, hr)

#ifdef _DEBUG
#define GFX_EXCEPT(hr) \
	engine::core::HrException(__LINE__, __FILE__, (hr), engine::core::DxgiInfoManager::GetInstance().GetMessages())

#define GFX_THROW_INFO(hrcall)					\
	engine::core::DxgiInfoManager::GetInstance().Set();		\
	if (FAILED(hr = (hrcall)))					\
	throw GFX_EXCEPT(hr)

#define GFX_DEVICE_REMOVED_EXCEPT(hr) \
	engine::core::DeviceRemovedException( \
		__LINE__, __FILE__, (hr), engine::core::DxgiInfoManager::GetInstance().GetMessages())

#define GFX_THROW_INFO_ONLY(call)									\
	engine::core::DxgiInfoManager::GetInstance().Set();							\
	(call);															\
	{																\
		auto v = engine::core::DxgiInfoManager::GetInstance().GetMessages();		\
		if (!v.empty())												\
		{															\
			throw engine::core::InfoException(__LINE__, __FILE__, v);		\
		}															\
	}
#else
#define GFX_EXCEPT(hr) engine::core::HrException(__LINE__, __FILE__, (hr))
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) engine::core::DeviceRemovedException(__LINE__, __FILE__, (hr))
#define GFX_THROW_INFO_ONLY(call) (call)
#endif