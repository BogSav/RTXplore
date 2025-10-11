#pragma once

#include "WinInclude.hpp"
#include "customException.hpp"

#include <vector>

namespace misc
{

class Exception : public misc::customException
{
	using misc::customException::customException;
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

}  // namespace misc
