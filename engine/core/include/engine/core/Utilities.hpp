#pragma once

#include <string>
#include <vector>

namespace misc
{

void ConvertStringToWString(const std::string& s, std::wstring& ws);
std::wstring GetWStringFromString(const std::string& s);
void ConvertWStringToSting(const std::wstring& ws, std::string& s);
std::string GetStringFromWString(const std::wstring& ws);

template <class BlotType>
static std::string convertBlobToString(BlotType* pBlob)
{
	std::vector<char> infoLog(pBlob->GetBufferSize() + 1);
	memcpy(infoLog.data(), pBlob->GetBufferPointer(), pBlob->GetBufferSize());
	infoLog[pBlob->GetBufferSize()] = 0;
	return std::string(infoLog.data());
}

};  // namespace misc
