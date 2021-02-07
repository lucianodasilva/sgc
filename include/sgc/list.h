#pragma once
#ifndef SGC_LIST_H
#define SGC_LIST_H

#include "allocator.h"
#include <list>

namespace sgc {

	template < typename _t >
	using list = std::list < _t, sgc::allocator < _t > >;

}

#endif //SGC_LIST_H
