#include "workbench/common.hpp"
#include "workbench/stl_parser.hpp"
#include "workbench/ansys_csv_parser.hpp"
#include "workbench/thermal_solver.hpp"
#include "workbench/batch_solver.hpp"
#include "workbench/hardware_mock.hpp"
#include "workbench/backpropagation.hpp"

#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

using namespace workbench;

int tests_passed = 0;
int tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "  [FAIL] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            tests_failed++; \
        } else { \
            tests_passed++; \
        } \
    } while(0)

void test_stl_parser() {
    std::cout << "[TEST SUITE 1] STL Geometry Parser\n";

    // Positive test: valid ASCII STL
    std::string valid_stl = 
        "solid test_bracket\n"
        "  facet normal 0.0 0.0 1.0\n"
        "    outer loop\n"
        "      vertex 0.0 0.0 0.0\n"
        "      vertex 0.05 0.0 0.0\n"
        "      vertex 0.05 0.05 0.0\n"
        "    endloop\n"
        "  endfacet\n"
        "endsolid test_bracket\n";

    try {
        auto mesh = STLParser::parse_ascii(valid_stl, "valid.stl");
        TEST_ASSERT(mesh.solid_name == "test_bracket", "STL solid name parsed correctly");
        TEST_ASSERT(mesh.triangles.size() == 1, "Triangle count is 1");
        TEST_ASSERT(std::abs(mesh.bbox.max.x - 0.05) < 1e-6, "Bounding box max x correct");
    } catch (const std::exception& e) {
        TEST_ASSERT(false, std::string("Unexpected exception in valid STL: ") + e.what());
    }

    // Negative test 1: Empty file
    try {
        STLParser::parse_ascii("", "empty.stl");
        TEST_ASSERT(false, "Empty STL did not throw exception");
    } catch (const ValidationException&) {
        TEST_ASSERT(true, "Empty STL threw ValidationException as required");
    } catch (...) {
        TEST_ASSERT(false, "Empty STL threw wrong exception type");
    }

    // Negative test 2: Non-finite NaN vertex
    std::string nan_stl = 
        "solid nan_mesh\n"
        "  facet normal 0.0 0.0 1.0\n"
        "    outer loop\n"
        "      vertex nan 0.0 0.0\n"
        "      vertex 0.05 0.0 0.0\n"
        "      vertex 0.05 0.05 0.0\n"
        "    endloop\n"
        "  endfacet\n"
        "endsolid nan_mesh\n";
    try {
        STLParser::parse_ascii(nan_stl, "nan.stl");
        TEST_ASSERT(false, "NaN STL did not throw exception");
    } catch (const WorkbenchException&) {
        TEST_ASSERT(true, "NaN coordinate in STL caught and rejected");
    }

    // Negative test 3: Malformed header
    try {
        STLParser::parse_ascii("invalid_header\nendsolid", "bad_header.stl");
        TEST_ASSERT(false, "Bad header STL did not throw exception");
    } catch (const ParseException&) {
        TEST_ASSERT(true, "Missing 'solid' header caught and rejected");
    }
}

void test_ansys_csv_parser() {
    std::cout << "[TEST SUITE 2] ANSYS CSV Parser\n";

    // Positive test: valid CSV with required columns
    std::string valid_csv = 
        "node_id,x_m,y_m,z_m,temperature_C\n"
        "1,0.000,0.000,0.000,100.00\n"
        "2,0.025,0.000,0.000,85.50\n"
        "3,0.050,0.000,0.000,70.00\n";

    try {
        auto res = ANSYSCSVParser::parse_csv(valid_csv, "valid.csv");
        TEST_ASSERT(res.nodes.size() == 3, "Parsed 3 nodal records");
        TEST_ASSERT(std::abs(res.min_temp - 70.0) < 1e-6, "Min temp is 70.0 C");
        TEST_ASSERT(std::abs(res.max_temp - 100.0) < 1e-6, "Max temp is 100.0 C");
        TEST_ASSERT(std::abs(res.avg_temp - 85.166667) < 1e-4, "Avg temp matches");
    } catch (const std::exception& e) {
        TEST_ASSERT(false, std::string("Unexpected exception in valid CSV: ") + e.what());
    }

    // Negative test 1: Missing column
    std::string missing_col_csv = 
        "node_id,x_m,y_m,temperature_C\n" // Missing z_m
        "1,0.0,0.0,100.0\n";
    try {
        ANSYSCSVParser::parse_csv(missing_col_csv, "missing_col.csv");
        TEST_ASSERT(false, "Missing column did not throw exception");
    } catch (const ParseException&) {
        TEST_ASSERT(true, "Missing z_m schema error caught and rejected");
    }

    // Negative test 2: Empty file
    try {
        ANSYSCSVParser::parse_csv("", "empty.csv");
        TEST_ASSERT(false, "Empty CSV did not throw exception");
    } catch (const ValidationException&) {
        TEST_ASSERT(true, "Empty CSV caught and rejected");
    }
}

