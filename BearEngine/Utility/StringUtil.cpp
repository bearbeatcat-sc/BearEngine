#include "StringUtil.h"
#include <tchar.h>
#include <Windows.h>

std::wstring StringUtil::GetWideStringFromString(const std::string& str)
{
    auto num1 = MultiByteToWideChar(CP_ACP,
        MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
        str.c_str(),
        -1,
        nullptr,
        0);

    std::wstring wstr;
    wstr.resize(num1);

    auto num2 = MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
        str.c_str(),
        -1,
        &wstr[0],
        num1);


    return wstr;
}

