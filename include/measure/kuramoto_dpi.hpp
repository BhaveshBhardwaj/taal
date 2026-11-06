#pragma once

#include "core/types.hpp"
#include <vector>
#include <string>
#include <complex>
#include <utility>

namespace taal::measure {

struct KuramotoOrderParameter {
    double r{0.0};              // Order parameter: 0 = completely scattered, 1 = perfect lockstep
    double mean_phase{0.0};     // Collective phase angle Psi in radians [0, 2*PI)
    int64_t mean_time_sec{0};   // Time of day corresponding to Psi
    size_t sample_size{0};      // Number of decision phases included
};

struct DecisionSample {
    int64_t timestamp_sec;      // Unix timestamp or seconds from midnight
    std::string node_id;        // e.g. "red_fort_gate", "chandni_chowk_metro", "e_rickshaw_stand_1"
    std::string decision_type;  // e.g. "enter", "queue", "exit", "charge", "checkout"
};

struct SyncVsVolumeResult {
    double r2_volume_only{0.0};
    double rmse_volume_only{0.0};
    double r2_volume_and_dpi{0.0};
    double rmse_volume_and_dpi{0.0};
    double r2_delta{0.0};        // r2_volume_and_dpi - r2_volume_only
    double rmse_reduction_pct{0.0};
    bool hypothesis_confirmed{false};
    std::string summary_proof;
};

class KuramotoDPI {
public:
    KuramotoDPI() = default;

    // Direct calculation from a collection of phases in [0, 2*PI)
    static KuramotoOrderParameter compute(const std::vector<double>& phases);

    // Compute from timestamps (seconds from midnight or epoch)
    static KuramotoOrderParameter compute_from_timestamps(const std::vector<int64_t>& timestamps);

    // Add decision observation to internal buffer
    void add_decision(const DecisionSample& sample);
    void add_decision(int64_t timestamp_sec, const std::string& node_id, const std::string& type);

    // Clear buffer
    void clear();

    // Compute DPI over a specific time window [t_start, t_end]
    KuramotoOrderParameter compute_window(int64_t t_start_sec, int64_t t_end_sec,
                                          const std::string& filter_node = "",
                                          const std::string& filter_type = "") const;

    // Run the hypothesis proof: Sync-vs-Volume comparison
    // Compares Model A (volume only) vs Model B (volume + DPI + interaction)
    static SyncVsVolumeResult run_sync_vs_volume_test(const std::vector<double>& volume_series,
                                                     const std::vector<double>& dpi_series,
                                                     const std::vector<double>& observed_stress);

    // Generate benchmark synthetic test data matching Delhi corridor characteristics
    static void generate_synthetic_validation_dataset(size_t sample_hours,
                                                      std::vector<double>& out_volume,
                                                      std::vector<double>& out_dpi,
                                                      std::vector<double>& out_stress);

private:
    std::vector<DecisionSample> samples_;
};

} // namespace taal::measure