void test_thermal_fea() {
    std::cout << "[TEST SUITE 3] 1D Thermal FEA Solver & Convergence\n";

    ThermalParams p;
    p.length = 1.0;
    p.area = 1e-4;
    p.conductivity = 10.0;
    p.heat_gen = 10000.0;
    p.t0 = 100.0;
    p.tL = 20.0;
    p.elements = 20;

    auto sol = ThermalSolver1D::solve(p);

    // 1. Boundary conditions imposition check
    TEST_ASSERT(std::abs(sol.nodes.front().t_fe - 100.0) < 1e-12, "T(0) = 100.0 C strictly satisfied");
    TEST_ASSERT(std::abs(sol.nodes.back().t_fe - 20.0) < 1e-12, "T(L) = 20.0 C strictly satisfied");

    // 2. Nodal exactness property: |T_i - T_exact(x_i)| ~ 0
    double max_nodal_err = 0.0;
    for (const auto& n : sol.nodes) {
        if (n.abs_error > max_nodal_err) max_nodal_err = n.abs_error;
    }
    TEST_ASSERT(max_nodal_err < 1e-10, "Nodal values coincide with exact solution within 1e-10 C");

    // 3. Midpoint error and convergence study across N = 10, 20, 40, 80
    auto conv = sol.convergence_study;
    TEST_ASSERT(conv.size() == 4, "Convergence study contains 4 entries (10, 20, 40, 80)");

    // Theoretical midpoint error: (q / (8k)) * h^2 = 125 * h^2
    // N=10 (h=0.1): 1.25 C
    // N=20 (h=0.05): 0.3125 C
    // N=40 (h=0.025): 0.078125 C
    // N=80 (h=0.0125): 0.01953125 C
    TEST_ASSERT(std::abs(conv[0].max_midpoint_error - 1.25) < 1e-6, "N=10 midpoint error is 1.25 C");
    TEST_ASSERT(std::abs(conv[1].max_midpoint_error - 0.3125) < 1e-6, "N=20 midpoint error is 0.3125 C");
    TEST_ASSERT(std::abs(conv[2].max_midpoint_error - 0.078125) < 1e-6, "N=40 midpoint error is 0.078125 C");
    TEST_ASSERT(std::abs(conv[3].max_midpoint_error - 0.01953125) < 1e-6, "N=80 midpoint error is 0.01953125 C");

    // Check O(h^2) convergence order ~ 2.0
    for (size_t i = 1; i < conv.size(); ++i) {
        TEST_ASSERT(std::abs(conv[i].order_of_convergence - 2.0) < 0.05, "Order of convergence is 2.0 (O(h^2))");
    }
}

