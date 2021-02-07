#pragma once
#ifndef SGC_BITSET_H
#define SGC_BITSET_H

#include <vector>
#include <limits>

#include "sgc/gc/utils.h"

namespace sgc {
	namespace details {

		struct bitset {
		public:

			static constexpr std::size_t bitness{ sizeof(std::size_t) * 8 };

			inline static std::size_t bit_to_index (std::size_t bit) noexcept {
				return (bit) / (sizeof (std::size_t) * 8);
			}

			inline void set (std::size_t bit) noexcept {
				auto index =  bit / bitness;
				auto offset = bit % bitness;

				auto mask = std::size_t {1} << offset;
				data () [index] |= mask;
			}

			inline void reset (std::size_t bit) noexcept {
				auto index =  bit / bitness;
				auto offset = bit % bitness;

				auto mask = ~(std::size_t {1} << offset);
				data () [index] &= mask;
			}

			inline bool is_set (std::size_t bit) const noexcept {
				auto index =  bit / bitness;
				auto offset = bit % bitness;

				auto mask = std::size_t {1} << offset;
				return (data () [index] & mask) != 0;
			}

			inline void clear(std::size_t size) {
				auto* ptr = data();
				for (std::size_t i = 0; i < size; ++i)
					ptr [i] = 0;
			}

			inline bool is_empty(std::size_t size) const {
				std::size_t sum {0};
				auto* ptr = data();

				for (std::size_t i = 0; i < size; ++i)
					sum += (ptr [i]);

				return sum == 0;
			}

			inline std::size_t* data() noexcept {
				return gc::utils::as_ptr < std::size_t >(this);
			}

			inline std::size_t const * data() const noexcept {
				return gc::utils::as_ptr < std::size_t const > (this);
			}
		};

	}
}

#endif //SGC_BITSET_H
