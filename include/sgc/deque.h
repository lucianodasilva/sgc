#pragma once
#ifndef SGC_DEQUE_H
#define SGC_DEQUE_H

#include "allocator.h"
#include <deque>

namespace sgc {

	template < typename _t >
	using deque = std::deque < _t, sgc::allocator < _t > >;

}

#endif //SGC_DEQUE_H
