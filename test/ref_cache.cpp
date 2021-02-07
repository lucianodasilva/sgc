#include <catch2/catch.hpp>
#include <sgc.h>
#include "test_tools.h"

namespace sgc {
	namespace test {

		SCENARIO("reserve and release references", "[REF_CACHE]") {

			std::size_t const input_block_size	{ 4096 };
			std::size_t const input_unit_size	{ 64 };

			gc::mapping const m_map {
				input_block_size, 
				input_unit_size 
			};

			GIVEN ("empty memory system") {
				gc::metrics metrics;
				gc::ref_cache cache { m_map, metrics };

				WHEN ("reserving reference") {
					auto * reference = cache.reserve ();

					THEN("a reference must be reserved and property placed") {
						
						// should be placed on the first region
						REQUIRE(m_map.get_ref_region(reference) == cache.get_active_regions ().head);

						REQUIRE(size_of(cache.get_active_regions()) == 1);
						REQUIRE(size_of(cache.get_active_regions().head->available_refs) == m_map.ref_count_per_region - 1);

						REQUIRE(cache.get_active_regions().head->active_count == 1);
					}
				}
			}

			GIVEN("one reference reserved system") {
				gc::metrics metrics;
				gc::ref_cache cache{ m_map, metrics };
				auto* reference = cache.reserve ();

				WHEN("reserving an additional reference") {
					auto* new_reference = cache.reserve();

					THEN("a reference must be reserved and property placed") {

						// should be placed on the first region
						REQUIRE(m_map.get_ref_region(new_reference) == cache.get_active_regions().head);

						REQUIRE(size_of(cache.get_active_regions()) == 1);
						REQUIRE(size_of(cache.get_active_regions().head->available_refs) == m_map.ref_count_per_region - 2);

						REQUIRE(cache.get_active_regions().head->active_count == 2);
					}
				}

				WHEN("releasing a reference") {
					cache.release(reference);

					THEN("a reference must be reserved and property placed") {

						// should be placed on the first region
						REQUIRE(size_of(cache.get_active_regions()) == 1);
						REQUIRE(size_of(cache.get_active_regions().head->available_refs) == m_map.ref_count_per_region);

						REQUIRE(cache.get_active_regions().head->active_count == 0);
					}
				}
			}
		}

		SCENARIO("ref cache collection", "[REF_CACHE_COLLECTION_STAGE]") {
			
			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 64 };
			
			GIVEN("system with all released units") {
				gc::service & service = service_controller::restart_service (input_block_size, input_unit_size);

				auto& cache = service_controller::get_ref_cache();

				// create and release reference
				auto * reference = cache.reserve();
				cache.release (reference);

				gc::ref_cache_collection_stage stage;

				WHEN("collecting the unused regions") {
					stage.collect(service, std::numeric_limits < std::size_t >::max());

					THEN("active regions should be released") {
						REQUIRE(size_of(cache.get_active_regions()) == 0);

						// all available units should be removed
						REQUIRE(cache.get_active_regions().empty());
					}
				}
			}
		}

	}
}