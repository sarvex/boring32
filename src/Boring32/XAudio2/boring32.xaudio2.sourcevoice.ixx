module;

#include <Windows.h>
#include <xaudio2.h>

export module boring32.xaudio2.sourcevoice;
import boring32.xaudio2.voice;

export namespace Boring32::XAudio2
{
	// https://docs.microsoft.com/en-us/windows/win32/api/xaudio2/nn-xaudio2-ixaudio2sourcevoice
	class SourceVoice : public Voice
	{
		public:
			virtual ~SourceVoice();
			SourceVoice(const SourceVoice&) = delete;
			SourceVoice(SourceVoice&&) noexcept;
			SourceVoice(IXAudio2SourceVoice* voice);

		public:
			virtual SourceVoice& operator=(const SourceVoice&) = delete;
			virtual SourceVoice& operator=(SourceVoice&&) noexcept;

		public:
			virtual void Close();

		protected:
			IXAudio2SourceVoice* m_voice;
	};
}