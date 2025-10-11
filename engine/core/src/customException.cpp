#include "customException.hpp"

#include <sstream>
#include <assert.h>

namespace misc
{
customException::customException(std::string what) noexcept : originalErrorMessage(what), type(Type::TEXT_ERROR)
{
}
customException::customException(int line, const char* file) noexcept : line(line), file(file), type(Type::LINE_FILE_ERROR)
{
}

const char* customException::what() const noexcept
{
	std::ostringstream oss;
	switch (type)
	{
	case misc::customException::Type::LINE_FILE_ERROR:
	{
		assert(line && file);

		oss << GetType() << std::endl << GetOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}
	case misc::customException::Type::TEXT_ERROR:
	{
		assert(originalErrorMessage);

		oss << GetType() << std::endl << *originalErrorMessage;
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}
	}
	return nullptr;
}

const char* customException::GetType() const noexcept
{
	switch (type)
	{
	case misc::customException::Type::LINE_FILE_ERROR: return "Line and file error";
	case misc::customException::Type::TEXT_ERROR: return "Simple text error";
	default: break;
	}
	return "Chili Exception";
}

int customException::GetLine() const noexcept
{
	assert(line);
	return *line;
}

const std::string& customException::GetFile() const noexcept
{
	assert(file);
	return *file;
}

std::string customException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << *file << std::endl << "[Line] " << *line;
	return oss.str();
}

}  // namespace misc