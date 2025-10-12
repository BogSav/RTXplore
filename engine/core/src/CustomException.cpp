#include "CustomException.hpp"

#include <sstream>
#include <assert.h>

namespace misc
{
CustomException::CustomException(std::string what) noexcept : originalErrorMessage(what), type(Type::TEXT_ERROR)
{
}
CustomException::CustomException(int line, const char* file) noexcept : line(line), file(file), type(Type::LINE_FILE_ERROR)
{
}

const char* CustomException::what() const noexcept
{
	std::ostringstream oss;
	switch (type)
	{
	case misc::CustomException::Type::LINE_FILE_ERROR:
	{
		assert(line && file);

		oss << GetType() << std::endl << GetOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}
	case misc::CustomException::Type::TEXT_ERROR:
	{
		assert(originalErrorMessage);

		oss << GetType() << std::endl << *originalErrorMessage;
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}
	}
	return nullptr;
}

const char* CustomException::GetType() const noexcept
{
	switch (type)
	{
	case misc::CustomException::Type::LINE_FILE_ERROR: return "Line and file error";
	case misc::CustomException::Type::TEXT_ERROR: return "Simple text error";
	default: break;
	}
	return "Chili Exception";
}

int CustomException::GetLine() const noexcept
{
	assert(line);
	return *line;
}

const std::string& CustomException::GetFile() const noexcept
{
	assert(file);
	return *file;
}

std::string CustomException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << *file << std::endl << "[Line] " << *line;
	return oss.str();
}

}  // namespace misc