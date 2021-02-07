#pragma once
#ifndef SGC_UTILS_H
#define SGC_UTILS_H

#include <cinttypes>
#include <type_traits>

#include "sgc/system.h"

#if defined (SGC_COMPILER_MSVC)
#include <intrin.h>
#endif

namespace sgc {
	namespace gc {
		namespace utils {

#if defined (SGC_COMPILER_GCC) || defined (SGC_COMPILER_CLANG)
#	define BRANCH_LIKELY(x)   __builtin_expect((x),1)
#	define BRANCH_UNLIKELY(x) __builtin_expect((x),0)
#else
#   define BRANCH_LIKELY(x)   (x)
#   define BRANCH_UNLIKELY(x) (x)
#endif

#if defined (SGC_COMPILER_GCC) || defined (SGC_COMPILER_CLANG)
			inline uint8_t count_tzb (std::size_t v) noexcept {
#	if defined (SGC_ARCH_X64)
				return static_cast < uint8_t > (__builtin_ctzl(v));
#	else
				return static_cast < uint8_t > (__builtin_ctz(v));
#	endif
			}
#elif defined (SGC_COMPILER_MSVC)
			inline uint8_t count_tzb(std::size_t v) noexcept {
				unsigned long c{ 0 };
#if defined (SGC_ARCH_X64)
				_BitScanForward64(&c, v);
#else
				_BitScanForward(&c, v);
#endif
				return static_cast < uint8_t > (c);
			}
#endif


			template<typename _t>
			constexpr inline bool is_pow_2(_t v) {
				static_assert(std::is_unsigned<_t>::value, "is_pow_2 does not support signed data types!");
				return (v & (v - 1)) == 0;
			}

			template<class _t>
			constexpr inline _t next_pow_2(_t value) {
				static_assert(std::is_unsigned<_t>::value, "next_pow_2 does not support signed data types!");

				if (is_pow_2(value))
					return value;

				constexpr _t bit_ceil{sizeof(_t) * 8};
				_t bit = 1;

				while (bit < bit_ceil) {
					value |= (value >> bit);
					bit = bit << 1;
				}

				return value + 1;
			}

			template<typename _t>
			inline std::uintptr_t as_integral(_t *ptr) noexcept {
				return reinterpret_cast<std::uintptr_t>(ptr);
			}

			template<typename _t>
			inline _t *as_ptr(std::uintptr_t integral) noexcept {
				return reinterpret_cast < _t * > (integral);
			}

			template<typename _t, typename _v>
			inline _t *as_ptr(_v *ptr) noexcept {
				return reinterpret_cast < _t * > (ptr);
			}

			template<typename _t>
			inline std::size_t padding_of(_t value, std::size_t alignment) noexcept {
				return (alignment - (as_integral(value) & (alignment - 1))) & (alignment - 1);
			}

			template<typename _t>
			inline _t *align_to_next(_t *value, std::size_t alignment) noexcept {
				return as_ptr<_t>(
					(as_integral(value) + (alignment - 1)) & ~(alignment - 1));
			}

			template<typename _t>
			inline _t *align_to_zero_bits(_t *value, std::size_t zero_bits) noexcept {
				return as_ptr<_t>((as_integral(value) >> zero_bits) << zero_bits);
			}

			template<typename _t>
			inline _t *align_to_next_zero_bits(_t *value, uint8_t zero_bits) noexcept {
				return align_to(as_integral(value) + (_t{1} << zero_bits) - _t{1}, zero_bits);
			}

		}
	}
}

#endif //SGC_UTILS_H
