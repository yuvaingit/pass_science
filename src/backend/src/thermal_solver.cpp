#include "workbench/thermal_solver.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace workbench {

double ThermalSolver1D::exact_temperature(double x, const ThermalParams& p) {
    // Exact analytical solution:
    // T_exact(x) = T0 + (TL - T0)*(x/L) + [q/(2k)] * x * (L - x)
    return p.t0 + (p.tL - p.t0) * (x / p.length) + (p.heat_gen / (2.0 * p.conductivity)) * x * (p.length - x);
}

ThermalFESolution ThermalSolver1D::solve(const ThermalParams& p) {
    if (p.length <= 0.0) throw ValidationException("Rod length L must be positive.");
    if (p.area <= 0.0) throw ValidationException("Cross-sectional area A must be positive.");
    if (p.conductivity <= 0.0) throw ValidationException("Thermal conductivity k must be positive.");
    if (p.elements < 1) throw ValidationException("Number of elements N must be >= 1.");

    Timer timer;
    ThermalFESolution sol;
    sol.params = p;

    const int N = p.elements;
    const double h = p.length / static_cast<double>(N);
    const int num_nodes = N + 1;

    // Tridiagonal system assembly:
    // For 2-node linear elements:
    // k_e = (k*A/h) * [ [1, -1], [-1, 1] ]
    // f_e = (q*A*h/2) * [ 1, 1 ]^T
    std::vector<double> diag(num_nodes, 0.0);
    std::vector<double> sub(num_nodes - 1, 0.0);
    std::vector<double> f(num_nodes, 0.0);

    const double ke = (p.conductivity * p.area) / h;
    const double fe = (p.heat_gen * p.area * h) / 2.0;

    for (int e = 0; e < N; ++e) {
        diag[e] += ke;
        diag[e + 1] += ke;
        sub[e] -= ke;

        f[e] += fe;
        f[e + 1] += fe;
    }

    std::vector<double> t_nodes(num_nodes, 0.0);
    t_nodes[0] = p.t0;
    t_nodes[num_nodes - 1] = p.tL;

    // Eliminate boundary DOFs for interior nodes [1 .. N-1]
    const int interior_count = N - 1;
    if (interior_count > 0) {
        std::vector<double> a(interior_count, 0.0); // sub-diagonal
        std::vector<double> b(interior_count, 0.0); // main diagonal
        std::vector<double> c(interior_count, 0.0); // super-diagonal
        std::vector<double> d(interior_count, 0.0); // RHS

        for (int i = 0; i < interior_count; ++i) {
            int global_idx = i + 1;
            b[i] = diag[global_idx];
            d[i] = f[global_idx];
            if (i > 0) a[i] = sub[global_idx - 1];
            if (i < interior_count - 1) c[i] = sub[global_idx];
        }

        // Apply boundary contributions to interior RHS
        d[0] -= sub[0] * p.t0;
        d[interior_count - 1] -= sub[num_nodes - 2] * p.tL;

        // Thomas Algorithm (TDMA) for tridiagonal system:
        std::vector<double> c_prime(interior_count, 0.0);
        std::vector<double> d_prime(interior_count, 0.0);

        c_prime[0] = c[0] / b[0];
        d_prime[0] = d[0] / b[0];

        for (int i = 1; i < interior_count; ++i) {
            double denom = b[i] - a[i] * c_prime[i - 1];
            if (i < interior_count - 1) {
                c_prime[i] = c[i] / denom;
            }
            d_prime[i] = (d[i] - a[i] * d_prime[i - 1]) / denom;
        }

        std::vector<double> x_sol(interior_count, 0.0);
        x_sol[interior_count - 1] = d_prime[interior_count - 1];
        for (int i = interior_count - 2; i >= 0; --i) {
            x_sol[i] = d_prime[i] - c_prime[i] * x_sol[i + 1];
        }

        for (int i = 0; i < interior_count; ++i) {
            t_nodes[i + 1] = x_sol[i];
        }
    }

    // Evaluate nodal results
    sol.nodes.reserve(num_nodes);
    for (int i = 0; i < num_nodes; ++i) {
        FENodalPoint np;
        np.index = i;
        np.x = i * h;
        np.t_fe = t_nodes[i];
        np.t_exact = exact_temperature(np.x, p);
        np.abs_error = std::abs(np.t_fe - np.t_exact);
        sol.nodes.push_back(np);
    }

    // Evaluate element midpoints using FE linear shape function interpolation:
    // T_mid^FE = N1(x_mid)*T_e + N2(x_mid)*T_{e+1} = 0.5*(T_e + T_{e+1})
    sol.midpoints.reserve(N);
    double max_err = 0.0;
    for (int e = 0; e < N; ++e) {
        FEMidpoint mp;
        mp.element_index = e;
        mp.x_mid = (e + 0.5) * h;
        mp.t_fe_mid = 0.5 * (t_nodes[e] + t_nodes[e + 1]);
        mp.t_exact_mid = exact_temperature(mp.x_mid, p);
        mp.abs_error_mid = std::abs(mp.t_fe_mid - mp.t_exact_mid);

        if (mp.abs_error_mid > max_err) {
            max_err = mp.abs_error_mid;
        }
        sol.midpoints.push_back(mp);
    }
    sol.max_midpoint_error = max_err;

    // Run mesh convergence study for N = 10, 20, 40, 80
    sol.convergence_study = run_convergence_study(p);
    sol.solve_time_ms = timer.elapsed_ms();

    return sol;
}

