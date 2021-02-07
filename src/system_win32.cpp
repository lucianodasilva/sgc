#ifdef _WIN32

#include "sgc/system.h"
#include "sgc/gc/utils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace sgc {
	namespace system {

		std::size_t const page_size{ [] {
			SYSTEM_INFO info;
			GetSystemInfo(&info);
			return info.dwPageSize;
		}() };

		void * reserve(std::size_t size, std::size_t alignment) {
			if (alignment == 0)
				alignment = page_size;

			auto aligned_size =
				size + (alignment - page_size);

			uint8_t * address{ nullptr };
			uint8_t retry{ 255 };

			do {
				address = reinterpret_cast <uint8_t*> (
					// if too many collisions are detected, enabling mem_top_down should reduce collision count
					::VirtualAlloc(nullptr, aligned_size, MEM_RESERVE /*| MEM_TOP_DOWN*/, PAGE_READWRITE));
				
				if (!address)
					return nullptr;

				uint8_t* aligned_address = reinterpret_cast <uint8_t*>(
					(reinterpret_cast <uintptr_t> (address) + (alignment - 1)) & ~(alignment - 1));

				if (aligned_address != address) {
					::VirtualFree(address, 0, MEM_RELEASE);
					address = reinterpret_cast <uint8_t*> (
						::VirtualAlloc(aligned_address, size, MEM_RESERVE, PAGE_READWRITE));
				}
				else {
					address = aligned_address;
				}

				--retry;
			} while (address == nullptr && retry > 0);

			return address;
			//TODO: Throw on bad address
		}

		bool release(void * address, std::size_t) {
			return ::VirtualFree(address, 0, MEM_RELEASE) == TRUE;
		}

		bool commit(void * address, std::size_t byte_length) {
			return ::VirtualAlloc(address, byte_length, MEM_COMMIT, PAGE_READWRITE)
				== address;
		}

		bool decommit(void * address, std::size_t) {

			return ::VirtualFree(address, 0, MEM_DECOMMIT) == TRUE;
		}

	}
}

#endif