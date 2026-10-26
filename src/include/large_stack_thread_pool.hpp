#pragma once

#include "duckdb/common/exception.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <vector>

#ifndef _WIN32
#include <pthread.h>
#endif

namespace duckdb {

// httplib's ThreadPool runs requests on std::thread, whose stack (512 KB on macOS, 1 MB on Windows) overflows
// when DuckDB plans a deep query. These workers get the 8 MB a Linux or macOS main thread has.
class LargeStackThreadPool final : public duckdb_httplib_openssl::TaskQueue {
public:
	static constexpr size_t STACK_SIZE = 8 * 1024 * 1024;

	explicit LargeStackThreadPool(size_t thread_count) {
		for (size_t i = 0; i < thread_count; i++) {
			threads.push_back(StartThread());
		}
	}

	bool enqueue(std::function<void()> fn) override {
		{
			std::lock_guard<std::mutex> lock(mutex);
			if (shutting_down) {
				return false;
			}
			jobs.push_back(std::move(fn));
		}
		cond.notify_one();
		return true;
	}

	void shutdown() override {
		{
			std::lock_guard<std::mutex> lock(mutex);
			shutting_down = true;
		}
		cond.notify_all();
		for (auto &thread : threads) {
			Join(thread);
		}
		threads.clear();
	}

private:
	void Work() {
		while (true) {
			std::function<void()> fn;
			{
				std::unique_lock<std::mutex> lock(mutex);
				cond.wait(lock, [this] { return shutting_down || !jobs.empty(); });
				if (jobs.empty()) {
					return;
				}
				fn = std::move(jobs.front());
				jobs.pop_front();
			}
			fn();
		}
	}

#ifdef _WIN32
	using thread_t = HANDLE;

	static DWORD WINAPI Run(LPVOID pool) {
		static_cast<LargeStackThreadPool *>(pool)->Work();
		return 0;
	}

	thread_t StartThread() {
		auto thread = CreateThread(nullptr, STACK_SIZE, Run, this, STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr);
		if (!thread) {
			throw IOException("Could not start an HTTP worker thread");
		}
		return thread;
	}

	static void Join(thread_t thread) {
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
	}
#else
	using thread_t = pthread_t;

	static void *Run(void *pool) {
		static_cast<LargeStackThreadPool *>(pool)->Work();
		return nullptr;
	}

	thread_t StartThread() {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, STACK_SIZE);
		thread_t thread;
		auto rc = pthread_create(&thread, &attr, Run, this);
		pthread_attr_destroy(&attr);
		if (rc != 0) {
			throw IOException("Could not start an HTTP worker thread");
		}
		return thread;
	}

	static void Join(thread_t thread) {
		pthread_join(thread, nullptr);
	}
#endif

	std::vector<thread_t> threads;
	std::list<std::function<void()>> jobs;
	bool shutting_down = false;
	std::mutex mutex;
	std::condition_variable cond;
};

} // namespace duckdb
