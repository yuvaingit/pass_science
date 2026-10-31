#include "workbench/backpropagation.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace workbench {

double BackpropagationEngine::compute_loss(double x, double y_target, double w, double b, double v, double c) {
    double a = w * x + b;
    double h = std::tanh(a);
    double y_hat = v * h + c;
    return 0.5 * (y_hat - y_target) * (y_hat - y_target);
}

void BackpropagationEngine::compute_analytical_gradients(
    double x, double y_target, double w, double b, double v, double c,
    double& grad_w, double& grad_b, double& grad_v, double& grad_c,
    double& out_h, double& out_y_hat, double& out_loss
) {
    double a = w * x + b;
    out_h = std::tanh(a);
    out_y_hat = v * out_h + c;
    double error = out_y_hat - y_target;
    out_loss = 0.5 * error * error;

    // Chain Rule:
    // dJ/dy_hat = (y_hat - y)
    // dJ/dv = (y_hat - y) * h
    // dJ/dc = (y_hat - y) * 1
    // dJ/dh = (y_hat - y) * v
    // dh/da = 1 - tanh^2(a) = 1 - h^2
    // dJ/da = (dJ/dh) * (dh/da) = (y_hat - y) * v * (1 - h^2)
    // dJ/dw = (dJ/da) * (da/dw) = (dJ/da) * x
    // dJ/db = (dJ/da) * (da/db) = (dJ/da) * 1
    grad_v = error * out_h;
    grad_c = error * 1.0;

    double dh_da = 1.0 - (out_h * out_h);
    double dj_da = error * v * dh_da;

    grad_w = dj_da * x;
    grad_b = dj_da * 1.0;
}

std::vector<GradientCheckResult> BackpropagationEngine::check_gradients_finite_difference(
    const BackpropParams& p, double epsilon, double threshold
) {
    double g_w = 0.0, g_b = 0.0, g_v = 0.0, g_c = 0.0;
    double h = 0.0, y_hat = 0.0, loss = 0.0;
    compute_analytical_gradients(p.x, p.y_target, p.w, p.b, p.v, p.c, g_w, g_b, g_v, g_c, h, y_hat, loss);

    // Central finite differences: [J(theta + eps) - J(theta - eps)] / (2 * eps)
    double num_w = (compute_loss(p.x, p.y_target, p.w + epsilon, p.b, p.v, p.c) -
                    compute_loss(p.x, p.y_target, p.w - epsilon, p.b, p.v, p.c)) / (2.0 * epsilon);

    double num_b = (compute_loss(p.x, p.y_target, p.w, p.b + epsilon, p.v, p.c) -
                    compute_loss(p.x, p.y_target, p.w, p.b - epsilon, p.v, p.c)) / (2.0 * epsilon);

    double num_v = (compute_loss(p.x, p.y_target, p.w, p.b, p.v + epsilon, p.c) -
                    compute_loss(p.x, p.y_target, p.w, p.b, p.v - epsilon, p.c)) / (2.0 * epsilon);

    double num_c = (compute_loss(p.x, p.y_target, p.w, p.b, p.v, p.c + epsilon) -
                    compute_loss(p.x, p.y_target, p.w, p.b, p.v, p.c - epsilon)) / (2.0 * epsilon);

    std::vector<GradientCheckResult> results;

    auto add_check = [&](const std::string& name, double anal, double num) {
        GradientCheckResult r;
        r.param_name = name;
        r.analytical_grad = anal;
        r.numerical_grad = num;
        r.absolute_error = std::abs(anal - num);
        r.passed = (r.absolute_error <= threshold);
        results.push_back(r);
    };

    add_check("w", g_w, num_w);
    add_check("b", g_b, num_b);
    add_check("v", g_v, num_v);
    add_check("c", g_c, num_c);

    return results;
}

