#pragma once
#ifndef SGC_FORWARD_LIST_H
#define SGC_FORWARD_LIST_H

#include "allocator.h"
#include <forward_list>

namespace sgc {

	template < typename _t >
	using forward_list = std::forward_list < _t, sgc::allocator < _t > >;

}

#endif //SGC_FORWARD_LIST_H
