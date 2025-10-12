#pragma once

#include <optional>
#include <string>
#include <memory>

namespace misc
{

class CustomException : public std::exception
{
public:
	CustomException(std::string what) noexcept;
	CustomException(int line, const char* file) noexcept;

	const char* what() const noexcept override;
	
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;

protected:
	mutable std::string whatBuffer;

private:
	enum class Type
	{
		LINE_FILE_ERROR,
		TEXT_ERROR
	};

	std::optional<int> line;
	std::optional<std::string> file;
	std::optional<std::string> originalErrorMessage;
	Type type;
};

}  // namespace misc