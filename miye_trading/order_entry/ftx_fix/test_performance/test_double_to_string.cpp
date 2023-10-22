#include "benchmark/benchmark.h"
#include <iostream>
#include <string.h>

#include "../../ftx_fix/double_to_string.hpp"

static void double_to_string(benchmark::State& state)
{
    char buffer[32];
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(acutus::dtoa(982747626.232894383, buffer));
        //std::cout << buffer << std::endl;
    }
}

BENCHMARK(double_to_string);


static void double_to_std_string(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(std::to_string(982747626.232894383));
    }
}

BENCHMARK(double_to_std_string);
