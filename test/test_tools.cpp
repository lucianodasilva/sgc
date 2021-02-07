#include "test_tools.h"

namespace sgc {
	namespace test {

		// super hammer shaaaaaaaaaaaaame
		sgc::gc::service & service_controller::restart_service (std::size_t block_size, uint8_t unit_size) {
			auto * instance = &sgc::gc::service::get();

			// force calling destructor
			instance->~service();

			// reset initialization settings
			gc::service::init_settings.block_size = block_size;
			gc::service::init_settings.unit_size = unit_size;

			// reconstruct service inplace
			::new (reinterpret_cast < void * >(instance)) sgc::gc::service ();

			// return instance
			return sgc::gc::service::get();
		}

		sgc::gc::ref_cache & service_controller::get_ref_cache () {
			return gc::service::get()._ref_cache;
		}

		sgc::gc::object_cache & service_controller::get_object_cache () {
			return gc::service::get()._object_cache;
		}

	}
}