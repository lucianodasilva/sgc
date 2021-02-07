#include "sgc/gc/object_cache.h"
#include "sgc/system.h"
#include "sgc/gc/service.h"

namespace sgc {
	namespace gc {
		stage_result object_cache_collection_stage::collect(sgc::gc::service &service, std::size_t target_it) {
			auto &cache = service._object_cache;
			auto &m_map = service.memory_mapping;

			// on begining of stage get
			// first available region
			if (!region_it) {
				region_it = cache._active_regions.head;
				prev_region_it = nullptr;
			}

			// iterate through all available object regions
			std::size_t it = target_it;
			while (it > 0 && region_it) {

				std::size_t used_blocks{m_map.unit_size - 1};
				// free unused blocks
				for (int i = 1; i < m_map.unit_size; ++i) {
					auto *block_map = m_map.get_unit_map_at_index(region_it, i);

					// if no unit is used from map
					// the first unit map entry is at
					// top level and marked as free
					if (block_map->level == cache.top_level &&
						!block_map->used) {
						// if block not already free
						if (*utils::as_ptr<uint32_t>(block_map) != cache.free_block_token) {
							auto *block = m_map.get_unit_block_at_index(region_it, i);
							// remove from free list
							cache._free_units[cache.top_level].remove(
								utils::as_ptr<available_unit>(block));

							// rewrite available_unit at block_maps
							cache.set_inplace_available_unit(region_it, i);

							// decommit block
							system::decommit(block, m_map.unit_block_size);

							// update metrics
							cache._metrics.used_physical -= m_map.unit_block_size;
						}

						--used_blocks;
					}
				}

				// if region has used blocks
				if (used_blocks > 0) {
					// next region
					prev_region_it = region_it;
					region_it = region_it->next;
				} else {
					// remove free units for this region
					for (int i = 1; i < m_map.unit_size; ++i) {
						auto unit = cache.get_inplace_available_unit(region_it, i);
						cache._free_units[cache.top_level].remove(unit);
					}

					// remove "self" from active regions
					if (prev_region_it) {
						prev_region_it->next = region_it->next;
					} else {
						cache._active_regions.head = region_it->next;
					}

					// move to next region
					auto *release_region = region_it;
					region_it = region_it->next;

					// release region
					system::release(release_region, m_map.unit_region_size);

					// update metrics
					cache._metrics.used_virtual -= m_map.unit_region_size;
					cache._metrics.used_physical -= m_map.unit_block_size;
					cache._metrics.used_system -= m_map.unit_block_size;

				}

				--it;
			}

			// return diference between the target iteration and
			// the specific iteration count
			// if region_it gets to null ( end of list )
			// then the stage was complete
			return {target_it - it, region_it == nullptr};
		}

		constexpr uint32_t make_free_block_token(uint8_t level) {
			union v_type {
				struct {
					map_entry map{false, 0};
					uint8_t token[3]{'F', 'R', 'E'};
				} data;

				uint32_t int_value;
			};

			v_type value{false, level};
			return value.int_value;
		}

		object_cache::object_cache(mapping const &memory_map, sgc::gc::metrics &metrics) :
			_m_map{memory_map},
			_metrics{metrics},
			top_level{level_of(_m_map.unit_block_size)},
			free_block_token{make_free_block_token(top_level)} {}

		object_cache::~object_cache() {
			// should only have active regions on failure or tests
			// but at least we release the reserved memory back
			// to the system
			while (auto *region = _active_regions.pop_front())
				system::release(region, _m_map.unit_region_size);
		}

		object_header *object_cache::reserve_unit(std::size_t size) {

			// validate and align size
			size = utils::next_pow_2(size);

			if (size < _m_map.unit_size)
				size = _m_map.unit_size;

			// search for smallest available level
			auto
				target_level = level_of(size),
				level = target_level;

			while (_free_units[level].empty()) {
				++level;

				if (level > top_level) {
					// if no free available units grow unit availability
					grow_available_units();

					// reset to largest level and break
					level = top_level;
					break;
				}
			}

			// get corresponding unit ready for use
			auto *unit = use_unit(level);

			// split unit to fit
			while (level > target_level) {
				--level;

				auto *to_split = get_unit_pair(unit, level);
				map_as_free(to_split, level);
			}

			*_m_map.get_unit_map_entry(unit) = {true, level};

			// update metrics
			_metrics.used_system += sizeof(object_header);
			_metrics.used_application += (_m_map.unit_size << level);

			return unit;
		}

