#include <catch2/catch.hpp>
#include <sgc.h>
#include "test_tools.h"

namespace sgc {
	namespace test {

		SCENARIO("reserve and release units", "[OBJECT_CACHE]") {

			std::size_t const input_block_size	{ 4096 };
			std::size_t const input_unit_size	{ 64 };
			std::size_t const small_unit_size	{ 32 };

			gc::mapping const m_map {
				input_block_size, 
				input_unit_size 
			};

			GIVEN ("empty memory system") {
				gc::metrics metrics;
				gc::object_cache cache { m_map, metrics };

				WHEN ("reserving unit of minimum size") {
					auto * unit = cache.reserve_unit (small_unit_size);

					THEN("a unit must be reserved and property placed") {
						
						// should be placed on the first region
						REQUIRE(m_map.get_unit_region (unit) == cache.get_active_regions ().head);

						// should be placed at the first place of the first block
						REQUIRE(m_map.get_unit_block_index(unit) == 1); // block zero reserved
						REQUIRE(m_map.get_unit_index(unit) == 0);
					}

					THEN("the free unit cache levels should split and top level consumed") {
						REQUIRE (size_of (cache.get_active_regions()) == 1);

						// one unit division per level causes "free" nodes to be 
						// generated per level
						REQUIRE (size_of (cache.get_free_units() [0]) == 1);
						REQUIRE (size_of (cache.get_free_units() [1]) == 1);
						REQUIRE (size_of (cache.get_free_units() [2]) == 1);
						REQUIRE (size_of (cache.get_free_units() [3]) == 1);
						REQUIRE (size_of (cache.get_free_units() [4]) == 1);
						REQUIRE (size_of (cache.get_free_units() [5]) == 1);

						// the remaining 62 free nodes out of the original 63
						// in the top level
						REQUIRE (size_of (cache.get_free_units() [6]) == 62);
					}
				}
			}

			GIVEN("one unit reserved system") {
				gc::metrics metrics;
				gc::object_cache cache{ m_map, metrics };

				auto* unit = cache.reserve_unit(small_unit_size);

				WHEN("reserving an additional unit of minimum size") {
					auto* new_unit = cache.reserve_unit(small_unit_size);

					THEN("a unit must be reserved and property placed") {

						// should be placed on the first region
						REQUIRE(m_map.get_unit_region(new_unit) == cache.get_active_regions().head);

						// should be placed at the second place of the first block
						REQUIRE(m_map.get_unit_block_index(new_unit) == 1); // block zero reserved
						REQUIRE(m_map.get_unit_index(new_unit) == 1);
					}

					THEN("only the zero level free node should be consumed") {
						REQUIRE(size_of(cache.get_active_regions()) == 1);

						REQUIRE(size_of(cache.get_free_units()[0]) == 0);

						REQUIRE(size_of(cache.get_free_units()[1]) == 1);
						REQUIRE(size_of(cache.get_free_units()[2]) == 1);
						REQUIRE(size_of(cache.get_free_units()[3]) == 1);
						REQUIRE(size_of(cache.get_free_units()[4]) == 1);
						REQUIRE(size_of(cache.get_free_units()[5]) == 1);

						// the remaining 62 free nodes out of the original 63
						// in the top level
						REQUIRE(size_of(cache.get_free_units()[6]) == 62);
					}
				}

				WHEN("releasing the reserved unit") {
					cache.release_unit(unit);

					THEN("all free nodes bellow the top level should have been merged") {
						REQUIRE(size_of(cache.get_active_regions()) == 1);

						// nodes should have been merged
						REQUIRE(size_of(cache.get_free_units()[0]) == 0);
						REQUIRE(size_of(cache.get_free_units()[1]) == 0);
						REQUIRE(size_of(cache.get_free_units()[2]) == 0);
						REQUIRE(size_of(cache.get_free_units()[3]) == 0);
						REQUIRE(size_of(cache.get_free_units()[4]) == 0);
						REQUIRE(size_of(cache.get_free_units()[5]) == 0);

						// the original 63 nodes should be restored
						REQUIRE(size_of(cache.get_free_units()[6]) == 63);
					}
				}
			}
		}

		SCENARIO("object cache collection", "[OBJECT_CACHE_COLLECTION_STAGE]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 64 };
			std::size_t const small_unit_size{ 32 };

			GIVEN("system with all released units") {
				sgc::gc::service & service = service_controller::restart_service (input_block_size, input_unit_size);

				auto& cache = service_controller::get_object_cache();

				// create and release unit
				auto * unit = cache.reserve_unit(small_unit_size);
				cache.release_unit(unit);

				gc::object_cache_collection_stage stage;

				WHEN("collecting the unused nodes and regions") {
					stage.collect(service, std::numeric_limits < std::size_t >::max());

					THEN("active regions should be released") {
						REQUIRE(size_of(cache.get_active_regions()) == 0);

						// all available units should be removed
						REQUIRE(size_of(cache.get_free_units()[6]) == 0);
					}
				}
			}
		}

	}
}