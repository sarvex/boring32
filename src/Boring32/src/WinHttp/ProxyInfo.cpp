#include "pch.hpp"
#include "include/WinHttp/ProxyInfo.hpp"
#include "include/Error/Win32Error.hpp"

namespace Boring32::WinHttp
{
	ProxyInfo::~ProxyInfo()
	{
		Close();
	}

	ProxyInfo::ProxyInfo()
	:	m_info{0},
		m_mustRelease(false)
	{ }

	void ProxyInfo::GetProxyForUrl(
		const HINTERNET session, 
		const std::wstring& url, 
		WINHTTP_AUTOPROXY_OPTIONS& options
	)
	{
		if (WinHttpGetProxyForUrl(session, url.c_str(), &options, &m_info) == false)
			throw Error::Win32Error("ProxyInfo::GetProxyForUrl(): WinHttpGetProxyForUrl() failed", GetLastError());
		m_mustRelease = true;
	}

	void ProxyInfo::Close()
	{
		if (m_mustRelease)
		{
			if (m_info.lpszProxy)
				GlobalFree(m_info.lpszProxy);
			if (m_info.lpszProxyBypass)
				GlobalFree(m_info.lpszProxyBypass);
			m_info.lpszProxy = nullptr;
			m_info.lpszProxyBypass = nullptr;
			m_mustRelease = false;
		}
	}

	void ProxyInfo::SetNamedProxy(
		const std::wstring& proxy,
		const std::wstring& proxyBypass
	)
	{
		Close();
		m_proxy = proxy;
		m_proxyBypass = proxyBypass;
		m_info.lpszProxy = m_proxy.empty() ? nullptr : &m_proxy[0];
		m_info.lpszProxyBypass = m_proxyBypass.empty() ? nullptr : &m_proxyBypass[0];
		m_info.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
	}

	void ProxyInfo::SetAllInfo(
		const std::wstring& proxy,
		const std::wstring& proxyBypass,
		const DWORD accessType
	)
	{
		Close();
		m_proxy = proxy;
		m_proxyBypass = proxyBypass;
		m_info.lpszProxy = m_proxy.empty() ? nullptr : &m_proxy[0];
		m_info.lpszProxyBypass = m_proxyBypass.empty() ? nullptr : &m_proxyBypass[0];
		m_info.dwAccessType = accessType;
	}

	void ProxyInfo::SetOnSession(HINTERNET session)
	{
		if (m_info.lpszProxy == nullptr)
			throw std::runtime_error("No proxy set");
		if (WinHttpSetOption(session, WINHTTP_OPTION_PROXY, &m_info, sizeof(m_info)) == false)
			throw Error::Win32Error("ProxyInfo::SetOnSession(): WinHttpSetOption() failed", GetLastError());
	}
}
