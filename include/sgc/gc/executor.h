#pragma once
#ifndef SGC_EXECUTOR_H
#define SGC_EXECUTOR_H

#include <array>
#include <cinttypes>
#include <chrono>
#include <initializer_list>
#include <memory>
#include <vector>

namespace sgc {
	namespace gc {

		struct service;

		// TODO: think about too "tight" timings, zero iterations is ambiguous, could mean both unrestricted execution and no execution
		struct stage_result {
			std::size_t const iterations;
			bool const completed;
		};

		class collection_stage {
		public:

			virtual ~collection_stage() = default;

			virtual stage_result collect(sgc::gc::service &service, std::size_t target_it) = 0;

			static constexpr uint32_t average_window_size{100};

			void push_metrics(std::size_t iterations, double elapsed_time_ms) noexcept;

			inline std::size_t get_average_iterations(double target_time_ms) const noexcept {
				auto res = static_cast < std::size_t > (target_time_ms / _average_it_time_ms);
				return res ? res : 1;
			}

		private:

			std::array<std::pair<double, std::size_t>, average_window_size>
				_it_window;

			double _average_it_time_ms{0};

			double _window_time_sum{0};
			double _window_it_sum{0};

			std::size_t _index{0};
		};

		template<typename ... _stages_tv>
		struct executor {
		public:

			inline executor() noexcept {
				using sink_t = int[];
				sink_t{(_stages.push_back(std::make_unique<_stages_tv>()), 0)...};
			}

			inline void run(sgc::gc::service &service, double target_time_ms) {
				using namespace std;
				using namespace std::chrono;

				using double_milliseconds = std::chrono::duration<double, std::milli>;

				// to check for state_advancement
				// if we do a full loop, there is
				// no need to continue execution
				std::size_t step_count{0};

				do {
					// TODO: study this a bit better for proper resolution
					auto start_stamp = high_resolution_clock::now();

					auto *stage = _stages[_stage_index].get();

					// execute stage
					// calculate expected iterations for time window
					auto target_it = stage->get_average_iterations(target_time_ms);

					// run stage for calculated iterations
					stage_result res{stage->collect(service, target_it)};

					double_milliseconds elapsed_time{high_resolution_clock::now() - start_stamp};

					// push execution metrics for continuous
					// calculation of average task performance
					stage->push_metrics(res.iterations, elapsed_time.count());

					// advance stage as needed
					if (res.completed) {
						++_stage_index;
						if (_stage_index == _stages.size())
							_stage_index = 0;
						++step_count;
					}

					target_time_ms -= elapsed_time.count();

					// run until time runs out or a full cycle is complete
				} while (target_time_ms > 0 && step_count < _stages.size());
			}

			inline void run_full_cycle(sgc::gc::service &service) {
				do {

					auto *stage = _stages[_stage_index].get();

					// execute stage
					// run stage for calculated iterations
					stage_result res{stage->collect(service, std::numeric_limits<std::size_t>::max())};

					// advance stage as needed
					if (res.completed) {
						++_stage_index;
						if (_stage_index == _stages.size())
							_stage_index = 0;

					}

					// run until time runs out or a full cycle is complete
				} while (_stage_index > 0);
			}

		private:
			std::vector<std::unique_ptr<collection_stage> >
				_stages;
			std::size_t _stage_index{0};

		};
	}

}

#endif //SGC_EXECUTOR_H
