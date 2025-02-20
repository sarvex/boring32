export module boring32.error:ntstatuserror;
import :boring32error;
import :functions;
import <string>;
import <stdexcept>;
import <source_location>;
import <format>;

export namespace Boring32::Error
{
	class NtStatusError : public Boring32Error
	{
		public:
			virtual ~NtStatusError() = default;
			NtStatusError(const NtStatusError& other) = default;
			NtStatusError(NtStatusError&& other) noexcept = default;

			// Related: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
			NtStatusError(
				const std::string& msg,
				const std::source_location location = std::source_location::current()
			) : m_errorCode(0), Boring32Error(msg, location)
			{
				GenerateErrorMessage(location, msg);
			}

			NtStatusError(
				const std::string& msg,
				const long errorCode,
				const std::source_location location = std::source_location::current()
			) : m_errorCode(errorCode), Boring32Error(msg, location)
			{
				GenerateErrorMessage(location, msg);
			}

		public:
			virtual NtStatusError& operator=(const NtStatusError& other) = default;
			virtual NtStatusError& operator=(NtStatusError&& other) noexcept = default;

		public:
			[[nodiscard]] virtual long GetErrorCode() const noexcept
			{
				return m_errorCode;
			}

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override
			{
				if (m_errorCode)
				{
					m_message = Boring32::Error::GetNtStatusCode<std::string>(m_errorCode);
					m_message = Error::FormatErrorMessage("NTSTATUS", location, message, m_errorCode, m_message);
				}
				else
				{
					m_message = Error::FormatErrorMessage("NTSTATUS", location, message);
				}
			}

		protected:
			long m_errorCode = 0;
	};
}