#include <benchmark/benchmark.h>
#include <sgc.h>

struct demo {
	sgc::gc_ptr <demo> to;
};

void gc_alloc_assign(benchmark::State & state) {

	{
		auto root = sgc::make_gc<demo>();
		auto node = root;

		for (auto _ : state) {

			for (decltype (state.range(0)) i = 0; i < state.range(0); ++i) {
				node->to = sgc::make_gc<demo>();
			}
		}
	}

	sgc::gc_full_collect();
	sgc::gc_full_collect();
}

void gc_collect(benchmark::State &state) {

	for (auto _ : state) {
		state.PauseTiming();
		{
			auto root = sgc::make_gc<demo>();
			auto node = root;


			for (decltype (state.range(0)) i = 0; i < state.range(0); ++i) {
				node->to = sgc::make_gc<demo>();
			}
		}
		state.ResumeTiming();

		sgc::gc_full_collect();
		sgc::gc_full_collect();
	}
}

struct no_gc_demo {
	no_gc_demo * cenas;
};

void no_gc_baseline_alloc(benchmark::State &state) {

	std::vector<std::unique_ptr<no_gc_demo> > recovery;

	for (auto _ : state) {
		auto root = new no_gc_demo();

		for (decltype (state.range(0)) i = 0; i < state.range(0); ++i) {
			auto obj = new no_gc_demo();
			obj->cenas = root;
			root->cenas = obj;

			state.PauseTiming();
			recovery.emplace_back(obj);
			state.ResumeTiming();
		}

	}

	state.PauseTiming();
	recovery.clear();
	state.ResumeTiming();
}

void no_gc_baseline_collect(benchmark::State &state) {

	std::vector<std::unique_ptr<no_gc_demo> > recovery;

	for (auto _ : state) {
		auto root = new no_gc_demo();

		state.PauseTiming();
		for (decltype (state.range(0)) i = 0; i < state.range(0); ++i) {
			auto obj = new no_gc_demo();
			obj->cenas = root;
			root->cenas = obj;

			recovery.emplace_back(obj);
		}
		state.ResumeTiming();

		recovery.clear();
	}
}

#define BENCH_LOW 1U << 8U
#define BENCH_HIGH 1U << 18U

BENCHMARK(gc_alloc_assign)->Range(BENCH_LOW, BENCH_HIGH);
//BENCHMARK(no_gc_baseline_alloc)->Range(BENCH_LOW, BENCH_HIGH);

//BENCHMARK(gc_collect)->Range(BENCH_LOW, BENCH_HIGH);
//BENCHMARK(no_gc_baseline_collect)->Range(BENCH_LOW, BENCH_HIGH);

BENCHMARK_MAIN();