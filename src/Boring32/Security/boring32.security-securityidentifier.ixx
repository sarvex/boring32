export module boring32.security:securityidentifier;
import <vector>;
import <string>;
import <win32.hpp>;
import boring32.error;
import boring32.raii;
import <stdexcept>;
import <iostream>;

export namespace Boring32::Security
{
	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-sid
	class SecurityIdentifier
	{
		public:
			virtual ~SecurityIdentifier()
			{
				Close();
			}

			SecurityIdentifier() = default;

			SecurityIdentifier(const SecurityIdentifier& other)
				: m_sid(nullptr)
			{
				Copy(other);
			}

			SecurityIdentifier(SecurityIdentifier&& other) noexcept
				: m_sid(nullptr)
			{
				Move(other);
			}

			SecurityIdentifier(
				const SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
				const std::vector<DWORD>& subAuthorities
			) : m_sid(nullptr)
			{
				Create(pIdentifierAuthority, subAuthorities);
			}

			SecurityIdentifier(const std::wstring& sidString)
				: SecurityIdentifier()
			{
				Create(sidString);
			}

		public:
			virtual SecurityIdentifier& operator=(const SecurityIdentifier& other)
			{
				Copy(other);
				return *this;
			}

			virtual SecurityIdentifier& operator=(SecurityIdentifier&& other) noexcept
			{
				Move(other);
				return *this;
			}

			virtual operator std::wstring() const
			{
				LPWSTR string = nullptr;
				// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertsidtostringsidw
				const bool succeeded = ConvertSidToStringSidW(m_sid, &string);
				if (!succeeded)
					throw Error::Win32Error("ConvertSidToStringSidW() failed", GetLastError());
				RAII::LocalHeapUniquePtr<wchar_t> ptr(string);
				return string;
			}

		public:
			virtual void Close()
			{
				if (!m_sid)
					return;
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-freesid
				if (FreeSid(m_sid))
					std::wcerr << L"Failed to release SID\n";
				m_sid = nullptr;
			}

			virtual PSID GetSid() const noexcept
			{
				return m_sid;
			}

			virtual BYTE GetSubAuthorityCount() const
			{
				if (!m_sid)
					return 0;
				if (!IsValidSid(m_sid))
					throw Error::Boring32Error("Invalid SID");

				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthoritycount
				PUCHAR authorityCount = GetSidSubAuthorityCount(m_sid);
				return static_cast<BYTE>(*authorityCount);
			}

			virtual SID_IDENTIFIER_AUTHORITY GetIdentifierAuthority() const
			{
				if (!m_sid)
					throw Error::Boring32Error("No valid SID");
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsididentifierauthority
				if (PSID_IDENTIFIER_AUTHORITY identifier = GetSidIdentifierAuthority(m_sid))
					return *identifier;
				throw Error::Win32Error("GetSidIdentifierAuthority() failed", GetLastError());
			}

			virtual DWORD GetSubAuthority(const DWORD index) const
			{
				if (!m_sid)
					throw Error::Boring32Error("No valid SID");
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthority
				if (PDWORD returnVal = GetSidSubAuthority(m_sid, index))
					return *returnVal;
				throw Error::Win32Error("GetSidSubAuthority() failed", GetLastError());
			}

			virtual std::vector<DWORD> GetAllSubAuthorities() const
			{
				if (!m_sid)
					return {};

				std::vector<DWORD> returnVal;
				for (BYTE i = 0, count = GetSubAuthorityCount(); i < count; i++)
					returnVal.push_back(GetSubAuthority(i));
				return returnVal;
			}

		protected:
			virtual void Copy(const SecurityIdentifier& other)
			{
				if (&other == this)
					return;

				Close();
				if (!other.m_sid)
					return;

				Create(other.GetIdentifierAuthority(), other.GetAllSubAuthorities());
			}

			virtual void Move(SecurityIdentifier& other) noexcept
			{
				Close();
				if (!other.m_sid)
					return;

				m_sid = other.m_sid;
				other.m_sid = nullptr;
			}

			virtual void Create(const std::wstring& sidString)
			{
				if (sidString.empty())
					throw Error::Boring32Error("sidString cannot be empty");
				// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
				if (!ConvertStringSidToSidW(sidString.c_str(), &m_sid))
					throw Error::Win32Error("ConvertStringSidToSidW() failed", GetLastError());
			}

			virtual void Create(
				const SID_IDENTIFIER_AUTHORITY& identifierAuthority,
				const std::vector<DWORD>& subAuthorities
			)
			{
				// What's the reason for doing this?
				std::vector<DWORD> subAuthorities2 = subAuthorities;
				if (subAuthorities2.size() != 8)
					subAuthorities2.resize(8);

				const bool succeeded = AllocateAndInitializeSid(
					&const_cast<SID_IDENTIFIER_AUTHORITY&>(identifierAuthority),
					static_cast<BYTE>(subAuthorities.size()),
					subAuthorities2[0],
					subAuthorities2[1],
					subAuthorities2[2],
					subAuthorities2[3],
					subAuthorities2[4],
					subAuthorities2[5],
					subAuthorities2[6],
					subAuthorities2[7],
					&m_sid
				);
				if (!succeeded)
					throw Error::Win32Error("failed to initialise SID", GetLastError());
			}
			

		protected:
			PSID m_sid = nullptr;
	};
}