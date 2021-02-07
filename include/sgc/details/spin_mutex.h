#pragma once
#ifndef SGC_SPIN_MUTEX_H
#define SGC_SPIN_MUTEX_H

#include <atomic>
#include <mutex>

namespace sgc {
	namespace details {

		struct spin_mutex {
		public:
			inline void lock() {
				while (_lockless_flag.test_and_set(std::memory_order_acquire)) {}
			}

			inline void unlock() {
				_lockless_flag.clear(std::memory_order_release);
			}

		private:
			std::atomic_flag _lockless_flag = ATOMIC_FLAG_INIT;
		};

		using spin_guard = std::lock_guard<spin_mutex>;
	}
}

#endif //SGC_SPIN_MUTEX_H
