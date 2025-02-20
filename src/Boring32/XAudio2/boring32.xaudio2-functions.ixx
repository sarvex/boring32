export module boring32.xaudio2:functions;
import <win32.hpp>;
import boring32.error;
import <stdexcept>;

namespace Boring32::XAudio2
{
	constexpr DWORD fourccRIFF = 'FFIR';
	constexpr DWORD fourccDATA = 'atad';
	constexpr DWORD fourccFMT = 'tmf'; // originally ' tmf';
	constexpr DWORD fourccWAVE = 'EVAW';
	constexpr DWORD fourccXWMA = 'AMWX';
	constexpr DWORD fourccDPDS = 'sdpd';
}

export namespace Boring32::XAudio2
{
	void FindChunk(
		HANDLE hFile, 
		DWORD fourcc, 
		DWORD& dwChunkSize,
		DWORD& dwChunkDataPosition
	)
    {
        DWORD result = SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
        if (result == INVALID_SET_FILE_POINTER)
            throw Error::Win32Error(
                "SetFilePointer() failed",
                GetLastError()
            );

        DWORD dwChunkType;
        DWORD dwChunkDataSize;
        DWORD dwRIFFDataSize = 0;
        DWORD dwFileType;
        DWORD bytesRead = 0;
        DWORD dwOffset = 0;

        HRESULT hr = S_OK;
        while (hr == S_OK)
        {
            DWORD dwRead;
            if (!ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, nullptr))
                throw Error::Win32Error(
                    "ReadFile() failed",
                    GetLastError()
                );

            if (!ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, nullptr))
                throw Error::Win32Error(
                    "ReadFile() failed",
                    GetLastError()
                );

            switch (dwChunkType)
            {
            case fourccRIFF:
                dwRIFFDataSize = dwChunkDataSize;
                dwChunkDataSize = 4;
                if (!ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, nullptr))
                    throw Error::Win32Error(
                        "ReadFile() failed",
                        GetLastError()
                    );
                break;

            default:
                result = SetFilePointer(
                    hFile,
                    dwChunkDataSize,
                    nullptr,
                    FILE_CURRENT
                );
                if (result == INVALID_SET_FILE_POINTER)
                    throw Error::Win32Error(
                        "SetFilePointer() failed",
                        GetLastError()
                    );
            }

            dwOffset += sizeof(DWORD) * 2;

            if (dwChunkType == fourcc)
            {
                dwChunkSize = dwChunkDataSize;
                dwChunkDataPosition = dwOffset;
                break;
            }
            dwOffset += dwChunkDataSize;
            if (bytesRead >= dwRIFFDataSize)
                throw std::runtime_error("bytesRead >= dwRIFFDataSize");
        }
    }
}
