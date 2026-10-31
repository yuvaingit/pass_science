#pragma once

#include "workbench/common.hpp"
#include <string>
#include <vector>

namespace workbench {

struct ThermalParams {
    double length = 1.0;          // L = 1.0 m
    double area = 1e-4;           // A = 1e-4 m^2
    double conductivity = 10.0;   // k = 10.0 W/(m K)
    double heat_gen = 10000.0;    // q = 10,000 W/m^3
    double t0 = 100.0;            // T(0) = 100.0 C
    double tL = 20.0;             // T(L) = 20.0 C
    int elements = 20;            // N uniform elements
};

struct FENodalPoint {
    int index = 0;
    double x = 0.0;
    double t_fe = 0.0;
    double t_exact = 0.0;
    double abs_error = 0.0;
};

struct FEMidpoint {
    int element_index = 0;
    double x_mid = 0.0;
    double t_fe_mid = 0.0;
    double t_exact_mid = 0.0;
    double abs_error_mid = 0.0;
};

struct ConvergenceRow {
    int N = 0;
    double h = 0.0;
    double max_midpoint_error = 0.0;
    double order_of_convergence = 0.0;
};

struct ThermalFESolution {
    ThermalParams params;
    std::vector<FENodalPoint> nodes;
    std::vector<FEMidpoint> midpoints;
    double max_midpoint_error = 0.0;
    std::vector<ConvergenceRow> convergence_study;
    double solve_time_ms = 0.0;

    std::string to_json() const;
    std::string to_csv() const;
};

class ThermalSolver1D {
public:
    // Solve steady-state 1D thermal FEA for a given parameter set
    static ThermalFESolution solve(const ThermalParams& p);

    // Exact analytical temperature formula:
    // T_exact(x) = T0 + (TL - T0)*(x/L) + [q/(2k)] * x * (L - x)
    static double exact_temperature(double x, const ThermalParams& p);

    // Run convergence study across N = 10, 20, 40, 80 elements
    static std::vector<ConvergenceRow> run_convergence_study(const ThermalParams& base_params);
};

} // namespace workbench
