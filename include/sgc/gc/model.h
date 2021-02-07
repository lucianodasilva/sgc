#pragma once
#ifndef SGC_TYPES_H
#define SGC_TYPES_H

#include <cinttypes>

#include "sgc/details/bitset.h"
#include "sgc/details/link_chain.h"
#include "utils.h"

/*
 * -- OBJECT REGION DEFINITION --
 *  -------------------------  ------------
 * |         |         |     \ \ |         |
 * | BLOCK 0 | BLOCK 1 | ... / / | BLOCK N |
 * | HEADERS |  DATA   |     \ \ |  DATA   |
 * |         |         |     / / |         |
 *  -------------------------  ------------
 *
 * -- HEADER BLOCK DEFINITION ---
 *  ------------------------------------  ----------------
 * | REG NODES | BLK MAP 1 | BLK MAP 2 | \ \ |  BLK MAP N |
 *  --------------------------------------- --------------
 *
 *  -- BLOCK MAP DEFINITION -----
 *  ---------------------------------------------  -------------
 * | USED_0 | LEVEL_0 | USED_1 | LEVEL_1 | USED_2 \ \ | LEVEL_N |
 *  ------------------------------------------------  ----------
 *  used 	: 1 bit
 *  level 	: 7 bits
 *
 */

namespace sgc {
	namespace gc {

		enum struct unit_stage : uint8_t {
			white = 0,
			gray,
			black
		};

		using destructor = void (*)(void *address);

		struct object_header;

		struct reference : public details::link_t<reference> {
			object_header * to;
			bool active; // HACK: TOO BIG
		};

		struct object_header : public details::dual_link_t<object_header> {
		public:
			destructor dctor;
			details::link_chain<reference> references;
			unit_stage stage; // HACK: could be squished somewhere else

			inline void *get_object() const noexcept {
				using namespace utils;
				return as_ptr<void>
					(as_integral(this) + sizeof(object_header));
			}
		};

		struct map_entry {
			bool used : 1;
			uint8_t level : 7;
		};

		struct available_unit :
			public details::dual_link_t<available_unit> {
		};

		struct object_region :
			public details::link_t<object_region> {
		};

		struct ref_region
			: public details::dual_link_t <ref_region >
		{
			std::size_t 			active_count;
			details::link_chain < reference >
									available_refs;
		};

		struct object_block {
		};

		struct mapping {

			inline object_region *get_unit_region(object_header *object) const {
				using namespace utils;
				return as_ptr<object_region>(
					align_to_zero_bits(object, unit_region_zbit));
			}

			inline object_block *get_unit_block(object_header *object) const {
				using namespace utils;
				return as_ptr<object_block>(
					align_to_zero_bits(object, unit_block_zbit));
			}

			inline std::size_t get_unit_index(object_header *object) const {
				using namespace utils;
				return get_offset_from_block(object) >> unit_size_zbit;
			}

			inline object_header *get_unit_at_index(object_block *block, std::size_t unit_index) const {
				using namespace utils;
				return as_ptr<object_header>(
					as_integral(block) +
					(unit_size * unit_index));
			}

			inline uint8_t get_unit_block_index(object_header *object) const {
				using namespace utils;
				return static_cast <uint8_t> (
					get_offset_from_region(object) >> unit_block_zbit
				);
			}

			inline object_block *get_unit_block_at_index(object_region *region, uint8_t block_index) const {
				using namespace utils;
				return as_ptr<object_block>(
					as_integral(region) +
					(unit_block_size * block_index));
			}

			inline map_entry *get_unit_map_at_index(object_region *region, std::size_t block_index) const {
				using namespace utils;

				if (!block_index)
					return nullptr;

				return as_ptr<map_entry>(
					as_integral(region) +
					(unit_map_size * block_index));
			}

			inline map_entry *get_unit_map(object_header *object) const {
				return get_unit_map_at_index(
					get_unit_region(object),
					get_unit_block_index(object));
			}

			inline map_entry *get_unit_map_entry(object_header *object) const {
				return
					get_unit_map(object) +
					get_unit_index(object);
			}

			inline std::size_t get_offset_from_region(object_header *object) const {
				using namespace utils;
				return
					as_integral(object) -
					as_integral(get_unit_region(object));
			}

			inline std::size_t get_offset_from_block(object_header *object) const {
				using namespace utils;
				return
					as_integral(object) -
					as_integral(get_unit_block(object));
			}

			inline ref_region *get_ref_region(reference *ref) const {
				using namespace utils;
				return as_ptr<ref_region>(
					align_to_zero_bits(ref, ref_region_zbit));
			}

			inline reference *get_ref_block(ref_region *region) const {
				using namespace utils;

				return as_ptr<reference>(
					as_integral(region) + sizeof(ref_region));
			}

			inline static constexpr std::size_t calc_ref_count(std::size_t region_size) noexcept {
				auto res =
					(region_size - sizeof(ref_region)) /
					sizeof(reference);

				return static_cast <std::size_t> (res);
			}

			inline mapping(std::size_t block_size, std::size_t unit_size_) :
				unit_size{ utils::next_pow_2(
					sizeof (object_region) > unit_size_ ?
					sizeof (object_region) :
					unit_size_
				) },
				unit_block_size{
					unit_size > block_size ?
					unit_size :
					utils::next_pow_2(block_size)
				},
				unit_region_size	{unit_block_size * unit_size},
				unit_map_size		{unit_block_size / unit_size},
				ref_region_size		{unit_block_size},
				ref_count_per_region{calc_ref_count(ref_region_size)},
				unit_size_zbit		{static_cast <uint8_t> (utils::count_tzb(unit_size))},
				unit_block_zbit		{static_cast <uint8_t> (utils::count_tzb(unit_block_size))},
				unit_region_zbit	{static_cast <uint8_t> (utils::count_tzb(unit_region_size))},
				ref_region_zbit		{static_cast <uint8_t> (utils::count_tzb(ref_region_size))}
			{}

			std::size_t const
				unit_size,
				unit_block_size,
				unit_region_size,
				unit_map_size,
				ref_region_size,
				ref_count_per_region;

			uint8_t const
				unit_size_zbit,
				unit_block_zbit,
				unit_region_zbit,
				ref_region_zbit;
		};
	}
}

#endif //SGC_TYPES_H
