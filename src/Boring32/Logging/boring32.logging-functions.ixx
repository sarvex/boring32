module;

#include <source_location>

export module boring32.logging:functions;
import <string>;

export namespace Boring32::Logging
{
	// Based on https://www.reddit.com/r/cpp_questions/comments/vl2n68/variadic_template_and_default_arguments/
	struct MessageAndLocation
	{
		std::string_view message;
		std::source_location loc;

		template<typename T>
		MessageAndLocation(
			T&& msg,
			std::source_location loc = std::source_location::current()
		) : message{ std::forward<T>(msg) },
			loc{ loc }
		{}
	};

	template <class... Types>
	void Info(const std::string_view fmt, const Types&... Args)
	{
	}

	template <class... Types>
	void Warn(const MessageAndLocation msgAndLoc, const Types&... Args)
	{
	}

	template <class... Types>
	void Warn(
		const MessageAndLocation msgAndLoc, 
		const std::exception& ex, 
		const Types&... Args
	)
	{
	}

	template <class... Types>
	void Exception(
		const MessageAndLocation msgAndLoc,
		const std::exception& ex,
		const Types&... Args
	)
	{
	}

	template <class... Types>
	void Error(
		const MessageAndLocation msgAndLoc,
		const std::exception& ex,
		const Types&... Args
	)
	{
	}

	template <class... Types>
	void Error(
		const MessageAndLocation msgAndLoc,
		const Types&... Args
	)
	{
	}

	template <class... Types>
	void Debug(const std::string_view fmt, const Types&... Args)
	{
	}
}