module boring32.async:timerqueue;
import boring32.error;
import <iostream>;

namespace Boring32::Async
{
	TimerQueue::~TimerQueue()
	{
		Close(std::nothrow);
	}

	TimerQueue::TimerQueue(const Async::Event& completionEvent)
	:	m_completionEvent(completionEvent),
		m_waitForAllCallbacks(true)
	{
		if (!m_completionEvent)
			throw Error::Boring32Error("completionEvent cannot be nullptr");
		if (m_completionEvent == INVALID_HANDLE_VALUE)
			throw Error::Boring32Error("completionEvent cannot be INVALID_HANDLE_VALUE");
		InternalCreate();
	}

	TimerQueue::TimerQueue(const bool waitForAllCallbacks)
	:	m_timer(nullptr),
		m_waitForAllCallbacks(waitForAllCallbacks)
	{
		InternalCreate();
	}

	TimerQueue::TimerQueue(TimerQueue&& other) noexcept
	{
		Move(other);
	}

	TimerQueue& TimerQueue::operator=(TimerQueue&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void TimerQueue::Move(
		TimerQueue& other
	) noexcept try
	{
		Close(std::nothrow);
		m_completionEvent = other.m_completionEvent;
		m_timer = other.m_timer;
		other.m_timer = nullptr;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << ex.what() << std::endl;
	}

	void TimerQueue::Close()
	{
		if (!m_timer)
			return;
		
		HANDLE argValue = INVALID_HANDLE_VALUE; // waits for all callbacks on deletion
		if (m_completionEvent)
			argValue = m_completionEvent.GetHandle(); // waits for all callbacks on deletion and signals event
		else if (!m_waitForAllCallbacks)
			argValue = nullptr; // does not wait

		//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-deletetimerqueueex
		if (!DeleteTimerQueueEx(m_timer, argValue))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("DeleteTimerQueueEx() failed", lastError);
		}
		m_timer = nullptr;
	}

	bool TimerQueue::Close(
		const std::nothrow_t&
	) noexcept try
	{
		Close();
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << ex.what() << std::endl;
		return false;
	}

	HANDLE TimerQueue::GetHandle() const noexcept
	{
		return m_timer;
	}

	void TimerQueue::InternalCreate()
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-createtimerqueue
		m_timer = CreateTimerQueue();
		if (m_timer == nullptr)
			throw Error::Win32Error("CreateTimerQueue() failed", GetLastError());
	}
}
