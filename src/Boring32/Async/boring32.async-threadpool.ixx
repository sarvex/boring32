export module boring32.async:threadpool;
import boring32.error;
import <functional>;
import <memory>;
import <source_location>;
import <win32.hpp>;

export namespace Boring32::Async::ThreadPools
{
	typedef void
		(*ThreadPoolCallback)(
			PTP_CALLBACK_INSTANCE Instance,
			void* Parameter,
			PTP_WORK              Work
		);

	class ThreadPool
	{
		public:
			using LambdaCallback = std::function<
				void(PTP_CALLBACK_INSTANCE Instance, void*, PTP_WORK)
			>;
			using WorkParamTuple = std::tuple<LambdaCallback&, void*>;

			template<typename T>
			struct WorkItem
			{
				using Lambda = std::function<
					void(PTP_CALLBACK_INSTANCE Instance, T, PTP_WORK)
				>;
				Lambda Callback;
				T Parameter = nullptr;
				PTP_WORK Item = nullptr;
			};

			virtual ~ThreadPool();
			ThreadPool();
			ThreadPool(const DWORD minThreads, const DWORD maxThreads);

		public:
			virtual DWORD GetMinThread() const noexcept final;
			virtual DWORD GetMaxThread() const noexcept final;

		public:
			virtual void SetMinAndMaxThreads(const DWORD min, const DWORD max);
			virtual void SetMaxThreads(const DWORD value);
			virtual void SetMinThreads(const DWORD value);
			virtual void Close();


			[[nodiscard("Return value should remain live until callback is fully completed")]]
			virtual PTP_WORK CreateWork(
				ThreadPoolCallback& callback,
				void* param
			);

			template<typename T>
			void CreateWork(WorkItem<T>& outWorkItem)
			{
				if (m_pool == nullptr)
					throw Error::Boring32Error("m_pool is nullptr");

				outWorkItem.Item = CreateThreadpoolWork(
					InternalCallback, 
					&outWorkItem, 
					&m_environ
				);
				if (outWorkItem.Item == nullptr)
				{
					const auto location = std::source_location::current();
					// using current() directly causes a compiler internal error
					throw Error::Win32Error("CreateThreadpoolWork() failed", GetLastError());
				}
			}

			template<typename T>
			void CreateWork(WorkItem<T>&& workItem) = delete;

			virtual void SubmitWork(PTP_WORK workItem);
			virtual void SetCallbackRunsLong();
			virtual std::shared_ptr<TP_POOL> GetPoolHandle() const noexcept final;

		protected:
			static void InternalCallback(
				PTP_CALLBACK_INSTANCE instance, 
				void* parameter, 
				PTP_WORK work
			);
			static void ValidateArgs(const DWORD minThreads, const DWORD maxThreads);

			virtual ThreadPool& Copy(const ThreadPool& other);
			virtual ThreadPool& Move(ThreadPool& other) noexcept;
			virtual void InternalCreate();

		protected:
			std::shared_ptr<TP_POOL> m_pool;
			TP_CALLBACK_ENVIRON m_environ;
			DWORD m_minThreads;
			DWORD m_maxThreads;
	};
}