		void object_cache::release_unit(object_header *unit) {

			auto *block_start = _m_map.get_unit_block(unit);
			auto *node_map = _m_map.get_unit_map(unit);
			auto unit_index = _m_map.get_unit_index(unit);

			// coalescing nodes
			auto level = node_map[unit_index].level;

			// update metrics
			_metrics.used_system -= sizeof(object_header);
			_metrics.used_application -= (_m_map.unit_size << level);

			if (level < top_level) {

				auto pair_index = get_unit_pair(unit_index, level);

				while (
					!node_map[pair_index].used &&
					node_map[pair_index].level == level
					) {
					// merge block and remove available unit
					map_as_used(_m_map.get_unit_at_index(block_start, pair_index), level);

					if (pair_index < unit_index)
						unit_index = pair_index;

					++level;

					if (level == top_level)
						break;

					pair_index = get_unit_pair(unit_index, level);
				}
			}

			map_as_free(_m_map.get_unit_at_index(block_start, unit_index), level);
		}

		void object_cache::set_inplace_available_unit(object_region *region, std::size_t block_index) {
			auto *map_address = _m_map.get_unit_map_at_index(region, block_index);

			// mark map as free
			*utils::as_ptr<uint32_t>(map_address) = free_block_token;

			auto *free_unit = get_inplace_available_unit(region, block_index);

			// add temp location of available unit
			// to remaining map memory
			// to avoid pre-allocating memory blocks
			_free_units[top_level].push_front(free_unit);
		}

		available_unit *object_cache::get_inplace_available_unit(object_region *region, std::size_t block_index) {
			auto *map_address = _m_map.get_unit_map_at_index(region, block_index);

			return utils::as_ptr<available_unit>(
				utils::as_integral(map_address) + sizeof(std::size_t));
		}

		void object_cache::map_as_free(object_header *unit, uint8_t level) {
			_free_units[level].push_front(
				utils::as_ptr<available_unit>(unit));

			*_m_map.get_unit_map_entry(unit) = {false, level};
		}

		void object_cache::map_as_used(object_header *unit, uint8_t level) {
			_free_units[level].remove(
				utils::as_ptr<available_unit>(unit));

			*_m_map.get_unit_map_entry(unit) = {true, level};
		}

		object_header *object_cache::use_unit(uint8_t level) {
			auto *unit = utils::as_ptr<object_header>(_free_units[level].pop_front());

			if (level == top_level) {
				auto *region = _m_map.get_unit_region(unit);

				// detect if block is commited
				auto block_index = _m_map.get_unit_block_index(unit);

				// commit block and replace available_unit address
				if (!block_index) {
					// no data blocks live at block zero
					// therefore if block_index is zero
					// the data block is not committed
					block_index =
						static_cast < decltype(block_index) >
						((utils::as_integral(unit) - utils::as_integral(region)) /
						 _m_map.unit_map_size);

					auto *block_address = _m_map.get_unit_block_at_index(region, block_index);
					system::commit(block_address, _m_map.unit_block_size);

					// update metrics
					_metrics.used_physical += _m_map.unit_block_size;

					unit = utils::as_ptr<object_header>(block_address);
				}
			}

			return unit;
		}

		void object_cache::grow_available_units() {

			// create reserved virtual memory block
			auto *region = utils::as_ptr<object_region>(
				system::reserve(_m_map.unit_region_size, _m_map.unit_region_size));

			// commit header block and format
			system::commit(region, _m_map.unit_block_size);

			// special case to avoid committing all blocks
			// set available_unit address to node_map
			for (std::size_t i = _m_map.unit_size - 1; i > 0; --i)
				set_inplace_available_unit(region, i);

			// store as active region
			_active_regions.push_front(region);

			// update metrics
			_metrics.used_virtual += _m_map.unit_region_size;
			_metrics.used_physical += _m_map.unit_block_size;
			_metrics.used_system += _m_map.unit_block_size;
		}
	}
}