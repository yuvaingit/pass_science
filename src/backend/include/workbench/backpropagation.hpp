#pragma once

#include "workbench/common.hpp"
#include <string>
#include <vector>

namespace workbench {

struct BackpropParams {
    double x = 1.0;
    double y_target = 2.0;
    double w = 0.5;
    double b = 0.0;
    double v = 1.0;
    double c = 0.0;
    double lr = 0.1;
    int steps = 100;
};

struct GradientCheckResult {
    std::string param_name;
    double analytical_grad = 0.0;
    double numerical_grad = 0.0;
    double absolute_error = 0.0;
    bool passed = false; // error <= 1e-5
};

struct BackpropStep {
    int step = 0;
    double w = 0.0;
    double b = 0.0;
    double v = 0.0;
    double c = 0.0;
    double h = 0.0;
    double y_hat = 0.0;
    double loss = 0.0;
    double grad_w = 0.0;
    double grad_b = 0.0;
    double grad_v = 0.0;
    double grad_c = 0.0;
};

struct BackpropVerificationResult {
    BackpropParams params;
    double initial_a = 0.0;
    double initial_h = 0.0;
    double initial_y_hat = 0.0;
    double initial_loss = 0.0;

    std::vector<GradientCheckResult> gradient_checks;
    std::vector<BackpropStep> history;
    double final_loss = 0.0;
    bool all_checks_passed = false;

    std::string to_json() const;
    std::string to_csv() const;
};

class BackpropagationEngine {
public:
    // Evaluate loss for given parameters
    static double compute_loss(double x, double y_target, double w, double b, double v, double c);

    // Compute exact analytical gradients by chain rule
    static void compute_analytical_gradients(
        double x, double y_target, double w, double b, double v, double c,
        double& grad_w, double& grad_b, double& grad_v, double& grad_c,
        double& out_h, double& out_y_hat, double& out_loss
    );

    // Check gradients using central finite differences with epsilon = 1e-6
    static std::vector<GradientCheckResult> check_gradients_finite_difference(
        const BackpropParams& p, double epsilon = 1e-6, double threshold = 1e-5
    );

    // Run full 100-step training verification
    static BackpropVerificationResult run_verification(const BackpropParams& p);
};

} // namespace workbench
