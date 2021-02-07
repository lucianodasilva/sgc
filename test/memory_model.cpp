#include <catch2/catch.hpp>
#include <sgc.h>

namespace sgc {
	namespace test {

		SCENARIO("memory mapping new instance", "[MODEL]") {

			std::size_t const input_block_size { 4096 };
			std::size_t const input_unit_size { 32 };

			GIVEN("power of 2 block size and unit size") {
				std::size_t const expected_block_size { input_block_size };
				std::size_t const expected_unit_size { input_unit_size };

				gc::mapping const m_map { input_block_size, input_unit_size };

				REQUIRE(m_map.unit_block_size == expected_block_size);
				REQUIRE(m_map.unit_size == expected_unit_size);
			}

			GIVEN("non pow2 block size and pow2 unit size") {

				gc::mapping const m_map { input_block_size + 1, input_unit_size };

				THEN("expect next pow2 block size") {
					std::size_t const expected_block_size { input_block_size << 1U };
					std::size_t const expected_unit_size { input_unit_size };

					REQUIRE(m_map.unit_block_size == expected_block_size);
					REQUIRE(m_map.unit_size == expected_unit_size);
				}
			}

			GIVEN("pow2 block size and non pow2 unit size") {

				gc::mapping const m_map { input_block_size, input_unit_size + 1};

				THEN("expect next pow2 block size") {
					std::size_t const expected_block_size { input_block_size };
					std::size_t const expected_unit_size { input_unit_size << 1U };

					REQUIRE(m_map.unit_block_size == expected_block_size);
					REQUIRE(m_map.unit_size == expected_unit_size);
				}
			}
		}

		SCENARIO("find region addresses from object", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_unit_region for unit address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1000, 0x0),
						std::make_pair(0x2000, 0x0),
						std::make_pair(0x20000, 0x20000),
						std::make_pair(0x21000, 0x20000),
						std::make_pair(0x22000, 0x20000),
						std::make_pair(0x3FFFF, 0x20000),
						std::make_pair(0x40000, 0x40000),
						std::make_pair(0x44000, 0x40000)
					);

