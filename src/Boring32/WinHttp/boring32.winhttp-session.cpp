module boring32.winhttp:session;
import boring32.error;
import :winhttperror;
import <stdexcept>;

namespace Boring32::WinHttp
{
	Session::~Session()
	{
		Close();
	}

	Session::Session()
	:	m_userAgent(L""),
		m_proxyType(ProxyType::AutoProxy)
	{ }

	Session::Session(std::wstring userAgent)
	:	m_userAgent(std::move(userAgent)), 
		m_proxyType(ProxyType::AutoProxy)
	{
		InternalCreate();
	}

	Session::Session(std::wstring userAgent, const ProxyType proxyType)
	:	m_userAgent(std::move(userAgent)),
		m_proxyType(proxyType)
	{
		if (m_proxyType == ProxyType::NamedProxy)
			throw std::invalid_argument("Named proxy cannot be specified as an option for this constructor");
		InternalCreate();
	}

	Session::Session(std::wstring userAgent, std::wstring namedProxy)
	:	m_userAgent(std::move(userAgent)),
		m_proxyType(ProxyType::NamedProxy),
		m_namedProxy(std::move(namedProxy))
	{
		InternalCreate();
	}

	Session::Session(std::wstring userAgent, std::wstring namedProxy, std::wstring proxyBypass)
	:	m_userAgent(std::move(userAgent)),
		m_proxyType(ProxyType::NamedProxy),
		m_namedProxy(std::move(namedProxy)),
		m_proxyBypass(std::move(proxyBypass))
	{
		InternalCreate();
	}

	Session::Session(const Session& other)
	{
		Copy(other);
	}

	Session& Session::operator=(const Session& other)
	{
		Copy(other);
		return *this;
	}

	Session& Session::Copy(const Session& other)
	{
		if (this == &other)
			return *this;
		Close();
		m_proxyType = other.m_proxyType;
		m_userAgent = other.m_userAgent;
		m_namedProxy = other.m_namedProxy;
		m_proxyBypass = other.m_proxyBypass;
		m_session = other.m_session;
		return *this;
	}

	Session::Session(Session&& other) noexcept
	{
		Move(other);
	}

	Session& Session::operator=(Session&& other) noexcept
	{
		Move(other);
		return *this;
	}

	Session& Session::Move(Session& other) noexcept
	{
		Close();
		m_session = std::move(other.m_session);
		m_proxyType = other.m_proxyType;
		m_userAgent = std::move(other.m_userAgent);
		m_namedProxy = std::move(other.m_namedProxy);
		m_proxyBypass = std::move(other.m_proxyBypass);
		other.Close();
		return *this;
	}

	HINTERNET Session::GetSession() const noexcept
	{
		return m_session.get();
	}

	void Session::Close() noexcept
	{
		m_session = nullptr;
		m_proxyType = ProxyType::NoProxy;
		m_userAgent.clear();
		m_namedProxy.clear();
		m_proxyBypass.clear();
	}

	ProxyType Session::GetProxyType() const noexcept
	{
		return m_proxyType;
	}

	const std::wstring& Session::GetUserAgent() const noexcept
	{
		return m_userAgent;
	}
	
	const std::wstring& Session::GetNamedProxy() const noexcept
	{
		return m_namedProxy;
	}

	const std::wstring& Session::GetProxyBypass() const noexcept
	{
		return m_proxyBypass;
	}

	void Session::InternalCreate()
	{
		if (m_userAgent.empty())
			throw WinHttpError("UserAgent cannot be empty");
		if (static_cast<DWORD>(m_proxyType) == WINHTTP_ACCESS_TYPE_NAMED_PROXY && m_namedProxy.empty())
			throw WinHttpError("ProxyName parameter is required when access type is NamedProxy");

		const wchar_t* proxyType = static_cast<DWORD>(m_proxyType) == WINHTTP_ACCESS_TYPE_NAMED_PROXY
			? m_namedProxy.c_str() 
			: WINHTTP_NO_PROXY_NAME;
		const wchar_t* proxyBypass = m_proxyBypass.empty()
			? WINHTTP_NO_PROXY_BYPASS
			: m_proxyBypass.c_str();
		// https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopen
		const HINTERNET handle = WinHttpOpen(
			m_userAgent.c_str(),
			static_cast<DWORD>(m_proxyType),
			proxyType,
			proxyBypass,
			0
		);
		if (!handle)
		{
			const auto lastError = GetLastError();
			Error::ThrowNested(
				Error::Win32Error("WinHttpOpen() failed", lastError),
				WinHttpError("Failed to open WinHttpSession handle")
			);
		}
		
		m_session = SharedWinHttpSession(handle, WinHttpCloseHandle);
	}
}
