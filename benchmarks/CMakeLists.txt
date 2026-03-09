add_executable(cymbolic_benchmarks
    bench_expr.cpp
)

target_link_libraries(cymbolic_benchmarks
    PRIVATE
        cymbolic
        benchmark::benchmark_main
)