					THEN("return region address") {
						auto* expected_region =
							gc::utils::as_ptr < gc::object_region >(address.second);

						auto* region = m_map.get_unit_region(
							gc::utils::as_ptr < gc::object_header >(address.first));

						REQUIRE(region == expected_region);
					}

				}

			}
		}

		SCENARIO("find block addresses from object", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_unit_block for unit address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1000, 0x1000),
						std::make_pair(0x2000, 0x2000),
						std::make_pair(0x20000, 0x20000),
						std::make_pair(0x21000, 0x21000),
						std::make_pair(0x22000, 0x22000),
						std::make_pair(0x3FFFF, 0x3F000),
						std::make_pair(0x40000, 0x40000),
						std::make_pair(0x44000, 0x44000)
					);

					THEN("return block address") {
						auto* expected_block =
							gc::utils::as_ptr < gc::object_block >(address.second);

						auto* region = m_map.get_unit_block(
							gc::utils::as_ptr < gc::object_header >(address.first));

						REQUIRE(region == expected_block);
					}

				}
			}
		}

		SCENARIO("find unit index from object", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_unit_index for unit address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1020, 1),
						std::make_pair(0x2040, 2),
						std::make_pair(0x20060, 3),
						std::make_pair(0x21080, 4),
						std::make_pair(0x220A0, 5),
						std::make_pair(0x3F0C0, 6),
						std::make_pair(0x40000, 0),
						std::make_pair(0x44000, 0)
					);

					THEN("return unit index") {
						std::size_t expected_index = (address.second);

						auto index = m_map.get_unit_index(
							gc::utils::as_ptr < gc::object_header >(address.first));

						REQUIRE(index == expected_index);
					}

				}
			}
		}

		SCENARIO("find unit at index", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_unit_at_index for block and index") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_tuple(0x1020, 1, 0x1000),
						std::make_tuple(0x2040, 2, 0x2000),
						std::make_tuple(0x20060, 3, 0x20000),
						std::make_tuple(0x21080, 4, 0x21000),
						std::make_tuple(0x220A0, 5, 0x22000),
						std::make_tuple(0x3F0C0, 6, 0x3F000),
						std::make_tuple(0x40000, 0, 0x40000)
					);

					THEN("return unit") {
						auto* expected_unit = gc::utils::as_ptr < gc::object_header >(
							std::get < 0 >(address));

						auto unit = m_map.get_unit_at_index(
							gc::utils::as_ptr < gc::object_block >(std::get < 2 >(address)),
							std::get < 1 >(address)
						);

						REQUIRE(unit == expected_unit);
					}

				}
			}
		}

		SCENARIO("find unit block index from object", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_unit_block_index for unit address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1020, 1),
						std::make_pair(0x2040, 2),
						std::make_pair(0x20060, 0),
						std::make_pair(0x21080, 1),
						std::make_pair(0x220A0, 2),
						std::make_pair(0x3F0C0, 31),
						std::make_pair(0x40000, 0),
						std::make_pair(0x44000, 4)
					);

					THEN("return unit block index") {
						std::size_t expected_block_index = (address.second);

						auto index = m_map.get_unit_block_index(
							gc::utils::as_ptr < gc::object_header >(address.first));

						REQUIRE(index == expected_block_index);
					}

				}
			}
		}

		SCENARIO("find unit block for index", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN ("get_unit_block_at_index for region and index") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_tuple (0x0, 1, 0x1000),
						std::make_tuple (0x0, 2, 0x2000),
						std::make_tuple (0x20000, 0, 0x20000),
						std::make_tuple (0x20000, 1, 0x21000),
						std::make_tuple (0x20000, 2, 0x22000),
						std::make_tuple (0x40000, 0, 0x40000)
					);

					THEN ("return unit") {
						auto * expected_block = gc::utils::as_ptr < gc::object_block > (
							std::get < 2 > (address));

						auto block = m_map.get_unit_block_at_index (
							gc::utils::as_ptr < gc::object_region > (std::get < 0 > (address)),
							std::get < 1 > (address)
						);

						REQUIRE (block == expected_block);
					}

				}

			}

		}

		SCENARIO("find unit map at index", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_unit_map_at_index for region and index") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_tuple(0x0,		1, 0x80),
						std::make_tuple(0x0,		2, 0x100),
						std::make_tuple(0x20000,	1, 0x20080),
						std::make_tuple(0x20000,	2, 0x20100)
					);

					THEN("return unit") {
						auto* expected_map = gc::utils::as_ptr < gc::map_entry >(
							std::get < 2 >(address));

						auto map = m_map.get_unit_map_at_index(
							gc::utils::as_ptr < gc::object_region >(std::get < 0 >(address)),
							std::get < 1 >(address)
						);

						REQUIRE(map == expected_map);
					}

				}

			}

		}

		SCENARIO("find map addresses from object", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_unit_map for unit address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1000, 0x80),
						std::make_pair(0x2000, 0x100),
						std::make_pair(0x21000, 0x20080),
						std::make_pair(0x22000, 0x20100),
						std::make_pair(0x3FFFF, 0x20F80),
						std::make_pair(0x41000, 0x40080),
						std::make_pair(0x44000, 0x40200)
					);

					THEN("return block address") {
						auto* expected_map =
							gc::utils::as_ptr < gc::map_entry >(address.second);

						auto* map = m_map.get_unit_map (
							gc::utils::as_ptr < gc::object_header >(address.first));

						REQUIRE(map == expected_map);
					}

				}
			}
		}

		SCENARIO("find map entry addresses from object", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_unit_map_entry for unit address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1060, 0x83),
						std::make_pair(0x21E0, 0x10F),
						std::make_pair(0x221E0, 0x2010F)
					);

					THEN("return block address") {
						auto* expected_map =
							gc::utils::as_ptr < gc::map_entry >(address.second);

						auto* map = m_map.get_unit_map_entry(
							gc::utils::as_ptr < gc::object_header >(address.first));

						REQUIRE(map == expected_map);
					}

				}
			}
		}

		SCENARIO("find offset from object", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_offset_from_region for unit address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1000, 0x1000),
						std::make_pair(0x2000, 0x2000),
						std::make_pair(0x20000, 0x0),
						std::make_pair(0x21000, 0x1000),
						std::make_pair(0x22000, 0x2000),
						std::make_pair(0x40000, 0x0),
						std::make_pair(0x44000, 0x4000)
					);

					THEN("return region address") {
						auto expected_offset = address.second;

						auto offset = m_map.get_offset_from_region(
							gc::utils::as_ptr < gc::object_header >(address.first));

						REQUIRE(offset == expected_offset);
					}

				}

			}
		}

		SCENARIO("find offset from block", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_offset_from_block for unit address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1000, 0x0),
						std::make_pair(0x2C80, 0xC80),
						std::make_pair(0x222C0, 0x2C0)
					);

					THEN("return region address") {
						auto expected_offset = address.second;

						auto offset = m_map.get_offset_from_block (
							gc::utils::as_ptr < gc::object_header >(address.first));

						REQUIRE(offset == expected_offset);
					}

				}

			}
		}

		SCENARIO("find ref region addresses from reference", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_ref_region for reference address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1000, 0x1000),
						std::make_pair(0x2000, 0x2000),
						std::make_pair(0x20000, 0x20000),
						std::make_pair(0x21000, 0x21000),
						std::make_pair(0x22000, 0x22000),
						std::make_pair(0x3FFFF, 0x3F000),
						std::make_pair(0x40000, 0x40000),
						std::make_pair(0x44000, 0x44000)
					);

					THEN("return region address") {
						auto* expected_region =
							gc::utils::as_ptr < gc::ref_region >(address.second);

						auto* region = m_map.get_ref_region(
							gc::utils::as_ptr < gc::reference >(address.first));

						REQUIRE(region == expected_region);
					}

				}

			}
		}

		SCENARIO("find ref block addresses from region", "[MODEL]") {

			std::size_t const input_block_size{ 4096 };
			std::size_t const input_unit_size{ 32 };

			gc::mapping const m_map{ input_block_size, input_unit_size };

			GIVEN("an initialized object m_map") {

				WHEN("get_ref_region for reference address") {

					// address / region expectancy
					auto address = GENERATE(
						std::make_pair(0x1000, 0x1000 + sizeof(gc::ref_region)),
						std::make_pair(0x2000, 0x2000 + sizeof(gc::ref_region))
					);

					THEN("return region address") {
						auto* expected_reference =
							gc::utils::as_ptr < gc::reference >(address.second);

						auto* reference = m_map.get_ref_block(
							gc::utils::as_ptr < gc::ref_region >(address.first));

						REQUIRE(reference  == expected_reference);
					}

				}

			}
		}

	}
}