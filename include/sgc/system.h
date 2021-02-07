#pragma once
#ifndef SGC_SYSTEM_H
#define SGC_SYSTEM_H

#include <cstdlib>

// --- uniform system macros ---

// os
#if defined (__linux__) && !defined(__ANDROID__)
#	define SGC_OS_LINUX
#elif defined (__ANDROID__)
#	define SGC_OS_ANDROID
#elif defined (__APPLE__)
#	define SGC_OS_DARWIN
#elif defined (_WIN32)
#	define SGC_OS_WINDOWS
#endif

// compiler
#if defined (__GNUC__)
#	define SGC_COMPILER_GCC
#elif defined (_MSC_VER)
#	define SGC_COMPILER_MSVC
#elif defined (__clang__)
#	define SGC_COMPILER_CLANG
#endif

// build architecture
#if defined (__x86_64__) || defined (_M_X64)
#	define SGC_ARCH_X64
#elif defined (__i386) || defined (_M_IX86)
#	define SGC_ARCH_IX86
#elif defined (__arm__) || defined (_M_ARM)
#	define SGC_ARCH_ARM
#elif defined (__aarch64__) || defined (_M_ARM64)
#	define SGC_ARCH_ARM64
#endif

// -----------------------------

namespace sgc {
	namespace system {

		extern std::size_t const page_size;

		void * 	reserve (std::size_t size, std::size_t alignment);
		bool 	release (void * address, std::size_t size);
		bool 	commit 	(void * address, std::size_t size);
		bool 	decommit(void * address, std::size_t size);

	}
};

#endif //SGC_SYSTEM_H