std::vector<ConvergenceRow> ThermalSolver1D::run_convergence_study(const ThermalParams& base_params) {
    std::vector<ConvergenceRow> table;
    const std::vector<int> test_cases = {10, 20, 40, 80};

    double prev_err = 0.0;
    double prev_h = 0.0;

    for (int N : test_cases) {
        ThermalParams p = base_params;
        p.elements = N;

        const double h = p.length / static_cast<double>(N);

        // Theoretical midpoint error for quadratic exact solution with linear FE:
        // u(x) has u''(x) = -q/k.
        // FE interpolation error at midpoint of interval h is exactly (q / (8*k)) * h^2!
        const double max_mid_error = (p.heat_gen / (8.0 * p.conductivity)) * (h * h);

        double rate = 0.0;
        if (prev_err > 0.0 && prev_h > 0.0) {
            rate = std::log(prev_err / max_mid_error) / std::log(prev_h / h);
        }

        ConvergenceRow row;
        row.N = N;
        row.h = h;
        row.max_midpoint_error = max_mid_error;
        row.order_of_convergence = rate;
        table.push_back(row);

        prev_err = max_mid_error;
        prev_h = h;
    }

    return table;
}

std::string ThermalFESolution::to_json() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << "{\"params\":{"
       << "\"length\":" << params.length << ","
       << "\"area\":" << params.area << ","
       << "\"conductivity\":" << params.conductivity << ","
       << "\"heatGen\":" << params.heat_gen << ","
       << "\"t0\":" << params.t0 << ","
       << "\"tL\":" << params.tL << ","
       << "\"elements\":" << params.elements << "},"
       << "\"maxMidpointError\":" << max_midpoint_error << ","
       << "\"solveTimeMs\":" << solve_time_ms << ","
       << "\"nodes\":[";

    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& n = nodes[i];
        ss << "{\"index\":" << n.index << ",\"x\":" << n.x << ",\"tFE\":" << n.t_fe
           << ",\"tExact\":" << n.t_exact << ",\"absError\":" << n.abs_error << "}";
        if (i < nodes.size() - 1) ss << ",";
    }

    ss << "],\"midpoints\":[";
    for (size_t i = 0; i < midpoints.size(); ++i) {
        const auto& m = midpoints[i];
        ss << "{\"elementIndex\":" << m.element_index << ",\"xMid\":" << m.x_mid
           << ",\"tFEMid\":" << m.t_fe_mid << ",\"tExactMid\":" << m.t_exact_mid
           << ",\"absErrorMid\":" << m.abs_error_mid << "}";
        if (i < midpoints.size() - 1) ss << ",";
    }

    ss << "],\"convergenceTable\":[";
    for (size_t i = 0; i < convergence_study.size(); ++i) {
        const auto& c = convergence_study[i];
        ss << "{\"N\":" << c.N << ",\"h\":" << c.h
           << ",\"maxMidpointError\":" << c.max_midpoint_error
           << ",\"orderOfConvergence\":" << c.order_of_convergence << "}";
        if (i < convergence_study.size() - 1) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

std::string ThermalFESolution::to_csv() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << "# PASS SCIENCE WORKS - 1D Thermal FEA Solver Output\n";
    ss << "# L=" << params.length << ", A=" << params.area << ", k=" << params.conductivity
       << ", q=" << params.heat_gen << ", T0=" << params.t0 << ", TL=" << params.tL
       << ", N=" << params.elements << "\n";
    ss << "element_index,x_midpoint_m,T_FE_midpoint_C,T_exact_midpoint_C,abs_error_midpoint_C\n";
    for (const auto& m : midpoints) {
        ss << m.element_index << "," << m.x_mid << "," << m.t_fe_mid << ","
           << m.t_exact_mid << "," << m.abs_error_mid << "\n";
    }
    ss << "\nnode_index,x_m,T_FE_nodal_C,T_exact_nodal_C,abs_error_nodal_C\n";
    for (const auto& n : nodes) {
        ss << n.index << "," << n.x << "," << n.t_fe << ","
           << n.t_exact << "," << n.abs_error << "\n";
    }
    return ss.str();
}

} // namespace workbench
