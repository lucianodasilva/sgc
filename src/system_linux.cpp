#ifdef __linux

#include <zconf.h>
#include <sys/mman.h>

#include "sgc/system.h"
#include "sgc/gc/utils.h"

namespace sgc {
	namespace system {

		std::size_t const page_size {
			static_cast < std::size_t > (sysconf(_SC_PAGESIZE))
		};

		void * reserve (std::size_t size, std::size_t alignment) {


			if (!alignment)
				alignment = page_size;

			std::size_t padded_size = size + (alignment - page_size);

			auto * address = reinterpret_cast < uint8_t * > (
				mmap (
					nullptr,
					padded_size,
					PROT_NONE, MAP_ANON | MAP_PRIVATE,
					-1,
					0));

			if (!address)
				return nullptr;

			auto * aligned_address = gc::utils::align_to_next (address, alignment);
			auto starting_pad = aligned_address - address;

			if (starting_pad != 0)
				munmap(address, starting_pad);

			auto ending_pad = padded_size - (starting_pad + size);

			if (ending_pad != 0)
				munmap (aligned_address + size, ending_pad);

			return aligned_address;
		}

		bool release (void * address, std::size_t size) {
			return munmap (address, size) == 0;
		}

		bool commit (void * address, std::size_t size) {
			return mprotect(address, size, PROT_WRITE | PROT_READ) == 0;
		}

		bool decommit(void* address, std::size_t size)
		{
			return mprotect(address, size, PROT_NONE) == 0;
		}

	}
}

#endif