#pragma once
#ifndef SGC_ALLOCATOR_H
#define SGC_ALLOCATOR_H

#include "sgc/gc/service.h"
#include "gc_ptr.h"

namespace sgc {

	template < typename _t >
	struct allocator : public std::allocator < _t > {
	public:

		allocator() noexcept {
			root = sgc::gc::service::get().root();
		};

		template<typename _u>
		explicit allocator(sgc::allocator<_u> const & other) noexcept {
			root = other.root;
		}

		template < typename _u >
		struct rebind {
			using other = sgc::allocator < _u >;
		};

		sgc::gc::object_header *root;
	};

}

namespace std {


	template< typename _t >
	struct allocator_traits < sgc::allocator < _t > > {

		using allocator_type = sgc::allocator < _t >;
		using value_type = typename allocator_type::value_type;

		using pointer = typename allocator_type::pointer;
		using const_pointer = typename allocator_type::const_pointer;

		using void_pointer = void *;
		using const_void_pointer = void const *;

		using difference_type = typename allocator_type::difference_type;
		using size_type = typename allocator_type::size_type;

		template<typename _u>
		using rebind_alloc = typename allocator_type::template rebind<_u>::other;

		template<typename _u>
		using rebind_traits = std::allocator_traits<rebind_alloc<_u> >;

		static pointer allocate(allocator_type &alloc, size_type n) {
			return alloc.allocate(n);
		}

		static pointer allocate(allocator_type &alloc, size_type n, const_void_pointer hint) {
			return allocate(alloc, n);
		}

		static void deallocate(allocator_type &alloc, pointer p, size_type n) {
			alloc.deallocate(p, n);
		}

		template<typename _u, typename ... _args_tv>
		static void construct(allocator_type &alloc, _u *p, _args_tv &&... args) {
			auto &service = sgc::gc::service::get();

			auto *stacked = service.root();
			service.set_root (alloc.root);

			::new(reinterpret_cast < void * > (p)) _u(std::forward<_args_tv>(args)...);

			service.set_root (stacked);
		}

		template<typename _u>
		static void destroy(allocator_type &alloc, _u *p) {
			p->~_u();
		}

		static size_type max_size(const allocator_type &) noexcept {
			return std::numeric_limits<size_type>::max();
		}

		static allocator_type select_on_container_copy_construction(const allocator_type &a) {
			return a;
		}

	};

}

#endif //SGC_ALLOCATOR_H
