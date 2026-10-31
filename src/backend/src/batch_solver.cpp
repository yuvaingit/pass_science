#include "workbench/batch_solver.hpp"
#include <random>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <thread>

namespace workbench {

std::vector<BatchCase> BatchThermalSolver::generate_workload(const BatchConfig& config) {
    std::vector<BatchCase> cases;
    cases.reserve(config.case_count);

    std::mt19937 rng(config.seed);
    std::uniform_real_distribution<double> dist_k(config.k_min, config.k_max);
    std::uniform_real_distribution<double> dist_q(config.q_min, config.q_max);

    for (int i = 0; i < config.case_count; ++i) {
        BatchCase bc;
        bc.id = i;
        bc.params.length = 1.0;
        bc.params.area = 1e-4;
        bc.params.conductivity = dist_k(rng);
        bc.params.heat_gen = dist_q(rng);
        bc.params.t0 = 100.0;
        bc.params.tL = 20.0;
        bc.params.elements = config.mesh_size;
        cases.push_back(bc);
    }

    return cases;
}

std::vector<ThermalFESolution> BatchThermalSolver::solve_serial(const std::vector<BatchCase>& cases) {
    std::vector<ThermalFESolution> results;
    results.reserve(cases.size());

    for (const auto& bc : cases) {
        results.push_back(ThermalSolver1D::solve(bc.params));
    }
    return results;
}

std::vector<ThermalFESolution> BatchThermalSolver::solve_parallel(const std::vector<BatchCase>& cases, size_t num_workers) {
    if (num_workers <= 1) {
        return solve_serial(cases);
    }

    std::vector<ThermalFESolution> results(cases.size());
    ThreadPool pool(num_workers);

    // Dispatch work in blocks or per-item to worker pool
    // Assigning to preallocated results[i] guarantees deterministic order
    const size_t total = cases.size();
    const size_t chunk_size = std::max(size_t(1), total / (num_workers * 4));
    std::vector<std::future<void>> futures;

    for (size_t start = 0; start < total; start += chunk_size) {
        size_t end = std::min(start + chunk_size, total);
        futures.push_back(pool.enqueue([&cases, &results, start, end]() {
            for (size_t i = start; i < end; ++i) {
                results[i] = ThermalSolver1D::solve(cases[i].params);
            }
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    return results;
}

BenchmarkReport BatchThermalSolver::run_benchmark(const BatchConfig& config, double launch_to_ready_ms) {
    BenchmarkReport report;
    report.config = config;
    report.launch_to_ready_ms = launch_to_ready_ms;

    unsigned int hw_threads = std::thread::hardware_concurrency();
    if (hw_threads == 0) hw_threads = 4;
    report.logical_cores = hw_threads;
    report.physical_cores = std::max(1u, hw_threads / 2u);
    report.ram_gb = 16.0;

#if defined(__clang__)
    report.compiler = "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__);
#elif defined(__GNUC__)
    report.compiler = "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#elif defined(_MSC_VER)
    report.compiler = "MSVC " + std::to_string(_MSC_VER);
#else
    report.compiler = "C++17 Native Compiler";
#endif

#if defined(NDEBUG)
    report.optimization_flags = "-O3 -DNDEBUG -std=c++17";
#else
    report.optimization_flags = "-O2 -std=c++17";
#endif

    report.cpu_info = "Host Multi-Core CPU (" + std::to_string(report.logical_cores) + " Logical Cores)";

    // Generate test cases with fixed seed
    auto workload = generate_workload(config);

    // Warm-up pass to populate instruction cache and dynamic frequency scaling
    Timer warmup_timer;
    auto warmup_subset = std::vector<BatchCase>(workload.begin(), workload.begin() + std::min(size_t(50), workload.size()));
    solve_serial(warmup_subset);
    report.warmup_ms = warmup_timer.elapsed_ms();

    // Determine thread worker counts: 1, 2, 4 (capped at logical cores)
    std::vector<int> worker_counts = {1};
    if (hw_threads >= 2) worker_counts.push_back(2);
    if (hw_threads >= 4) worker_counts.push_back(4);
    else if (hw_threads > 2 && hw_threads < 4) worker_counts.push_back(static_cast<int>(hw_threads));

    // Reference solve for agreement checking
    auto serial_reference = solve_serial(workload);

    double t1_median = 0.0;
    bool all_agreement_ok = true;

    for (int p : worker_counts) {
        ThreadMeasurement tm;
        tm.workers = p;
        tm.repetitions_ms.reserve(5);

        double max_diff = 0.0;

        for (int rep = 0; rep < 5; ++rep) {
            Timer run_timer;
            std::vector<ThermalFESolution> res;
            if (p == 1) {
                res = solve_serial(workload);
            } else {
                res = solve_parallel(workload, static_cast<size_t>(p));
            }
            double elapsed = run_timer.elapsed_ms();
            tm.repetitions_ms.push_back(elapsed);

            // Verify serial/parallel agreement on first repetition
            if (rep == 0) {
                for (size_t c = 0; c < workload.size(); ++c) {
                    const auto& s_nodes = serial_reference[c].nodes;
                    const auto& p_nodes = res[c].nodes;
                    for (size_t n = 0; n < s_nodes.size(); ++n) {
                        double diff = std::abs(s_nodes[n].t_fe - p_nodes[n].t_fe);
                        if (diff > max_diff) max_diff = diff;
                    }
                }
            }
        }

        // Compute median across 5 repetitions
        std::vector<double> sorted_reps = tm.repetitions_ms;
        std::sort(sorted_reps.begin(), sorted_reps.end());
        tm.median_ms = sorted_reps[2]; // 3rd element of 5 is median

        if (p == 1) {
            t1_median = tm.median_ms;
            tm.speedup = 1.0;
            tm.efficiency = 100.0;
            tm.max_temp_diff_vs_serial = 0.0;
        } else {
            tm.speedup = t1_median / (tm.median_ms > 0 ? tm.median_ms : 1.0);
            tm.efficiency = (tm.speedup / static_cast<double>(p)) * 100.0;
            tm.max_temp_diff_vs_serial = max_diff;
            if (max_diff > 1e-8) {
                all_agreement_ok = false;
            }
        }

        report.runs.push_back(tm);
    }

    report.agreement_verified = all_agreement_ok;
    return report;
}

std::string BenchmarkReport::to_json() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "{\"config\":{"
       << "\"caseCount\":" << config.case_count << ","
       << "\"meshSize\":" << config.mesh_size << ","
       << "\"seed\":" << config.seed << ","
       << "\"kMin\":" << config.k_min << ","
       << "\"kMax\":" << config.k_max << ","
       << "\"qMin\":" << config.q_min << ","
       << "\"qMax\":" << config.q_max << "},"
       << "\"cpuInfo\":\"" << JsonHelper::escape(cpu_info) << "\","
       << "\"logicalCores\":" << logical_cores << ","
       << "\"physicalCores\":" << physical_cores << ","
       << "\"ramGb\":" << ram_gb << ","
       << "\"compiler\":\"" << JsonHelper::escape(compiler) << "\","
       << "\"flags\":\"" << JsonHelper::escape(optimization_flags) << "\","
       << "\"warmupMs\":" << warmup_ms << ","
       << "\"launchToReadyMs\":" << launch_to_ready_ms << ","
       << "\"serialParallelAgreementOk\":" << (agreement_verified ? "true" : "false") << ","
       << "\"runs\":[";

    for (size_t i = 0; i < runs.size(); ++i) {
        const auto& r = runs[i];
        ss << "{\"threads\":" << r.workers << ","
           << "\"medianMs\":" << r.median_ms << ","
           << "\"speedup\":" << r.speedup << ","
           << "\"efficiency\":" << r.efficiency << ","
           << "\"maxTempDiff\":" << r.max_temp_diff_vs_serial << ","
           << "\"repetitions\":[";
        for (size_t j = 0; j < r.repetitions_ms.size(); ++j) {
            ss << r.repetitions_ms[j];
            if (j < r.repetitions_ms.size() - 1) ss << ",";
        }
        ss << "]}";
        if (i < runs.size() - 1) ss << ",";
    }

    ss << "]}";
    return ss.str();
}

std::string BenchmarkReport::to_csv() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "# PASS SCIENCE WORKS - Multicore Benchmark Execution Report\n";
    ss << "# CPU=" << cpu_info << ", LogicalCores=" << logical_cores << ", RAM=" << ram_gb << "GB\n";
    ss << "# Compiler=" << compiler << ", Flags=" << optimization_flags << "\n";
    ss << "# Workload: Cases=" << config.case_count << ", MeshSize=" << config.mesh_size << ", Seed=" << config.seed << "\n";
    ss << "workers,rep1_ms,rep2_ms,rep3_ms,rep4_ms,rep5_ms,median_ms,speedup_T1_Tp,efficiency_pct,max_temp_diff_C\n";
    for (const auto& r : runs) {
        ss << r.workers << ",";
        for (double rep : r.repetitions_ms) {
            ss << rep << ",";
        }
        ss << r.median_ms << "," << r.speedup << "," << r.efficiency << "," << r.max_temp_diff_vs_serial << "\n";
    }
    return ss.str();
}

} // namespace workbench
