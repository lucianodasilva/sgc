#pragma once
#ifndef SGC_METRICS_H
#define SGC_METRICS_H

#include <cinttypes>

namespace sgc {
	namespace gc {

		struct metrics {
		public:
			std::size_t used_virtual;
			std::size_t used_physical;
			std::size_t used_system;
			std::size_t used_application;

			inline constexpr double get_system_racio() const {
				return static_cast < double > (used_system) / used_application;
			}

			uint32_t objects;
			uint32_t references;
		};
	}
}

#endif