BackpropVerificationResult BackpropagationEngine::run_verification(const BackpropParams& p) {
    BackpropVerificationResult res;
    res.params = p;

    // Initial forward pass
    res.initial_a = p.w * p.x + p.b;
    res.initial_h = std::tanh(res.initial_a);
    res.initial_y_hat = p.v * res.initial_h + p.c;
    res.initial_loss = 0.5 * (res.initial_y_hat - p.y_target) * (res.initial_y_hat - p.y_target);

    // Gradient check with epsilon = 1e-6 and threshold = 1e-5
    res.gradient_checks = check_gradients_finite_difference(p, 1e-6, 1e-5);
    res.all_checks_passed = true;
    for (const auto& chk : res.gradient_checks) {
        if (!chk.passed) res.all_checks_passed = false;
    }

    // Run 100 training steps with simultaneous gradient descent updates
    double cur_w = p.w;
    double cur_b = p.b;
    double cur_v = p.v;
    double cur_c = p.c;

    res.history.reserve(p.steps + 1);

    for (int step = 0; step <= p.steps; ++step) {
        double gw = 0.0, gb = 0.0, gv = 0.0, gc = 0.0;
        double h = 0.0, y_hat = 0.0, loss = 0.0;
        compute_analytical_gradients(p.x, p.y_target, cur_w, cur_b, cur_v, cur_c, gw, gb, gv, gc, h, y_hat, loss);

        BackpropStep bs;
        bs.step = step;
        bs.w = cur_w;
        bs.b = cur_b;
        bs.v = cur_v;
        bs.c = cur_c;
        bs.h = h;
        bs.y_hat = y_hat;
        bs.loss = loss;
        bs.grad_w = gw;
        bs.grad_b = gb;
        bs.grad_v = gv;
        bs.grad_c = gc;
        res.history.push_back(bs);

        // Simultaneous gradient update: theta = theta - lr * grad
        if (step < p.steps) {
            cur_w -= p.lr * gw;
            cur_b -= p.lr * gb;
            cur_v -= p.lr * gv;
            cur_c -= p.lr * gc;
        }
    }

    res.final_loss = res.history.back().loss;
    return res;
}

std::string BackpropVerificationResult::to_json() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << "{\"config\":{"
       << "\"x\":" << params.x << ",\"yTarget\":" << params.y_target << ","
       << "\"w\":" << params.w << ",\"b\":" << params.b << ","
       << "\"v\":" << params.v << ",\"c\":" << params.c << ","
       << "\"lr\":" << params.lr << ",\"steps\":" << params.steps << "},"
       << "\"forwardInitial\":{"
       << "\"a\":" << initial_a << ",\"h\":" << initial_h << ","
       << "\"yHat\":" << initial_y_hat << ",\"loss\":" << initial_loss << "},"
       << "\"finalLoss\":" << final_loss << ","
       << "\"allChecksPassed\":" << (all_checks_passed ? "true" : "false") << ","
       << "\"gradientChecks\":[";

    for (size_t i = 0; i < gradient_checks.size(); ++i) {
        const auto& g = gradient_checks[i];
        ss << "{\"paramName\":\"" << g.param_name << "\","
           << "\"analyticalGrad\":" << g.analytical_grad << ","
           << "\"numericalGrad\":" << g.numerical_grad << ","
           << "\"absoluteError\":" << g.absolute_error << ","
           << "\"passed\":" << (g.passed ? "true" : "false") << "}";
        if (i < gradient_checks.size() - 1) ss << ",";
    }

    ss << "],\"trainingHistory\":[";
    for (size_t i = 0; i < history.size(); ++i) {
        const auto& s = history[i];
        ss << "{\"step\":" << s.step << ",\"w\":" << s.w << ",\"b\":" << s.b
           << ",\"v\":" << s.v << ",\"c\":" << s.c << ",\"h\":" << s.h
           << ",\"yHat\":" << s.y_hat << ",\"loss\":" << s.loss
           << ",\"gradW\":" << s.grad_w << ",\"gradB\":" << s.grad_b
           << ",\"gradV\":" << s.grad_v << ",\"gradC\":" << s.grad_c << "}";
        if (i < history.size() - 1) ss << ",";
    }

    ss << "]}";
    return ss.str();
}

std::string BackpropVerificationResult::to_csv() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(8);
    ss << "# PASS SCIENCE WORKS - Backpropagation 100-Step Training History\n";
    ss << "# Sample: x=" << params.x << ", y=" << params.y_target << ", lr=" << params.lr << "\n";
    ss << "step,w,b,v,c,h,y_hat,loss,grad_w,grad_b,grad_v,grad_c\n";
    for (const auto& s : history) {
        ss << s.step << "," << s.w << "," << s.b << "," << s.v << "," << s.c << ","
           << s.h << "," << s.y_hat << "," << s.loss << ","
           << s.grad_w << "," << s.grad_b << "," << s.grad_v << "," << s.grad_c << "\n";
    }
    return ss.str();
}

} // namespace workbench
