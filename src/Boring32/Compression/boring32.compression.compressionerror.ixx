module;

#include <source_location>
#include <stdexcept>
#include <string>

export module boring32.compression.compressionerror;

import boring32.error.boring32error;

export namespace Boring32::Compression
{
	class CompressionError : public Error::Boring32Error
	{
		public:
			virtual ~CompressionError();
			CompressionError(const CompressionError& other);
			CompressionError(CompressionError&& other) noexcept;
			CompressionError(
				const std::source_location& location,
				const std::string& message
			);

		public:
			virtual CompressionError& operator=(const CompressionError& other);
			virtual CompressionError& operator=(CompressionError&& other) noexcept;

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override;
	};
}