#pragma once
#ifndef SGC_UNORDERED_MAP_H
#define SGC_UNORDERED_MAP_H

#include "allocator.h"
#include <unordered_map>

namespace sgc {

	template <
		typename _key_t,
		typename _t,
		typename _hash_t = std::hash < _key_t >,
		typename _pred_t = std::equal_to < _key_t > >
	using unordered_map = std::unordered_map < _key_t, _t, _hash_t, _pred_t, sgc::allocator < std::pair < const _key_t, _t > > >;

	template <
		typename _key_t,
		typename _t,
		typename _hash_t = std::hash < _key_t >,
		typename _pred_t = std::equal_to < _key_t > >
	using unordered_multimap = std::unordered_multimap < _key_t, _t, _hash_t, _pred_t, sgc::allocator < std::pair < const _key_t, _t > > >;
}

#endif //SGC_UNORDERED_MAP_H
