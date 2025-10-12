#pragma once

#include "WinInclude.hpp"
#include "CustomException.hpp"

#include <vector>

namespace engine::core
{

class Exception : public engine::core::CustomException
{
	using engine::core::CustomException::CustomException;
};

class HrException : public Exception
{
public:
	HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
	const char* what() const noexcept override;
	const char* GetType() const noexcept override;
	HRESULT GetErrorCode() const noexcept;
	std::string GetErrorDescription() const noexcept;
	std::string GetErrorInfo() const noexcept;

private:
	HRESULT hr;
	std::string info;
};

class InfoException : public Exception
{
public:
	InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
	const char* what() const noexcept override;
	const char* GetType() const noexcept override;
	std::string GetErrorInfo() const noexcept;

private:
	std::string info;
};

class DeviceRemovedException : public HrException
{
	using HrException::HrException;

public:
	const char* GetType() const noexcept override;

private:
	std::string reason;
};

}  // namespace engine::core
