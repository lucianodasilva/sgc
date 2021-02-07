#pragma once
#ifndef SGC_UNITS_H
#define SGC_UNITS_H

namespace sgc {
	namespace units {

		inline constexpr std::size_t kbytes (std::size_t v) {
			return v << 6;
		}

		inline constexpr std::size_t mbytes (std::size_t v) {
			return v << 20;
		}

	}
}

#endif //SGC_SGC_UNITS_H
