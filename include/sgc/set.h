#pragma once
#ifndef SGC_SET_H
#define SGC_SET_H

#include "allocator.h"
#include <set>

namespace sgc {

	template < typename _t, typename _compare_t = std::less < _t > >
	using set = std::set < _t, _compare_t, sgc::allocator < _t > >;

	template < typename _t, typename _compare_t = std::less < _t > >
	using multiset = std::multiset < _t, _compare_t, sgc::allocator < _t > >;

}

#endif //SGC_SET_H
