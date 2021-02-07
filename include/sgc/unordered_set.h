#pragma once
#ifndef SGC_UNORDERED_SET_H
#define SGC_UNORDERED_SET_H

#include "allocator.h"
#include <unordered_set>

namespace sgc {

	template <
		typename _t,
		typename _hash_t = std::hash < _t >,
		typename _pred_t = std::equal_to < _t > >
	using unordered_set = std::unordered_set < _t, _hash_t, _pred_t, sgc::allocator < _t > >;

	template <
		typename _t,
		typename _hash_t = std::hash < _t >,
		typename _pred_t = std::equal_to < _t > >
	using unordered_multiset = std::unordered_multiset < _t, _hash_t, _pred_t, sgc::allocator < _t > >;

}

#endif //SGC_UNORDERED_SET_H
