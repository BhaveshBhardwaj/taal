#pragma once

#include "core/types.hpp"
#include <vector>
#include <string>
#include <deque>

namespace taal::forecast {

struct EarlyWarningAlert {
    int64_t timestamp_sec;
    std::string location;
    double current_value;
    double rolling_variance;
    double lag1_autocorrelation;
    double csd_index;
    bool is_critical_slowing_down;
    std::string warning_message;
};

class CriticalSlowingDown {
public:
    explicit CriticalSlowingDown(size_t window_size = 24, double baseline_variance = 5.0);

    // Push new queue length / entry rate observation
    void record_observation(int64_t timestamp_sec, double value, const std::string& location = "Red Fort Lahore Gate");

    // Clear buffer
    void reset();

    // Compute CSD indicators on current window
    EarlyWarningAlert evaluate_current_state() const;

    // Run batch analysis on a complete time-series and return detected warning points
    std::vector<EarlyWarningAlert> analyze_series(const std::vector<int64_t>& timestamps,
                                                 const std::vector<double>& values,
                                                 const std::string& location = "corridor") const;

private:
    size_t window_size_;
    double baseline_variance_;
    std::deque<std::pair<int64_t, double>> history_;
    std::string last_location_;
};

} // namespace taal::forecast

