export module boring32.time:datetime;
import <cstdint>;
import <win32.hpp>;
import :functions;
import boring32.error;

export namespace Boring32::Time
{
	class DateTime
	{
		public:
			virtual ~DateTime() = default;
			
			DateTime()
			{
				GetSystemTimeAsFileTime(&m_ft);
			}

			DateTime(const DateTime& dateTime) = default;
			DateTime(DateTime&& dateTime) noexcept = default;
			DateTime(const SYSTEMTIME& st)
			{
				if (!SystemTimeToFileTime(&st, &m_ft))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("SystemTimeToFileTime() failed", lastError);
				}
			}

			DateTime(const FILETIME& ft)
				: m_ft(ft)
			{ }

		public:
			virtual uint64_t ToMicroSeconds() const noexcept
			{
				return FromFileTime(m_ft) / 10ull;
			}

			virtual uint64_t ToNanosecondTicks() const noexcept
			{
				return FromFileTime(m_ft);
			}

			virtual void AddMinutes(const int64_t minutes)
			{
				const int64_t nsTicks = minutes * 60ll * 1000ll * 1000ll * 10ll;
				SetNewTotal(ToNanosecondTicks() + nsTicks);
			}

			virtual void AddSeconds(const int64_t seconds)
			{
				const int64_t nsTicks = seconds * 1000ll * 1000ll * 10ll;
				SetNewTotal(ToNanosecondTicks() + nsTicks);
			}

			virtual void AddMillseconds(const int64_t milliseconds)
			{
				const int64_t nsTicks = milliseconds * 1000ll * 10ll;
				SetNewTotal(ToNanosecondTicks() + nsTicks);
			}

			virtual SYSTEMTIME ToSystemTime() const
			{
				SYSTEMTIME st;
				// https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-filetimetosystemtime
				if (!FileTimeToSystemTime(&m_ft, &st))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("FileTimeToSystemTime() failed", lastError);
				}
				return st;
			}

		protected:
			virtual void SetNewTotal(const uint64_t newTotal)
			{
				const LARGE_INTEGER li{
					.QuadPart = static_cast<long long>(newTotal)
				};
				m_ft = {
					.dwLowDateTime = li.LowPart,
					.dwHighDateTime = static_cast<DWORD>(li.HighPart)
				};
			}

		protected:
			FILETIME m_ft{ 0 };
	};
}
