#include "sgc/gc/executor.h"

namespace sgc {
	namespace gc {
		void collection_stage::push_metrics(std::size_t iterations, double elapsed_time_ms) noexcept {
			auto &slot = _it_window[_index];

			_window_time_sum -= slot.first;
			_window_it_sum -= slot.second;

			slot.first = elapsed_time_ms;
			slot.second = iterations;

			_window_time_sum += elapsed_time_ms;
			_window_it_sum += iterations;

			_average_it_time_ms = _window_time_sum / _window_it_sum;

			++_index;

			if (_index == average_window_size)
				_index = 0;
		}
	}
}