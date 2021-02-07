#pragma once
#ifndef SGC_SGC_H
#define SGC_SGC_H

#include "sgc/gc/service.h"
#include "sgc/gc/utils.h"
#include "sgc/gc_ptr.h"
#include "sgc/allocator.h"
#include "sgc/system.h"
#include "sgc/units.h"

namespace sgc {

	inline void gc_step(double target_time_ms) {
		sgc::gc::service::get().run_collection(target_time_ms);
	}

	inline void gc_full_collect() {
		sgc::gc::service::get().run_full_collection();
	}

}

#endif //SGC_SGC_H
