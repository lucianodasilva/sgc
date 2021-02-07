#pragma once
#ifndef SGC_LINK_CHAIN_H
#define SGC_LINK_CHAIN_H

#include <utility>

namespace sgc {
	namespace details {

		template<typename _t, template<typename> class node_t>
		struct is_node_t : public std::false_type {
		};

		template<typename _item_t, template<typename> class node_t>
		struct is_node_t<node_t<_item_t>, node_t> : public std::true_type {
		};

		template<typename _t>
		struct link_t {
			_t *next;
		};

		template<typename _t>
		struct dual_link_t {
			_t *prev;
			_t *next;
		};

		template<typename _t>
		constexpr bool is_single_linked{is_node_t<_t, link_t>::value};

		template<typename _t>
		constexpr bool is_dual_linked{is_node_t<_t, dual_link_t>::value};

		template<typename _link_t>
		struct iterator {

			inline iterator &operator++() {
				link = link->next;
				return *this;
			}

			inline bool operator!=(iterator const &it) const {
				return link != it.link;
			}

			inline _link_t &operator*() {
				return *link;
			}

			_link_t *link;
		};

		namespace policies {

			template<typename _link_t>
			struct single {
			protected:

				inline void push_set_tail(_link_t *link) noexcept {}

				inline void clear_tail() noexcept {}

				inline void remove_sync(_link_t *link) noexcept {
					if (head == link)
						head = link->next;
				}

				inline void remove_cursor_update(_link_t *prev, _link_t *cursor) noexcept {
					(prev ? prev->next : head) = cursor->next;
				}

				inline void pop_sync_tail() {}

				inline void swap(single &o) {
					std::swap(head, o.head);
				}

			public:
				_link_t * head { nullptr };
			};

			template<typename _link_t>
			struct dual {
			protected:

				inline void push_set_tail(_link_t *link) noexcept {
					link->prev = nullptr;

					if (head)
						head->prev = link;

					if (!tail)
						tail = link;
				}

				inline void clear_tail() noexcept {
					tail = nullptr;
				}

				inline void remove_sync(_link_t *link) noexcept {
					if (head == link)
						head = link->next;

					if (tail == link)
						tail = link->prev;
				}

				inline void remove_cursor_update(_link_t *, _link_t *cursor) {
					(cursor->prev ? cursor->prev->next : head) = cursor->next;
					(cursor->next ? cursor->next->prev : tail) = cursor->prev;
				}

				inline void pop_sync_tail() {
					if (!head)
						tail = nullptr;
				}

				inline void swap(dual &o) {
					std::swap(head, o.head);
					std::swap(tail, o.tail);
				}

			public:

				inline void push_back(_link_t *link) noexcept {
					link->prev = tail;
					link->next = nullptr;

					if (tail)
						tail->next = link;

					if (!head)
						head = link;

					tail = link;
				}

				inline _link_t *pop_back() noexcept {
					if (!tail)
						return nullptr;

					auto *link = tail;
					tail = link->prev;

					if (!tail)
						head = nullptr;

					return link;
				}

				_link_t
					*head { nullptr },
					*tail { nullptr };
			};

		}

		template<typename _link_t, template<typename> class _policy_t = policies::single>
		struct link_chain : public _policy_t<_link_t> {
		public:

			inline bool empty() const noexcept {
				return this->head == nullptr;
			}

			inline void push_front(_link_t *link) noexcept {
				link->next = this->head;

				// handle terminators
				this->push_set_tail(link);

				this->head = link;
			}

			template<typename _release_t>
			inline void clear(_release_t release_func) noexcept {
				while (this->head) {
					auto *link = this->head;
					this->head = link->next;
					release_func(link);
				}

				this->clear_tail();
			}

			inline void remove(
				_link_t *link
			) noexcept {
				static_assert(!is_dual_linked<_link_t>, "link chain does not support item removal");

				this->remove_sync(link);

				if (link->next)
					link->next->prev = link->prev;

				if (link->prev)
					link->prev->next = link->next;
			}

			template<typename _predicate_t, typename _release_t>
			inline void remove_if(_predicate_t predicate_func, _release_t release_func) {
				_link_t *prev = nullptr;
				_link_t *cursor = this->head;

				while (cursor) {

					if (predicate_func(cursor)) {

						this->remove_cursor_update(prev, cursor);

						_link_t *to_erase = cursor;
						cursor = cursor->next;

						release_func(to_erase);
					} else {
						prev = cursor;
						cursor = cursor->next;
					}
				}
			}

			inline _link_t *pop_front() noexcept {
				if (!this->head)
					return nullptr;

				auto *link = this->head;
				this->head = link->next;

				this->pop_sync_tail();

				return link;
			}

			iterator<_link_t> begin() { return {this->head}; }

			iterator<_link_t> end() const { return {nullptr}; }

			inline void swap(link_chain &chain) {
				_policy_t < _link_t >::swap(chain);
			}

		};
	}
}

#endif
