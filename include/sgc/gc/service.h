#pragma once
#ifndef SGC_SERVICE_H
#define SGC_SERVICE_H

#include <memory>
#include <mutex>

#include "executor.h"
#include "metrics.h"
#include "object_cache.h"
#include "ref_cache.h"
#include "sgc/details/spin_mutex.h"

namespace sgc {
	namespace test {
		struct service_controller;
	}

	template <typename>
	struct gc_ptr;

	template <typename>
	struct allocator;

	namespace gc {

		struct object_graph {
		public:

			inline bool empty () const {
				return _white.empty() && _gray.empty() && _black.empty();
			}

			inline object_header *pop_white() {
				return _white.pop_front();
			}

			inline object_header *pop_gray() {
				return _gray.pop_front();
			}

			inline void add_to_gray(object_header *unit) {
				unit->stage = unit_stage::gray;
				_gray.push_front(unit);
			}

			inline void move_to_gray(object_header *unit) {
				if (unit->stage == unit_stage::white)
					_white.remove(unit);
				else
					return;

				unit->stage = unit_stage::gray;
				_gray.push_front(unit);
			}

			inline void add_to_black(object_header *unit) {
				unit->stage = unit_stage::black;
				_black.push_front(unit);
			}

			void stage_cycle();

		private:
			details::link_chain<object_header, details::policies::dual>
				_white,
				_gray,
				_black;
		};

		class start_collection_stage : public collection_stage {
		public:
			stage_result collect(sgc::gc::service &service, std::size_t target_it) override;
		};

		class mark_collection_stage : public collection_stage {
		public:
			stage_result collect(sgc::gc::service &service, std::size_t target_it) override;
		};

		class unreachable_collection_stage : public collection_stage {
		public:
			stage_result collect(sgc::gc::service &service, std::size_t target_it) override;
		};

		struct service {
		public:

			// single instance accessor
			static service &init_with(std::size_t block_size, uint8_t unit_size);
			static service &get();

			template<typename _t, typename ... _args_tv>
			inline object_header *object_new(_args_tv &&... args) {

				// define destructor for object type
				destructor dctor = [](void *address) {
					reinterpret_cast <_t *> (address)->~_t();
				};

				// allocate object memory and define destructor
				object_header *address = allocate(sizeof(_t), dctor);

				// stack parent object
				object_header *stacked_parent = _parent;
				_parent = address;

				// call type constructor
				new(address->get_object()) _t(std::forward<_args_tv>(args)...);

				// restore parent
				_parent = stacked_parent;

				return address;
			}

			virtual ~service();

			// remove copy constructor and assignment operator;
			service(service const &) = delete;
			service &operator=(service const &) = delete;

			void run_collection(double target_time_ms);
			void run_full_collection();

			inline metrics const &get_metrics() const {
				return _metrics;
			}

			mapping const memory_mapping;

			friend class mark_collection_stage;
			friend class unreachable_collection_stage;
			friend class start_collection_stage;
			friend class object_cache_collection_stage;
			friend class ref_cache_collection_stage;

			template<typename>
			friend struct sgc::gc_ptr;

			template<typename>
			friend struct sgc::allocator;

			template<typename>
			friend struct std::allocator_traits;

			// for testing only
			friend struct sgc::test::service_controller;

		private:

			object_header *allocate(std::size_t size, destructor dctor = nullptr);

			reference *ref_new(object_header *from, object_header *to);

			void ref_release(reference *ref);

			inline object_header *root() {
				if (BRANCH_UNLIKELY(!_parent)) _parent = &_root;
				return _parent;
			}

			inline void set_root(object_header *root) {
				if (BRANCH_UNLIKELY(!_parent)) _parent = &_root;
				_parent = root;
			}

			service();

			struct service_init_settings {
				std::size_t block_size;
				uint8_t unit_size;
			};

			static service_init_settings init_settings;

			metrics _metrics{0};

			executor<
				start_collection_stage,
				mark_collection_stage,
				unreachable_collection_stage,
				object_cache_collection_stage,
				ref_cache_collection_stage
			>
				_stages;

			object_graph _object_graph;
			object_cache _object_cache;
			ref_cache _ref_cache;

			// stacking thread_local helper
			static thread_local object_header *
				_parent;

			object_header _root;
			details::spin_mutex _service_mutex;
		};
	}
}

#endif // !SGC_SERVICE_H
