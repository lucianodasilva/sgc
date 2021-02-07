#pragma once
#ifndef SGC_GC_PTR_H
#define SGC_GC_PTR_H

#include "sgc/gc/model.h"
#include "sgc/gc/service.h"

namespace sgc {

	template < typename _t >
	struct gc_ptr {
	public:

		// -- constructors --
		constexpr gc_ptr() = default;

		inline constexpr explicit gc_ptr(std::nullptr_t) : gc_ptr() {}

		inline gc_ptr(const gc_ptr & x) noexcept {
			copy(x);
		}

		template <typename _u>
		inline explicit gc_ptr(const gc_ptr<_u> & x) noexcept {
			copy(x);
		}

		inline gc_ptr(gc_ptr && x) noexcept {
			swap(x);
		}

		template <typename _u>
		inline explicit gc_ptr(gc_ptr<_u> && x) noexcept {
			swap(x);
		}

		// -- destructor --
		~gc_ptr () {
			if (_ref)
				sgc::gc::service::get().ref_release(_ref);
		}

		// -- operators --
		inline gc_ptr & operator= (const gc_ptr & x) noexcept {
			copy(x);
			return *this;
		}

		template <typename _u>
		inline gc_ptr& operator= (const gc_ptr<_u> & x) noexcept {
			copy(x);
			return *this;
		}

		inline gc_ptr& operator= (gc_ptr && x) noexcept {
			swap(x);
			return *this;
		}

		template <typename _u>
		inline gc_ptr& operator= (gc_ptr<_u> && x) noexcept {
			swap(x);
			return *this;
		}

		inline _t & operator*() const noexcept {
			return *operator->();
		}

		inline _t * operator->() const noexcept {
			if (_ref)
				return gc::utils::as_ptr < _t > (_ref->to->get_object());
			else
				return nullptr;
		}

		inline explicit operator bool() const noexcept {
			return _ref != nullptr;
		}

		// -- function --
		inline void swap(gc_ptr & x) noexcept {
			gc::object_header * obj = nullptr;
			gc::object_header * x_obj = nullptr;

			if (_ref) {
				obj = _ref->to;
				reset();
			}

			if (x._ref) {
				x_obj = x._ref->to;
				x.reset();
			}

			if (x_obj)
				_ref = sgc::gc::service::get().ref_new (_root, x_obj);

			if (obj)
				x._ref = sgc::gc::service::get().ref_new (x._root, obj);
		}

		inline void reset() noexcept {
			if (_ref) {
				sgc::gc::service::get().ref_release (_ref);
				_ref = nullptr;
			}
		}

		template<typename, typename ... _args_tv >
		friend auto make_gc (_args_tv &&...);

		template <typename>
		friend struct gc_ptr;

	private:

		template < typename _u >
		inline void copy(gc_ptr <_u> const & v) {
			reset();

			if (v._ref) {
				_ref = sgc::gc::service::get().ref_new (_root, v._ref->to);
			}
		}

		inline explicit gc_ptr(gc::object_header * unit) :
			_ref { sgc::gc::service::get().ref_new (_root, unit) }
		{}

		gc::object_header 	* _root { sgc::gc::service::get().root () };
		gc::reference 		* _ref  { nullptr };
	};

	template < typename _t, typename ... _args_tv >
	inline auto make_gc (_args_tv&& ... args) {
		return gc_ptr < _t > {
			sgc::gc::service::get().template object_new < _t >(
				std::forward < _args_tv >(args)...) };
	}

}

#endif //SGC_GC_PTR_H
