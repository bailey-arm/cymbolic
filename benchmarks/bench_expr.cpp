#include <benchmark/benchmark.h>
#include "cymbolic/expr.hpp"

// Placeholder benchmark – replace with real ones as you implement nodes.
// Pattern:
//   static void BM_EvalSomething(benchmark::State& state) {
//       auto expr = ...; // build your expression once
//       for (auto _ : state) {
//           benchmark::DoNotOptimize(expr->eval(env));
//       }
//   }
//   BENCHMARK(BM_EvalSomething)->Range(1, 1 << 16);

static void BM_Placeholder(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(42);
    }
}
BENCHMARK(BM_Placeholder);
