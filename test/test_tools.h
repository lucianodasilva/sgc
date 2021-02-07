#pragma once
#ifndef SGC_TEST_TOOLS_H
#define SGC_TEST_TOOLS_H

#include "sgc.h"

namespace sgc {
	namespace test {

		struct service_controller {
		public:
			// super hammer shaaaaaaaaaaaaame
			static sgc::gc::service & restart_service (std::size_t block_size, uint8_t unit_size);

			static sgc::gc::ref_cache & get_ref_cache();
			static sgc::gc::object_cache & get_object_cache ();
		};

		template < typename _t, template <class> class _policy_t >
		inline std::size_t size_of (details::link_chain < _t, _policy_t > const & list) noexcept {
			std::size_t size {0};

			auto * cursor = list.head;
			while (cursor) {
				++size;
				cursor = cursor->next;
			}

			return size;
		}

	}
}

#endif //SGC_TEST_TOOLS_H
