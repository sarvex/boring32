#include "pch.hpp"
#include <stdexcept>
#include <vector>
#include <sstream>
#include "include/WinHttp/WebSockets/WebSocket.hpp"
#include "include/WinHttp/ProxyInfo.hpp"
#include "include/Error/Error.hpp"

namespace Boring32::WinHttp::WebSockets
{
	// Adapted from https://stackoverflow.com/a/29752943/7448661
	std::wstring Replace(std::wstring source, const std::wstring& from, const std::wstring& to)
	{
		std::wstring newString;
		newString.reserve(source.length());

		std::wstring::size_type lastPos = 0;
		std::wstring::size_type findPos;

		while (std::wstring::npos != (findPos = source.find(from, lastPos)))
		{
			newString.append(source, lastPos, findPos - lastPos);
			newString += to;
			lastPos = findPos + from.length();
		}

		newString += source.substr(lastPos);

		return newString;
	}

	WebSocket::~WebSocket()
	{
		Close();
	}

	WebSocket::WebSocket()
	:	m_settings({}),
		m_winHttpSession(nullptr),
		m_winHttpConnection(nullptr),
		m_status(WebSocketStatus::NotInitialised)
	{ }

	WebSocket::WebSocket(WebSocketSettings settings)
	:	m_settings(std::move(settings)),
		m_winHttpSession(nullptr),
		m_winHttpConnection(nullptr),
		m_status(WebSocketStatus::NotInitialised)
	{ }

	WebSocket::WebSocket(WebSocket&& other) noexcept
	:	m_settings({}),
		m_winHttpSession(nullptr),
		m_winHttpConnection(nullptr),
		m_status(WebSocketStatus::NotInitialised)
	{
		Move(other);
	}

	WebSocket& WebSocket::operator=(WebSocket&& other) noexcept
	{
		Move(other);
		return *this;
	}
	
	void WebSocket::Move(WebSocket& other) noexcept
	{
		m_settings = std::move(other.m_settings);
		m_status = other.m_status;
		m_winHttpConnection = std::move(other.m_winHttpConnection);
		m_winHttpSession = std::move(other.m_winHttpSession);
		m_winHttpWebSocket = std::move(other.m_winHttpWebSocket);
	}

	const WebSocketSettings& WebSocket::GetSettings()
	{
		return m_settings;
	}

	void WebSocket::Connect(const std::wstring& path)
	{
		InternalConnect(path);
	}

	void WebSocket::Connect()
	{
		InternalConnect(L"");
	}

