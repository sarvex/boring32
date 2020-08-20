#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Exception.hpp"
#include "include/Raii/Win32Handle.hpp"
#include "include/Util/Util.hpp"

namespace Boring32::Raii
{
	Win32Handle::~Win32Handle()
	{
		InternalClose(false);
	}

	void Win32Handle::InternalClose(const bool throwOnFailure)
	{
		if (m_handle)
		{
			if (CloseHandle(m_handle) == false && throwOnFailure)
				throw Error::Win32Exception("CloseHandle() failed", GetLastError());
			m_handle = nullptr;
		}
	}

	void Win32Handle::Close()
	{
		InternalClose(true);
	}

	Win32Handle::Win32Handle()
	:	m_handle(nullptr)
	{ }
	
	Win32Handle::Win32Handle(const Win32Handle& otherHandle)
	:	m_handle(nullptr)
	{
		Copy(otherHandle);
	}

	void Win32Handle::operator=(const Win32Handle& other)
	{
		Copy(other);
	}

	void Win32Handle::Copy(const Win32Handle& other)
	{
		Close();
		m_handle = Win32Handle::DuplicatePassedHandle(other.GetHandle(), other.IsInheritable());
	}

	Win32Handle::Win32Handle(Win32Handle&& other) noexcept
	:	m_handle(nullptr)
	{
		Move(other);
	}

	void Win32Handle::operator=(Win32Handle&& other) noexcept
	{
		Move(other);
	}

	void Win32Handle::Move(Win32Handle& other) noexcept
	{
		Close();
		m_handle = other.m_handle;
		other.m_handle = nullptr;
	}

	Win32Handle::Win32Handle(const HANDLE handle)
	:	m_handle(handle)
	{ }

	void Win32Handle::operator=(const HANDLE other)
	{
		Close();
		m_handle = other;
	}

	bool Win32Handle::operator==(const HANDLE other) const
	{
		return m_handle == other;
	}

	bool Win32Handle::operator==(const Win32Handle& other) const
	{
		return m_handle == other.m_handle;
	}

	HANDLE* Win32Handle::operator&()
	{
		return &m_handle;
	}

	HANDLE Win32Handle::GetHandle() const
	{
		return m_handle;
	}

	HANDLE& Win32Handle::GetHandleAddress()
	{
		return m_handle;
	}

	HANDLE Win32Handle::DuplicateCurrentHandle() const
	{
		return Win32Handle::DuplicatePassedHandle(m_handle, IsInheritable());
	}

	bool Win32Handle::IsInheritable() const
	{
		return Win32Handle::HandleIsInheritable(m_handle);
	}

	void Win32Handle::SetInheritability(const bool isInheritable)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("Handle is null.");
		if (SetHandleInformation(m_handle, HANDLE_FLAG_INHERIT, isInheritable) == false)
			throw Error::Win32Exception("SetHandleInformation() failed", GetLastError());
	}

	HANDLE Win32Handle::Detach()
	{
		HANDLE temp = m_handle;
		m_handle = nullptr;
		return temp;
	}

	bool Win32Handle::HandleIsInheritable(const HANDLE handle)
	{
		if (handle == nullptr)
			return false;
		DWORD flags = 0;
		if (GetHandleInformation(handle, &flags) == false)
			throw Error::Win32Exception("GetHandleInformation() failed", GetLastError());
		return flags & HANDLE_FLAG_INHERIT;
	}

	HANDLE Win32Handle::DuplicatePassedHandle(const HANDLE handle, const bool isInheritable)
	{
		if (handle == nullptr)
			return nullptr;

		HANDLE duplicateHandle = nullptr;
		bool succeeded = DuplicateHandle(
			GetCurrentProcess(),
			handle,
			GetCurrentProcess(),
			&duplicateHandle,
			0,
			isInheritable,
			DUPLICATE_SAME_ACCESS
		);
		if (succeeded == false)
			throw Error::Win32Exception("DuplicateHandle() failed", GetLastError());

		return duplicateHandle;
	}
}