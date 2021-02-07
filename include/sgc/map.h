#pragma once
#ifndef SGC_MAP_H
#define SGC_MAP_H

#include "allocator.h"
#include <map>

namespace sgc {

	template < typename _key_t, typename _t, typename _compare_t = std::less < _key_t > >
	using map = std::map < _key_t, _t, _compare_t, sgc::allocator < std::pair < const _key_t, _t > > >;

	template < typename _key_t, typename _t, typename _compare_t = std::less < _key_t > >
	using multimap = std::multimap < _key_t, _t, _compare_t, sgc::allocator < std::pair < const _key_t, _t > > >;

}

#endif //SGC_MAP_H
