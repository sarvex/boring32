module;

#include <sstream>
#include <format>
#include "Windows.h"

module boring32.time;

namespace Boring32::Time
{
    DWORD SystemTimeToShortIsoDate(const SYSTEMTIME& st)
    {
        return std::stoul(std::format(L"{}{}{}", st.wYear, st.wMonth, st.wDay));
    }

    DWORD SystemTimeToShortIsoDate()
    {
        SYSTEMTIME st;
        GetSystemTime(&st);
        return SystemTimeToShortIsoDate(st);
    }

    uint64_t FromFileTime(const FILETIME& ft)
    {
        const ULARGE_INTEGER uli = { 
            .LowPart = ft.dwLowDateTime,
            .HighPart = ft.dwHighDateTime
        };
        return uli.QuadPart;
    }

	SYSTEMTIME LargeIntegerTimeToSystemTime(const LARGE_INTEGER& li)
	{
        FILETIME ft;
        ft.dwLowDateTime = li.LowPart;
        ft.dwHighDateTime = li.HighPart;
        SYSTEMTIME st;
        // https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-filetimetosystemtime
        FileTimeToSystemTime(&ft, &st);
        return st;
	}

    std::wstring GetTimeAsUtcString(const SYSTEMTIME& st)
    {
        // Format date buffer
        constexpr UINT dateStringLength = 9;
        wchar_t dateString[dateStringLength];
        // https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-getdateformatex
        GetDateFormatEx(
            LOCALE_NAME_INVARIANT,
            0,
            &st,
            L"yyyyMMdd",
            dateString,
            dateStringLength,
            nullptr
        );

        // Format time buffer
        constexpr UINT timeStringLength = 9;
        wchar_t timeString[timeStringLength];
        GetTimeFormatEx(
            LOCALE_NAME_INVARIANT,
            0,
            &st,
            L"HH:mm:ss",
            timeString,
            timeStringLength
        );

        TIME_ZONE_INFORMATION tzi;
        // https://docs.microsoft.com/en-us/windows/win32/api/datetimeapi/nf-datetimeapi-gettimeformatex
        GetTimeZoneInformation(&tzi);
        std::wstringstream wss;
        wss << dateString
            << L"-"
            << timeString
            << L"."
            << st.wMilliseconds;
        DWORD actualBias = tzi.Bias * -1;
        if (actualBias >= 0)
            wss << L"+";
        wss << actualBias;
        return wss.str();
    }
}