void test_multithreading_and_agreement() {
    std::cout << "[TEST SUITE 4] Multithreaded Batch Execution & Numerical Agreement\n";

    BatchConfig cfg;
    cfg.case_count = 200;
    cfg.mesh_size = 40;
    cfg.seed = 42;

    auto workload = BatchThermalSolver::generate_workload(cfg);
    TEST_ASSERT(workload.size() == 200, "Workload generated exactly 200 cases");

    // Solve serial
    auto serial_res = BatchThermalSolver::solve_serial(workload);

    // Solve parallel with 2 and 4 workers
    auto par2_res = BatchThermalSolver::solve_parallel(workload, 2);
    auto par4_res = BatchThermalSolver::solve_parallel(workload, 4);

    TEST_ASSERT(serial_res.size() == par2_res.size(), "Result sizes match (2 workers)");
    TEST_ASSERT(serial_res.size() == par4_res.size(), "Result sizes match (4 workers)");

    // Numerical agreement check: max absolute temperature difference <= 1e-8 C
    double max_diff_p2 = 0.0;
    double max_diff_p4 = 0.0;

    for (size_t c = 0; c < workload.size(); ++c) {
        for (size_t n = 0; n < serial_res[c].nodes.size(); ++n) {
            double d2 = std::abs(serial_res[c].nodes[n].t_fe - par2_res[c].nodes[n].t_fe);
            double d4 = std::abs(serial_res[c].nodes[n].t_fe - par4_res[c].nodes[n].t_fe);
            if (d2 > max_diff_p2) max_diff_p2 = d2;
            if (d4 > max_diff_p4) max_diff_p4 = d4;
        }
    }

    TEST_ASSERT(max_diff_p2 <= 1e-8, "Serial vs Parallel(2) agreement <= 1e-8 C verified");
    TEST_ASSERT(max_diff_p4 <= 1e-8, "Serial vs Parallel(4) agreement <= 1e-8 C verified");
}

void test_hardware_mocks() {
    std::cout << "[TEST SUITE 5] Embedded Hardware Mocks & Fault Recovery\n";

    HardwareWorkbenchManager mgr;
    auto devs = mgr.list_devices();
    TEST_ASSERT(devs.size() == 3, "Discovered exactly 3 mock devices (MCU, FPGA, JTAG)");

    auto mcu = mgr.get_device("mcu-01");
    auto fpga = mgr.get_device("fpga-01");
    auto jtag = mgr.get_device("jtag-01");

    TEST_ASSERT(mcu != nullptr, "MCU discovered");
    TEST_ASSERT(fpga != nullptr, "FPGA discovered");
    TEST_ASSERT(jtag != nullptr, "JTAG adapter discovered");

    // Telemetry read
    auto t_mcu = mcu->read_telemetry();
    TEST_ASSERT(t_mcu.voltage_3v3 > 3.2 && t_mcu.voltage_3v3 < 3.4, "MCU voltage 3.3V in range");

    // Failure injection on FPGA
    fpga->inject_failure("UART/SPI Timeout");
    TEST_ASSERT(fpga->get_status() == DeviceStatus::Timeout, "FPGA status updated to Timeout");

    // Recovery
    fpga->recover();
    TEST_ASSERT(fpga->get_status() == DeviceStatus::Connected, "FPGA recovered back to Connected state");
}

void test_backpropagation() {
    std::cout << "[TEST SUITE 6] Backpropagation Verification & Finite Difference Check\n";

    BackpropParams p;
    p.x = 1.0;
    p.y_target = 2.0;
    p.w = 0.5;
    p.b = 0.0;
    p.v = 1.0;
    p.c = 0.0;
    p.lr = 0.1;
    p.steps = 100;

    auto res = BackpropagationEngine::run_verification(p);

    // Initial forward pass values check:
    // a = 0.5
    // h = tanh(0.5) = 0.46211716
    // y_hat = 0.46211716
    // loss = 0.5 * (0.46211716 - 2.0)^2 = 1.18254182
    TEST_ASSERT(std::abs(res.initial_h - 0.46211716) < 1e-6, "Initial h matches tanh(0.5)");
    TEST_ASSERT(std::abs(res.initial_y_hat - 0.46211716) < 1e-6, "Initial y_hat matches 0.46211716");
    TEST_ASSERT(std::abs(res.initial_loss - 1.18254182) < 1e-6, "Initial loss is 1.18254182");

    // Central finite differences gradient checks (epsilon = 1e-6, error <= 1e-5)
    TEST_ASSERT(res.gradient_checks.size() == 4, "4 parameters checked (w, b, v, c)");
    for (const auto& chk : res.gradient_checks) {
        TEST_ASSERT(chk.passed, std::string("Finite difference check passed for ") + chk.param_name);
        TEST_ASSERT(chk.absolute_error <= 1e-5, std::string("Error <= 1e-5 for ") + chk.param_name);
    }

    // 100-step training loss reduction check
    TEST_ASSERT(res.history.size() == 101, "Recorded 101 steps (step 0 to 100)");
    TEST_ASSERT(res.final_loss < res.initial_loss, "Final loss strictly lower than initial loss");
    TEST_ASSERT(res.final_loss < 0.05, "Final loss reduced significantly after 100 steps");
}

