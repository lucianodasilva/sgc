cmake_minimum_required(VERSION 3.10)

project (sgc LANGUAGES CXX)

option (SGC_UNIT_TESTS "enable compilation of unit tests" OFF)
option (SGC_BENCHMARKS "enable compilation of benchmarks" OFF)

set (CXX_STANDARD 14)

include(FetchContent)

add_library (sgc STATIC
        include/sgc.h
		include/sgc/details/bitset.h
		include/sgc/details/link_chain.h
		include/sgc/details/spin_mutex.h
		include/sgc/gc/executor.h
		include/sgc/gc/metrics.h
		include/sgc/gc/model.h
		include/sgc/gc/object_cache.h
		include/sgc/gc/ref_cache.h
		include/sgc/gc/service.h
		include/sgc/gc/utils.h
		include/sgc/allocator.h
		include/sgc/deque.h
		include/sgc/forward_list.h
		include/sgc/gc_ptr.h
		include/sgc/list.h
		include/sgc/map.h
		include/sgc/set.h
        include/sgc/system.h
		include/sgc/units.h
		include/sgc/unordered_map.h
		include/sgc/unordered_set.h
		include/sgc/vector.h
		src/gc/executor.cpp
		src/gc/object_cache.cpp
		src/gc/ref_cache.cpp
		src/gc/service.cpp
		src/sgc.cpp
        src/system_linux.cpp
        src/system_win32.cpp)

if (MSVC)
	# teach intellisense to deal gc pointers
	target_sources(sgc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/extras/vs/sgc.natvis)
endif()

set_target_properties(
		sgc
		PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON)

target_include_directories(sgc PUBLIC
        include/)

# unit testing
if (SGC_UNIT_TESTS)

	FetchContent_Declare(
		catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG v2.13.4
		GIT_SHALLOW TRUE)

	FetchContent_MakeAvailable (
		catch2)
	
	add_executable (
			sgc_unit_test
			test/test_tools.h
			test/test_tools.cpp
			test/main.cpp
			test/memory_model.cpp
			test/object_cache.cpp
			test/ref_cache.cpp)

	set_target_properties(
			sgc_unit_test
			PROPERTIES
			CXX_STANDARD 14
			CXX_STANDARD_REQUIRED ON
			RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	target_link_libraries (sgc_unit_test PRIVATE sgc Catch2::Catch2)

	add_executable (
			sgc_demo
			test/demo.cpp)

	target_link_libraries (sgc_demo PUBLIC sgc)
endif()

# unit testing
if (SGC_BENCHMARKS)

	set (BENCHMARK_ENABLE_TESTING CACHE BOOL OFF FORCE)
	set (BENCHMARK_ENABLE_GTEST_TESTS CACHE BOOL OFF FORCE)

	FetchContent_Declare(
		googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark
		GIT_TAG v1.5.2
		GIT_SHALLOW TRUE)

	FetchContent_MakeAvailable(googlebenchmark)

	add_executable (
			sgc_benchmark
			benchmark/main.cpp)

	set_target_properties(
			sgc_benchmark
			PROPERTIES
			CXX_STANDARD 14
			CXX_STANDARD_REQUIRED ON
			RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benchmark)

	target_link_libraries (sgc_benchmark PRIVATE sgc benchmark::benchmark)

	#	-- Flags for profiling --
	#
	#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	#	target_compile_options(sgc PUBLIC
	#			-fno-omit-frame-pointer
	#			-g)

	#	target_compile_options(sgc_benchmark PUBLIC
	#			-fno-omit-frame-pointer
	#			-g)
	#endif()

	#if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	#	target_compile_options(sgc PUBLIC
	#		/Z7 )

	#	set_target_properties (sgc PROPERTIES LINK_FLAGS "/PROFILE")

	#	target_compile_options(sgc_benchmark PUBLIC
	#		/Z7 )

	#	set_target_properties (sgc_benchmark PROPERTIES LINK_FLAGS "/PROFILE")
	#endif()

endif()