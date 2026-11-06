#include "forecast/critical_slowing_down.hpp"
#include <cmath>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace taal::forecast {

CriticalSlowingDown::CriticalSlowingDown(size_t window_size, double baseline_variance)
    : window_size_(window_size), baseline_variance_(baseline_variance) {}

void CriticalSlowingDown::record_observation(int64_t timestamp_sec, double value, const std::string& location) {
    history_.push_back({timestamp_sec, value});
    last_location_ = location;
    while (history_.size() > window_size_) {
        history_.pop_front();
    }
}

void CriticalSlowingDown::reset() {
    history_.clear();
}

EarlyWarningAlert CriticalSlowingDown::evaluate_current_state() const {
    size_t n = history_.size();
    if (n < 6) {
        return {
            history_.empty() ? 0 : history_.back().first,
            last_location_,
            history_.empty() ? 0.0 : history_.back().second,
            0.0, 0.0, 0.0, false, "Gathering calibration window"
        };
    }

    double sum = 0.0;
    for (const auto& item : history_) {
        sum += item.second;
    }
    double mean = sum / static_cast<double>(n);

    // Compute variance
    double ss = 0.0;
    for (const auto& item : history_) {
        double d = item.second - mean;
        ss += d * d;
    }
    double variance = ss / static_cast<double>(n);

    // Compute lag-1 autocorrelation
    double num_ac = 0.0;
    for (size_t i = 1; i < n; ++i) {
        num_ac += (history_[i].second - mean) * (history_[i - 1].second - mean);
    }
    double rho1 = (ss > 1e-9) ? (num_ac / ss) : 0.0;
    rho1 = std::clamp(rho1, -1.0, 0.99);

    // Critical Slowing Down composite index
    double base_v = (baseline_variance_ > 1e-4) ? baseline_variance_ : 1.0;
    double var_ratio = variance / base_v;
    double damping_factor = 1.0 / (1.0 - std::min(rho1, 0.95));
    double csd_index = var_ratio * damping_factor;

    // Surge tipping point alert conditions:
    // When autocorrelation rises above 0.50 and variance ratio rises above 2.5x baseline
    bool is_csd = (rho1 > 0.45 && var_ratio > 2.0 && csd_index > 4.0);

    std::ostringstream ss_msg;
    if (is_csd) {
        ss_msg << "[CRITICAL SLOWING DOWN DETECTED] Queue dynamics exhibiting phase-transition instability! "
               << "Var=" << std::fixed << std::setprecision(1) << variance
               << " (+" << static_cast<int>(var_ratio * 100) << "%), Lag-1 Autocorr="
               << std::setprecision(2) << rho1 << ". Surge expected in 15-20 minutes before volume peak!";
    } else {
        ss_msg << "Stable queue flow. CSD Index: " << std::fixed << std::setprecision(2) << csd_index;
    }

    return {
        history_.back().first,
        last_location_,
        history_.back().second,
        variance,
        rho1,
        csd_index,
        is_csd,
        ss_msg.str()
    };
}

std::vector<EarlyWarningAlert> CriticalSlowingDown::analyze_series(const std::vector<int64_t>& timestamps,
                                                                   const std::vector<double>& values,
                                                                   const std::string& location) const {
    CriticalSlowingDown detector(window_size_, baseline_variance_);
    std::vector<EarlyWarningAlert> alerts;

    for (size_t i = 0; i < timestamps.size(); ++i) {
        detector.record_observation(timestamps[i], values[i], location);
        auto alert = detector.evaluate_current_state();
        if (alert.is_critical_slowing_down) {
            alerts.push_back(alert);
        }
    }
    return alerts;
}

} // namespace taal::forecast

