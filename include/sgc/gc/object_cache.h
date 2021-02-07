#pragma once
#ifndef SGC_OBJECT_CACHE_H
#define SGC_OBJECT_CACHE_H

#include <cinttypes>

#include "executor.h"
#include "metrics.h"
#include "model.h"

namespace sgc {
	namespace gc {
		class object_cache_collection_stage : public collection_stage {
		public:
			stage_result collect(sgc::gc::service &service, std::size_t target_it) override;

		private:
			object_region
				*region_it{nullptr},
				*prev_region_it{nullptr};
		};

		struct object_cache {
		public:

			friend class object_cache_collection_stage;

			using free_unit_table = details::link_chain<
				available_unit,
				details::policies::dual
			>[255];

			// constructor / destructor
			explicit object_cache(mapping const &memory_map, sgc::gc::metrics &metrics);

			~object_cache();

			// actual external api
			object_header *reserve_unit(std::size_t size);

			void release_unit(object_header *unit);

			// internal element getters
			inline free_unit_table const &get_free_units() const noexcept {
				return _free_units;
			}

			inline details::link_chain <object_region> const &get_active_regions() const noexcept {
				return _active_regions;
			}

			// calculate pairs
			inline std::size_t get_unit_pair(std::size_t unit_index, uint8_t level) const {
				return unit_index ^ (std::size_t{1} << level);
			}

			inline object_header *get_unit_pair(object_header *unit, uint8_t level) const {
				auto index = _m_map.get_unit_index(unit);
				auto pair_index = get_unit_pair(index, level);

				return _m_map.get_unit_at_index(
					_m_map.get_unit_block(unit),
					pair_index);
			}

			// calculate level of size ( must be pow2 )
			inline uint8_t level_of(std::size_t size) const noexcept {
				return utils::count_tzb(size) - utils::count_tzb(_m_map.unit_size);
			}

		private:

			void set_inplace_available_unit(object_region *region, std::size_t block_index);

			available_unit *get_inplace_available_unit(object_region *region, std::size_t block_index);

			void map_as_free(object_header *unit, uint8_t level);

			void map_as_used(object_header *unit, uint8_t level);

			object_header *use_unit(uint8_t level);

			void grow_available_units();

			mapping const &_m_map;
			metrics &_metrics;

			free_unit_table _free_units;
			details::link_chain <object_region>
				_active_regions;

		public:
			std::uint8_t const
				top_level;
			uint32_t const free_block_token;
		};
	}
}

#endif //SGC_OBJECT_CACHE_H
