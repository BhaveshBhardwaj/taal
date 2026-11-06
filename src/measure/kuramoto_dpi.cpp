#include "measure/kuramoto_dpi.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <random>

namespace taal::measure {

KuramotoOrderParameter KuramotoDPI::compute(const std::vector<double>& phases) {
    if (phases.empty()) {
        return {0.0, 0.0, 0, 0};
    }
    double sum_cos = 0.0;
    double sum_sin = 0.0;
    for (double phi : phases) {
        sum_cos += std::cos(phi);
        sum_sin += std::sin(phi);
    }
    double n = static_cast<double>(phases.size());
    double mean_cos = sum_cos / n;
    double mean_sin = sum_sin / n;
    double r = std::sqrt(mean_cos * mean_cos + mean_sin * mean_sin);
    if (r > 1.0) r = 1.0;

    double psi = std::atan2(mean_sin, mean_cos);
    if (psi < 0.0) psi += TWO_PI;

    int64_t mean_time = phase_to_seconds(psi);
    return {r, psi, mean_time, phases.size()};
}

KuramotoOrderParameter KuramotoDPI::compute_from_timestamps(const std::vector<int64_t>& timestamps) {
    std::vector<double> phases;
    phases.reserve(timestamps.size());
    for (int64_t ts : timestamps) {
        phases.push_back(seconds_to_phase(ts));
    }
    return compute(phases);
}

void KuramotoDPI::add_decision(const DecisionSample& sample) {
    samples_.push_back(sample);
}

void KuramotoDPI::add_decision(int64_t timestamp_sec, const std::string& node_id, const std::string& type) {
    samples_.push_back({timestamp_sec, node_id, type});
}

void KuramotoDPI::clear() {
    samples_.clear();
}

KuramotoOrderParameter KuramotoDPI::compute_window(int64_t t_start_sec, int64_t t_end_sec,
                                                  const std::string& filter_node,
                                                  const std::string& filter_type) const {
    std::vector<double> phases;
    for (const auto& s : samples_) {
        if (s.timestamp_sec >= t_start_sec && s.timestamp_sec <= t_end_sec) {
            if (!filter_node.empty() && s.node_id != filter_node) continue;
            if (!filter_type.empty() && s.decision_type != filter_type) continue;
            phases.push_back(seconds_to_phase(s.timestamp_sec));
        }
    }
    return compute(phases);
}

SyncVsVolumeResult KuramotoDPI::run_sync_vs_volume_test(const std::vector<double>& volume_series,
                                                       const std::vector<double>& dpi_series,
                                                       const std::vector<double>& observed_stress) {
    size_t n = volume_series.size();
    if (n < 4 || dpi_series.size() != n || observed_stress.size() != n) {
        return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false, "Insufficient data for regression comparison"};
    }

    // Model A: Volume only
    auto res_a = fit_linear_regression_2d(volume_series, observed_stress);

    // Model B: Volume, DPI, and interaction (Volume * DPI)
    std::vector<double> interaction(n);
    for (size_t i = 0; i < n; ++i) {
        interaction[i] = volume_series[i] * dpi_series[i];
    }
    std::vector<std::vector<double>> X_b = {volume_series, dpi_series, interaction};
    auto res_b = fit_multivariate_regression(X_b, observed_stress);

    double r2_delta = res_b.r_squared - res_a.r_squared;
    double rmse_reduction = (res_a.rmse > 1e-9) ? ((res_a.rmse - res_b.rmse) / res_a.rmse) * 100.0 : 0.0;
    // Hypothesis confirmed if Model B reduces error significantly (>20%) and improves R^2
    bool confirmed = (res_b.r_squared > res_a.r_squared + 0.04) && (rmse_reduction >= 20.0);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4);
    ss << "Sync-vs-Volume Test:\n"
       << "  Model A (Volume-Only):     R2 = " << res_a.r_squared << ", RMSE = " << res_a.rmse << "\n"
       << "  Model B (Volume + DPI):    R2 = " << res_b.r_squared << ", RMSE = " << res_b.rmse << "\n"
       << "  Delta R2: +" << r2_delta << " (" << std::setprecision(1) << rmse_reduction << "% error reduction)\n"
       << "  Hypothesis: " << (confirmed ? "PROVED (Stress strongly driven by phase coherence R(t))"
                                      : "Inconclusive / Weak coherence effect");

    return {res_a.r_squared, res_a.rmse, res_b.r_squared, res_b.rmse, r2_delta, rmse_reduction, confirmed, ss.str()};
}

void KuramotoDPI::generate_synthetic_validation_dataset(size_t sample_hours,
                                                        std::vector<double>& out_volume,
                                                        std::vector<double>& out_dpi,
                                                        std::vector<double>& out_stress) {
    out_volume.clear();
    out_dpi.clear();
    out_stress.clear();

    std::mt19937 rng(42);
    std::normal_distribution<double> noise(0.0, 1.5);

    for (size_t h = 0; h < sample_hours; ++h) {
        int hour_of_day = static_cast<int>(h % 24);
        double volume = 2000.0;
        double dpi = 0.20;

        // Morning spread: 8,000 visitors distributed across several hours (DPI = 0.25)
        if (hour_of_day >= 8 && hour_of_day <= 11) {
            volume = 6500.0;
            dpi = 0.28;
        }
        // Afternoon heat lull: lower volume (1,200), low DPI
        else if (hour_of_day >= 13 && hour_of_day <= 15) {
            volume = 1200.0;
            dpi = 0.18;
        }
        // Evening show exit unison: moderate volume (3,800), but extreme lockstep DPI (0.94)
        else if (hour_of_day >= 20 && hour_of_day <= 22) {
            volume = 3800.0;
            dpi = 0.94; // Extreme phase coherence!
        }

        // Stress is fundamentally phase-coherence driven:
        // High volume spread out (6500 * 0.28^2 = 509) vs moderate volume locked (3800 * 0.94^2 = 3357)
        double physical_stress = 8.0 + (0.001 * volume) + (35.0 * dpi) + (0.015 * volume * dpi * dpi) + noise(rng);
        if (physical_stress < 0.0) physical_stress = 0.0;

        out_volume.push_back(volume);
        out_dpi.push_back(dpi);
        out_stress.push_back(physical_stress);
    }
}

} // namespace taal::measure