	void WebSocket::InternalConnect(const std::wstring& path)
	{
		if (m_status != WebSocketStatus::NotInitialised)
			throw std::runtime_error("WebSocket needs to be in NotInitialised state to connect");

		try
		{
			DWORD accessType = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
			if (m_settings.PacUrl != L"")
				accessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
			else if (m_settings.Proxies != L"")
				accessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopen
			m_winHttpSession = WinHttpOpen(
				m_settings.UserAgent.empty()
				? L"websocket-user-agent"
				: m_settings.UserAgent.c_str(),
				accessType,
				accessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY
				? m_settings.Proxies.c_str()
				: WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS,
				0
			);
			if (m_winHttpSession == nullptr)
				throw Error::Win32Error("WebSocket::InternalConnect(): WinHttpOpen() failed", GetLastError());

			if (m_settings.PacUrl != L"")
			{
				// Note that WinHttp does not support loading PAC files locally:
				// https://docs.microsoft.com/en-us/troubleshoot/browsers/cannot-read-pac-file
				ProxyInfo proxyInfo;
				proxyInfo.SetAutoProxy(m_winHttpSession.Get(), m_settings.PacUrl, m_settings.Server + path);
				proxyInfo.SetOnSession(m_winHttpSession.Get());
			}

			m_winHttpConnection = WinHttpConnect(
				m_winHttpSession.Get(),
				m_settings.Server.c_str(),
				m_settings.Port,
				0
			);
			if (m_winHttpConnection == nullptr)
				throw Error::Win32Error("WebSocket::InternalConnect(): WinHttpConnect() failed", GetLastError());

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest
			WinHttpHandle requestHandle = WinHttpOpenRequest(
				m_winHttpConnection.Get(),
				L"GET",
				path.c_str(),
				nullptr,
				nullptr,
				nullptr,
				WINHTTP_FLAG_SECURE
			);
			if (requestHandle == nullptr)
				throw Error::Win32Error("WebSocket::InternalConnect(): WinHttpOpenRequest() failed", GetLastError());

			if (m_settings.IgnoreSslErrors)
			{
				DWORD dwFlags =
					SECURITY_FLAG_IGNORE_UNKNOWN_CA |
					SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
					SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
					SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
				// https://docs.microsoft.com/en-us/windows/win32/winhttp/option-flags
				// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsetoption
				bool setSecurityOption = WinHttpSetOption(
					requestHandle.Get(),
					WINHTTP_OPTION_SECURITY_FLAGS,
					&dwFlags,
					sizeof(dwFlags)
				);
				if (setSecurityOption == false)
					throw Error::Win32Error("WebSocket::InternalConnect(): WinHttpSetOption() failed", GetLastError());
			}

			bool setWebsocketOption = WinHttpSetOption(
				requestHandle.Get(),
				WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
				nullptr,
				0
			);
			if (setWebsocketOption == false)
				throw Error::Win32Error("WebSocket::InternalConnect(): WinHttpSetOption() failed", GetLastError());

			bool success = WinHttpSendRequest(
				requestHandle.Get(),
				WINHTTP_NO_ADDITIONAL_HEADERS,
				0,
				nullptr,
				0,
				0,
				0
			);
			if (success == false)
				throw Error::Win32Error("WebSocket::InternalConnect(): WinHttpSendRequest() failed", GetLastError());

			success = WinHttpReceiveResponse(requestHandle.Get(), 0);
			if (success == false)
				throw Error::Win32Error("WebSocket::InternalConnect(): WinHttpReceiveResponse() failed", GetLastError());

			DWORD statusCode = 0;
			DWORD statusCodeSize = sizeof(statusCode);
			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpqueryheaders
			success = WinHttpQueryHeaders(
				requestHandle.Get(),
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				WINHTTP_HEADER_NAME_BY_INDEX,
				&statusCode,
				&statusCodeSize,
				WINHTTP_NO_HEADER_INDEX
			);
			if (success == false)
				throw Error::Win32Error("WebSocket::InternalConnect(): WinHttpQueryHeaders() failed", GetLastError());

			if (statusCode != 101) // switching protocol
			{
				throw std::runtime_error(
					"Received unexpected HTTP response code while upgrading to websocket: "
					+ std::to_string(statusCode)
				);
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpwebsocketcompleteupgrade
			m_winHttpWebSocket = WinHttpWebSocketCompleteUpgrade(requestHandle.Get(), 0);
			if (m_winHttpWebSocket == nullptr)
				throw Error::Win32Error("WebSocket::InternalConnect(): WinHttpWebSocketCompleteUpgrade() failed", GetLastError());

			requestHandle = nullptr;
			m_status = WebSocketStatus::Connected;
		}
		catch (...)
		{
			m_status = WebSocketStatus::Error;
			throw;
		}
	}

	WebSocketStatus WebSocket::GetStatus()
	{
		return m_status;
	}

	void WebSocket::SendString(const std::string& msg)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");

		DWORD statusCode = WinHttpWebSocketSend(
			m_winHttpWebSocket.Get(),
			WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
			(PVOID)&msg[0],
			(DWORD)(msg.size() * sizeof(char))
		);
		if (statusCode != ERROR_SUCCESS)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Error("WebSocket::SendString(): WinHttpWebSocketSend() failed", statusCode);
		}
	}

	void WebSocket::SendBuffer(const std::vector<char>& buffer)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to send data");

		DWORD statusCode = WinHttpWebSocketSend(
			m_winHttpWebSocket.Get(),
			WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
			(PVOID)&buffer[0],
			(DWORD)(buffer.size() * sizeof(char))
		);
		if (statusCode != ERROR_SUCCESS)
		{
			m_status = WebSocketStatus::Error;
			throw Error::Win32Error("WebSocket::SendString(): WinHttpWebSocketSend() failed", statusCode);
		}
	}

	bool WebSocket::Receive(std::vector<char>& receiveBuffer)
	{
		if (m_status != WebSocketStatus::Connected)
			throw std::runtime_error("WebSocket is not connected to receive data");

		constexpr UINT bufferBlockSize = 2048;
		receiveBuffer.clear();
		receiveBuffer.resize(bufferBlockSize);
		WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
		DWORD bufferLength = (DWORD)(receiveBuffer.size() * sizeof(char));
		DWORD totalBytesTransferred = 0;
		char* currentBufferPointer = &receiveBuffer[0];
		
		while (true)
		{
			DWORD bytesTransferred = 0;
			DWORD statusCode = WinHttpWebSocketReceive(
				m_winHttpWebSocket.Get(),
				currentBufferPointer,
				bufferLength,
				&bytesTransferred,
				&bufferType);
			// If the server terminates the connection, 12030 will returned.
			if (statusCode != ERROR_SUCCESS)
				throw Error::Win32Error("Connection error when receiving websocket data", statusCode);
			
			// The server closed the connection.
			if (bufferType == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
			{
				Close();
				return false;
			}

			currentBufferPointer += bytesTransferred;
			bufferLength -= bytesTransferred;
			totalBytesTransferred += bytesTransferred;

			// We've now got a complete buffer of either binary or UTF8 type.
			if (
				bufferType == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE
				|| bufferType == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE
			)
			{
				receiveBuffer.resize(totalBytesTransferred);
				return true;
			}

			if (bufferLength == 0)
			{
				receiveBuffer.resize(receiveBuffer.size() + bufferBlockSize);
				bufferLength = (DWORD)receiveBuffer.size() * sizeof(char);
				currentBufferPointer = &receiveBuffer[0] + totalBytesTransferred;
			}
		}
	}

	void WebSocket::Close()
	{
		WinHttpWebSocketClose(m_winHttpWebSocket.Get(), WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
		m_status = WebSocketStatus::Closed;
	}
}