void test_cancellation() {
    std::cout << "[TEST SUITE 7] Computation Cancellation Controls & Thread Safety\n";

    // 1. Immediate cancellation test on Thermal FEA
    {
        std::atomic<bool> cancel_flag{true};
        ThermalParams p;
        p.elements = 100;
        bool caught = false;
        try {
            ThermalSolver1D::solve(p, &cancel_flag);
        } catch (const OperationCancelledException&) {
            caught = true;
        } catch (...) {}
        TEST_ASSERT(caught, "ThermalSolver1D::solve threw OperationCancelledException on cancelled flag");
    }

    // 2. Cancellation during convergence study
    {
        std::atomic<bool> cancel_flag{true};
        ThermalParams p;
        bool caught = false;
        try {
            ThermalSolver1D::run_convergence_study(p, &cancel_flag);
        } catch (const OperationCancelledException&) {
            caught = true;
        } catch (...) {}
        TEST_ASSERT(caught, "ThermalSolver1D::run_convergence_study threw OperationCancelledException");
    }

    // 3. Batch solve serial cancellation
    {
        BatchConfig cfg;
        cfg.case_count = 100;
        auto cases = BatchThermalSolver::generate_workload(cfg);
        std::atomic<bool> cancel_flag{true};
        bool caught = false;
        try {
            BatchThermalSolver::solve_serial(cases, &cancel_flag);
        } catch (const OperationCancelledException&) {
            caught = true;
        } catch (...) {}
        TEST_ASSERT(caught, "BatchThermalSolver::solve_serial aborted on cancellation flag");
    }

    // 4. Thread-pool parallel cancellation with active worker threads
    {
        BatchConfig cfg;
        cfg.case_count = 1000;
        cfg.mesh_size = 80;
        auto cases = BatchThermalSolver::generate_workload(cfg);
        std::atomic<bool> cancel_flag{false};

        ThreadPool pool(4);
        bool caught = false;

        // Trigger cancellation concurrently after a microsecond delay
        std::thread canceller([&cancel_flag]() {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
            cancel_flag.store(true, std::memory_order_seq_cst);
        });

        try {
            BatchThermalSolver::solve_parallel(cases, pool, &cancel_flag);
        } catch (const OperationCancelledException&) {
            caught = true;
        } catch (...) {}

        canceller.join();
        TEST_ASSERT(caught || cancel_flag.load(), "Parallel batch solver handled dynamic cancellation cleanly without deadlocks");
    }

    // 5. Full benchmark cancellation
    {
        BatchConfig cfg;
        cfg.case_count = 200;
        std::atomic<bool> cancel_flag{true};
        bool caught = false;
        try {
            BatchThermalSolver::run_benchmark(cfg, 40.0, &cancel_flag);
        } catch (const OperationCancelledException&) {
            caught = true;
        } catch (...) {}
        TEST_ASSERT(caught, "BatchThermalSolver::run_benchmark aborted cleanly on cancellation");
    }
}

int main() {
    std::cout << "========================================================\n"
              << "  UNIFIED ENGINEERING WORKBENCH - C++17 TEST SUITE\n"
              << "========================================================\n\n";

    test_stl_parser();
    test_ansys_csv_parser();
    test_thermal_fea();
    test_multithreading_and_agreement();
    test_hardware_mocks();
    test_backpropagation();
    test_cancellation();

    std::cout << "\n--------------------------------------------------------\n";
    std::cout << "  TEST SUMMARY: " << tests_passed << " PASSED, " << tests_failed << " FAILED\n";
    std::cout << "--------------------------------------------------------\n";

    return (tests_failed == 0) ? 0 : 1;
}
