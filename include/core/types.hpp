#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <numbers>

namespace taal {

constexpr double PI = 3.14159265358979323846;
constexpr double TWO_PI = 2.0 * PI;
constexpr int64_t SECONDS_PER_DAY = 86400;

inline double seconds_to_phase(int64_t seconds_from_midnight) {
    int64_t rem = seconds_from_midnight % SECONDS_PER_DAY;
    if (rem < 0) rem += SECONDS_PER_DAY;
    return (static_cast<double>(rem) / static_cast<double>(SECONDS_PER_DAY)) * TWO_PI;
}

inline int64_t phase_to_seconds(double phase) {
    while (phase < 0.0) phase += TWO_PI;
    while (phase >= TWO_PI) phase -= TWO_PI;
    return static_cast<int64_t>((phase / TWO_PI) * SECONDS_PER_DAY);
}

inline std::string format_time_hhmmss(int64_t seconds_from_midnight) {
    int64_t rem = seconds_from_midnight % SECONDS_PER_DAY;
    if (rem < 0) rem += SECONDS_PER_DAY;
    int h = static_cast<int>(rem / 3600);
    int m = static_cast<int>((rem % 3600) / 60);
    int s = static_cast<int>(rem % 60);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
    return std::string(buf);
}

struct RegressionMetrics {
    double r_squared{0.0};
    double rmse{0.0};
    double mae{0.0};
    std::vector<double> coefficients;
};

inline RegressionMetrics fit_linear_regression_2d(const std::vector<double>& x1,
                                                 const std::vector<double>& y) {
    size_t n = x1.size();
    if (n < 2 || x1.size() != y.size()) {
        return {0.0, 0.0, 0.0, {0.0, 0.0}};
    }
    double sum_x = 0.0, sum_y = 0.0, sum_xx = 0.0, sum_xy = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum_x += x1[i];
        sum_y += y[i];
        sum_xx += x1[i] * x1[i];
        sum_xy += x1[i] * y[i];
    }
    double denom = n * sum_xx - sum_x * sum_x;
    if (std::abs(denom) < 1e-9) {
        double y_mean = sum_y / n;
        return {0.0, 0.0, 0.0, {y_mean, 0.0}};
    }
    double slope = (n * sum_xy - sum_x * sum_y) / denom;
    double intercept = (sum_y - slope * sum_x) / n;

    double y_mean = sum_y / n;
    double ss_tot = 0.0, ss_res = 0.0, abs_err = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double y_pred = intercept + slope * x1[i];
        double diff = y[i] - y_pred;
        ss_res += diff * diff;
        abs_err += std::abs(diff);
        ss_tot += (y[i] - y_mean) * (y[i] - y_mean);
    }
    double r2 = (ss_tot > 1e-9) ? (1.0 - (ss_res / ss_tot)) : 0.0;
    if (r2 < 0.0) r2 = 0.0;
    double rmse = std::sqrt(ss_res / n);
    double mae = abs_err / n;
    return {r2, rmse, mae, {intercept, slope}};
}

// Multi-variable regression: y = beta0 + beta1*x1 + beta2*x2 + beta3*x3 using normal equations
inline RegressionMetrics fit_multivariate_regression(const std::vector<std::vector<double>>& X,
                                                    const std::vector<double>& y) {
    size_t n = y.size();
    if (n == 0 || X.empty() || X[0].size() != n) {
        return {0.0, 0.0, 0.0, {}};
    }
    size_t k = X.size(); // number of features
    size_t p = k + 1;    // features + intercept

    // Build normal equations: (X_mat^T * X_mat) * beta = X_mat^T * y
    std::vector<std::vector<double>> A(p, std::vector<double>(p, 0.0));
    std::vector<double> B(p, 0.0);

    for (size_t row = 0; row < n; ++row) {
        std::vector<double> xi(p);
        xi[0] = 1.0;
        for (size_t j = 0; j < k; ++j) {
            xi[j + 1] = X[j][row];
        }
        for (size_t i = 0; i < p; ++i) {
            B[i] += xi[i] * y[row];
            for (size_t j = 0; j < p; ++j) {
                A[i][j] += xi[i] * xi[j];
            }
        }
    }

    // Gaussian elimination with partial pivoting
    for (size_t i = 0; i < p; ++i) {
        size_t pivot = i;
        for (size_t j = i + 1; j < p; ++j) {
            if (std::abs(A[j][i]) > std::abs(A[pivot][i])) {
                pivot = j;
            }
        }
        std::swap(A[i], A[pivot]);
        std::swap(B[i], B[pivot]);

        if (std::abs(A[i][i]) < 1e-12) {
            A[i][i] = 1e-12; // regularize
        }

        for (size_t j = i + 1; j < p; ++j) {
            double factor = A[j][i] / A[i][i];
            for (size_t col = i; col < p; ++col) {
                A[j][col] -= factor * A[i][col];
            }
            B[j] -= factor * B[i];
        }
    }

    // Back substitution
    std::vector<double> beta(p, 0.0);
    for (int i = static_cast<int>(p) - 1; i >= 0; --i) {
        double sum = B[i];
        for (size_t j = i + 1; j < p; ++j) {
            sum -= A[i][j] * beta[j];
        }
        beta[i] = sum / A[i][i];
    }

    // Metrics
    double y_mean = std::accumulate(y.begin(), y.end(), 0.0) / n;
    double ss_tot = 0.0, ss_res = 0.0, abs_err = 0.0;
    for (size_t row = 0; row < n; ++row) {
        double y_pred = beta[0];
        for (size_t j = 0; j < k; ++j) {
            y_pred += beta[j + 1] * X[j][row];
        }
        double diff = y[row] - y_pred;
        ss_res += diff * diff;
        abs_err += std::abs(diff);
        ss_tot += (y[row] - y_mean) * (y[row] - y_mean);
    }
    double r2 = (ss_tot > 1e-9) ? (1.0 - (ss_res / ss_tot)) : 0.0;
    if (r2 < 0.0) r2 = 0.0;
    return {r2, std::sqrt(ss_res / n), abs_err / n, beta};
}

} // namespace taal

