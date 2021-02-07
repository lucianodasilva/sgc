#include "sgc/gc/ref_cache.h"
#include "sgc/gc/service.h"
#include "sgc/system.h"

namespace sgc {
	namespace gc {
		stage_result ref_cache_collection_stage::collect(
			sgc::gc::service &service,
			std::size_t target_it
		) {
			auto &cache = service._ref_cache;
			auto &m_map = service.memory_mapping;

			// on begining of stage get
			// first available region
			if (!region_it) {
				region_it = cache._active_regions.head;
				prev_region_it = nullptr;
			}

			// iterate through all available reference regions
			std::size_t it = target_it;
			while (it > 0 && region_it) {

				// if region
				if (region_it->active_count != 0) {
					// next region
					prev_region_it = region_it;
					region_it = region_it->next;
				} else {

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
					system::release(release_region, m_map.ref_region_size);

					// update metrics
					cache._metrics.used_virtual -= m_map.ref_region_size;
					cache._metrics.used_physical -= m_map.ref_region_size;
					cache._metrics.used_system -= m_map.ref_region_size;
				}

				--it;
			}

			// return difference between the target iteration and
			// the specific iteration count
			// if region_it gets to null ( end of list )
			// then the stage was complete
			return {target_it - it, region_it == nullptr};
		}

		ref_cache::ref_cache(mapping const &memory_map, sgc::gc::metrics &metrics) :
			_m_map{memory_map},
			_metrics{metrics} {}

		ref_cache::~ref_cache() {
			// should only have active regions on failure or tests
			// but at least we release the reserved memory back
			// to the system
			while (auto *region = _active_regions.pop_front())
				system::release(region, _m_map.ref_region_size);
		}

		reference *ref_cache::reserve() {

			// if has no other active region get more
			if (_active_regions.empty())
				grow_available();

			auto * region = _active_regions.head;
			auto * ref = region->available_refs.pop_front ();

			// house keep regions
			if (region->available_refs.empty()) {
				// if no further references are available remove
				// region from active and move to full
				_active_regions.pop_front();
				_full_regions.push_front (region);
			}

			// count used reference
			++region->active_count;

			// mark ref as enabled
			ref->active = true;

			return ref;
		}

		void ref_cache::release(reference *ref) {
			// get owning region
			auto * region = _m_map.get_ref_region (ref);

			// restore region to available if
			// this is the "first" available
			if (region->active_count == _m_map.ref_count_per_region) {
				_full_regions.remove (region);
				_active_regions.push_front (region);
			}

			// decrease used reference counter
			--region->active_count;
			region->available_refs.push_front(ref);
		}

		void ref_cache::grow_available() {
			// allocate reference block
			auto *region = utils::as_ptr<ref_region>(
				system::reserve(_m_map.ref_region_size, _m_map.ref_region_size));

			system::commit(region, _m_map.ref_region_size);

			// reset counter
			region->active_count = 0;

			// format empty reference link nodes
			auto *ref_vector = _m_map.get_ref_block(region);
			auto count = _m_map.ref_count_per_region;

			for (std::size_t i = 1; i < count; ++i)
				ref_vector[i - 1].next = ref_vector + i;

			ref_vector[count - 1].next = nullptr;

			// set formated list
			region->available_refs.head = ref_vector;
			_active_regions.push_front(region);

			// update metrics
			_metrics.used_virtual += _m_map.ref_region_size;
			_metrics.used_physical += _m_map.ref_region_size;
			_metrics.used_system += _m_map.ref_region_size;
		}
	}
}