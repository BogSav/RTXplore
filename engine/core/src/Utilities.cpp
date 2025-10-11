#include "Utilities.hpp"

namespace misc
{

void ConvertStringToWString(const std::string& s, std::wstring& ws)
{
	std::size_t size = s.size() + 1;
	ws = std::wstring(size, L'\0');

	std::size_t outSize;
	mbstowcs_s(&outSize, &ws[0], size, s.c_str(), size - 1);
}

std::wstring GetWStringFromString(const std::string& s)
{
	std::wstring ws;
	ConvertStringToWString(s, ws);
	return ws;
}

void ConvertWStringToSting(const std::wstring& ws, std::string& s)
{
	std::size_t size = ws.size() + 1;
	s = std::string(size, '\0');

	std::size_t outSize;
	wcstombs_s(&outSize, &s[0], size, ws.c_str(), size - 1);
}

std::string GetStringFromWString(const std::wstring& ws)
{
	std::string s;
	ConvertWStringToSting(ws, s);
	return s;
}

}  // namespace misc