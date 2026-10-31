#include "workbench/common.hpp"
#include "workbench/stl_parser.hpp"
#include "workbench/ansys_csv_parser.hpp"
#include "workbench/thermal_solver.hpp"
#include "workbench/batch_solver.hpp"
#include "workbench/hardware_mock.hpp"
#include "workbench/backpropagation.hpp"
#include "workbench/http_server.hpp"

#include <iostream>
#include <string>
#include <vector>

void print_help() {
    std::cout << "Unified Engineering Workbench - C++17 Core Engine\n"
              << "PASS SCIENCE WORKS | R&D Common Technical Assignment 2026-2027\n\n"
              << "Usage:\n"
              << "  workbench_server [--port <port>] [--no-browser] [--static <dir>]\n"
              << "  workbench_server --fea [--elements <N>]\n"
              << "  workbench_server --benchmark [--cases <N>] [--threads <P>]\n"
              << "  workbench_server --backprop\n"
              << "  workbench_server --help\n";
}

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string static_dir = "dist";
    bool launch_browser = true;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_help();
            return 0;
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--no-browser") {
            launch_browser = false;
        } else if (arg == "--static" && i + 1 < argc) {
            static_dir = argv[++i];
        } else if (arg == "--fea") {
            workbench::ThermalParams p;
            p.elements = (i + 1 < argc && argv[i + 1][0] != '-') ? std::stoi(argv[++i]) : 20;
            auto sol = workbench::ThermalSolver1D::solve(p);
            std::cout << sol.to_csv() << std::endl;
            return 0;
        } else if (arg == "--benchmark") {
            workbench::BatchConfig cfg;
            cfg.case_count = 500;
            auto rep = workbench::BatchThermalSolver::run_benchmark(cfg);
            std::cout << rep.to_csv() << std::endl;
            return 0;
        } else if (arg == "--backprop") {
            workbench::BackpropParams p;
            auto res = workbench::BackpropagationEngine::run_verification(p);
            std::cout << res.to_csv() << std::endl;
            return 0;
        }
    }

    workbench::HttpServer server(port, static_dir);
    if (!server.start()) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }

    if (launch_browser) {
        server.open_browser();
    }

    server.run_loop();
    return 0;
}
