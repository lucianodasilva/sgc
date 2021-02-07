#pragma once
#ifndef SGC_REF_CACHE_H
#define SGC_REF_CACHE_H

#include "executor.h"
#include "metrics.h"
#include "model.h"

namespace sgc {
	namespace gc {
		class ref_cache_collection_stage : public collection_stage {
		public:
			stage_result collect(sgc::gc::service &service, std::size_t target_it) override;

		private:
			ref_region
				*region_it{nullptr},
				*prev_region_it{nullptr};
		};

		struct ref_cache {
		public:

			ref_cache(mapping const &memory_map, sgc::gc::metrics &metrics);

			~ref_cache();

			inline details::link_chain <ref_region> const &get_full_regions() const noexcept {
				return _full_regions;
			}

			inline details::link_chain <ref_region> const &get_active_regions() const noexcept {
				return _active_regions;
			}

			reference *reserve();

			void release(reference *ref);

			friend class ref_cache_collection_stage;

		private:

			void grow_available();

			mapping const &_m_map;
			metrics &_metrics;

			details::link_chain <ref_region> _full_regions;
			details::link_chain <ref_region> _active_regions;
		};
	}
}

#endif //SGC_REF_CACHE_H