#include "sgc/gc/service.h"
#include "sgc/gc/object_cache.h"
#include "sgc/gc/utils.h"
#include "sgc/units.h"


namespace sgc {
	namespace gc {
		service::service_init_settings service::init_settings{
			units::mbytes(4),
			64
		};

		service &service::init_with(std::size_t block_size, uint8_t unit_size) {
			init_settings.block_size = block_size;
			init_settings.unit_size = unit_size;

			return get();
		}

		service &service::get() {
			static service instance;
			return instance;
		}

		void object_graph::stage_cycle() {
			// TODO: assert _white and _gray should be empty. Perhaps should throw an exception
			if (!_white.empty() && !_gray.empty())
				return;

			_white.swap(_black);

			for (auto &obj : _white)
				obj.stage = unit_stage::white;
		}

		stage_result start_collection_stage::collect(sgc::gc::service &service, std::size_t target_it) {
			// this should be fast enough to not worry about
			// iterations

			// make all past cycle black object white
			service._object_graph.stage_cycle();

			// make all root references gray
			// TODO: remove copy pasted solution
			service._root.references.remove_if(
				[&service](reference *ref) {
					if (ref->active) {
						service._object_graph.move_to_gray(ref->to);
						return false;
					} else {
						return true;
					}
				},
				[&service](reference *ref) {
					service._ref_cache.release(ref);
				}
			);

			// single iteration and always complete
			return {1, true};
		}

		stage_result mark_collection_stage::collect(sgc::gc::service &service, std::size_t target_it) {
			auto &m_map = service.memory_mapping;

			std::size_t it = target_it;

			// run through all gray objects
			object_header *unit{nullptr};
			while (it > 0) {
				unit = service._object_graph.pop_gray();

				if (!unit)
					break;

				// promote referenced units to gray
				// remove unused references
				unit->references.remove_if(
					[&service](reference *ref) {
						if (ref->active) {
							service._object_graph.move_to_gray(ref->to);
							return false;
						} else {
							return true;
						}
					},
					[&service](reference *ref) {
						service._ref_cache.release(ref);
					}
				);

				// promote current unit to black
				service._object_graph.add_to_black(unit);

				--it;
			}

			// return executed iterations
			// if no more gray objects are available
			// stage is complete
			return {target_it - it, unit == nullptr};
		}

		stage_result unreachable_collection_stage::collect(sgc::gc::service &service, std::size_t target_it) {
			auto &m_map = service.memory_mapping;

			auto &object_cache = service._object_cache;
			auto &ref_cache = service._ref_cache;

			// at this stage all white objects
			// are unreachable
			std::size_t it = target_it;
			object_header *unit{nullptr};

			while (it > 0) {
				unit = service._object_graph.pop_white();

				if (!unit)
					break;

				// if available call object destructor
				if (unit->dctor)
					unit->dctor(unit->get_object());

				// release references
				unit->references.clear([&](reference *ref) {
					ref_cache.release(ref);
				});

				// release from data storage
				object_cache.release_unit(unit);

				// update metrics
				--service._metrics.objects;

				// count down iterations
				--it;
			}

			// return executed iterations
			// if no more white objects are available
			// stage is complete
			return {target_it - it, unit == nullptr};
		}

		thread_local object_header *service::_parent{nullptr};

		service::service() :
			memory_mapping{init_settings.block_size, init_settings.unit_size},
			_object_cache{memory_mapping, _metrics},
			_ref_cache{memory_mapping, _metrics} {}

		object_header *service::allocate(std::size_t size, destructor dctor) {
			details::spin_guard lock {_service_mutex};

			// scale size to include unit info
			size += sizeof(object_header);

			// get memory unit
			auto *unit = _object_cache.reserve_unit(size);
			unit->dctor = dctor;

			// add to current gray
			_object_graph.add_to_gray(unit);

			// update metrics
			++_metrics.objects;

			return unit;
		}

		reference *service::ref_new(object_header *from, object_header *to) {
			details::spin_guard lock {_service_mutex};

			auto *ref = _ref_cache.reserve();

			from->references.push_front(ref);
			ref->to = to;

			// handle possible hidden references
			_object_graph.move_to_gray(to);

			// update metrics
			++_metrics.references;

			return ref;
		}

		void service::ref_release(reference *ref) {
			ref->active = false;

			// update metrics
			--_metrics.references;
		}

		void service::run_collection(double target_time_ms) {
			details::spin_guard lock {_service_mutex};
			_stages.run(*this, target_time_ms);
		}

		void service::run_full_collection() {
			details::spin_guard lock {_service_mutex};
			_stages.run_full_cycle(*this);
		}

		service::~service() {
			// clear all root references
			_root.references.clear([&](reference * ref) {
				_ref_cache.release(ref);
			});

			// while objects still exist run full collection
			while (!_object_graph.empty())
				run_full_collection();
		}
	}
}