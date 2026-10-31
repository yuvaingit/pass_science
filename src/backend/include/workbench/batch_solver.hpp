#pragma once

#include "workbench/common.hpp"
#include "workbench/thermal_solver.hpp"
#include "workbench/thread_pool.hpp"
#include <vector>
#include <string>

namespace workbench {

struct BatchConfig {
    int case_count = 1000;
    int mesh_size = 50;
    uint32_t seed = 42;
    double k_min = 5.0;
    double k_max = 50.0;
    double q_min = 1000.0;
    double q_max = 20000.0;
};

struct BatchCase {
    int id = 0;
    ThermalParams params;
};

struct ThreadMeasurement {
    int workers = 1;
    double pool_init_ms = 0.0;            // Separate initialization overhead
    double warmup_ms = 0.0;               // Warmup run duration
    std::vector<double> repetitions_ms;    // 5 measured repetitions
    double median_ms = 0.0;
    double serial_time_ms = 0.0;          // T_serial
    double parallel_time_ms = 0.0;        // T_parallel
    double speedup = 1.0;                 // T1 / Tp
    double efficiency = 100.0;             // (T1 / Tp) / p * 100%
    double pool_shutdown_ms = 0.0;        // Separate shutdown overhead
    double max_temp_diff_vs_serial = 0.0; // Numerical agreement verification (<= 1e-8 C)
};

struct BenchmarkReport {
    BatchConfig config;
    std::string cpu_info;
    unsigned int logical_cores = 0;
    std::string physical_cores_str = "unavailable";
    unsigned int physical_cores = 0;
    double ram_gb = 16.0;
    std::string compiler;
    std::string optimization_flags;
    double warmup_ms = 0.0;
    double launch_to_ready_ms = 0.0;
    std::vector<ThreadMeasurement> runs;
    bool agreement_verified = false;

    std::string to_json() const;
    std::string to_csv() const;
};

class BatchThermalSolver {
public:
    // Generate deterministic batch cases with fixed PRNG seed and varied positive k & q
    static std::vector<BatchCase> generate_workload(const BatchConfig& config);

    // Solve batch sequentially (serial reference) with optional cancellation
    static std::vector<ThermalFESolution> solve_serial(
        const std::vector<BatchCase>& cases,
        std::atomic<bool>* cancel_flag = nullptr
    );

    // Solve batch in parallel using a pre-initialized, reusable bounded worker pool
    static std::vector<ThermalFESolution> solve_parallel(
        const std::vector<BatchCase>& cases,
        ThreadPool& pool,
        std::atomic<bool>* cancel_flag = nullptr
    );

    // Convenience overload that manages its own pool
    static std::vector<ThermalFESolution> solve_parallel(
        const std::vector<BatchCase>& cases,
        size_t num_workers,
        std::atomic<bool>* cancel_flag = nullptr
    );

    // Full multicore benchmark suite: 1, 2, 4 threads, 5 repetitions, warm-up, pool reuse, agreement checks
    static BenchmarkReport run_benchmark(
        const BatchConfig& config,
        double launch_to_ready_ms = 40.0,
        std::atomic<bool>* cancel_flag = nullptr
    );
};

} // namespace workbench
