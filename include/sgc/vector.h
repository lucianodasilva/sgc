#pragma once
#ifndef SGC_VECTOR_H
#define SGC_VECTOR_H

#include "allocator.h"
#include <vector>

namespace sgc {

	template < typename _t >
	using vector = std::vector < _t, sgc::allocator < _t > >;

}

#endif //SGC_VECTOR_H
