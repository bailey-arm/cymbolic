cmake_minimum_required(VERSION 3.14)
project(cymbolic VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clang-tidy / IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Options ───────────────────────────────────────────────────────────────────
option(CYMBOLIC_BUILD_TESTS      "Build unit tests"   ON)
option(CYMBOLIC_BUILD_BENCHMARKS "Build benchmarks"   ON)
option(CYMBOLIC_BUILD_EXAMPLES   "Build examples"     ON)

# ── Dependencies (FetchContent pulls them automatically) ──────────────────────
include(FetchContent)

# Eigen – header-only linear algebra.
# Disable Eigen's own test/benchmark/doc targets before processing its CMakeLists.
set(EIGEN_BUILD_TESTING  OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_BTL      OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_DOC      OFF CACHE BOOL "" FORCE)
# Prevent Eigen from registering its 900+ CTest entries
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG        3.4.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(eigen)
# Re-enable testing for our own targets
set(BUILD_TESTING ON CACHE BOOL "" FORCE)

# GoogleTest – unit testing
if(CYMBOLIC_BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Google Benchmark – microbenchmarks
if(CYMBOLIC_BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.3
        GIT_SHALLOW    TRUE
    )
    set(BENCHMARK_ENABLE_TESTING  OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL  OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
endif()

# ── Core library ──────────────────────────────────────────────────────────────
# Add source files here as you create them:
#   src/expr.cpp, src/autodiff.cpp, etc.
add_library(cymbolic STATIC
    src/placeholder.cpp   # remove once real sources exist
)

target_include_directories(cymbolic
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(cymbolic PUBLIC Eigen3::Eigen)

target_compile_options(cymbolic PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall -Wextra -Wpedantic>
)

# ── Tests ─────────────────────────────────────────────────────────────────────
if(CYMBOLIC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ── Benchmarks ────────────────────────────────────────────────────────────────
if(CYMBOLIC_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# ── Examples ──────────────────────────────────────────────────────────────────
if(CYMBOLIC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
