export module boring32.error:boring32error;
import <stdexcept>;
import <string>;
import <format>;
import <source_location>;
import :functions;

export namespace Boring32::Error
{
	class Boring32Error : public std::exception
	{
		public:
			virtual ~Boring32Error() = default;
			Boring32Error(const Boring32Error& other) = default;
			Boring32Error(Boring32Error&& other) noexcept = default;

			Boring32Error()
				: m_message("Boring32 encountered an error")
			{ }

			Boring32Error(
				const std::string& message, 
				const std::source_location location = std::source_location::current()
			)
			{
				GenerateErrorMessage(location, message);
			}

			template<typename...Args>
			Boring32Error(
				const std::string& message,
				const std::source_location location,
				Args...args
			)
			{
				GenerateErrorMessage(
					location,
					std::vformat(
						message, 
						std::make_format_args(args...)
					)
				);
			}

		public:
			virtual Boring32Error& operator=(const Boring32Error& other) = default;
			virtual Boring32Error& operator=(Boring32Error&& other) noexcept = default;

		public:
			virtual const char* what() const noexcept override
			{
				return m_message.c_str();
			}

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			)
			{
				m_message = Error::FormatErrorMessage("Boring32", location, message);
			}

		protected:
			std::string m_message;
	};
}