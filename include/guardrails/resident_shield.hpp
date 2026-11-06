#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace taal::guardrails {

struct LaneStatus {
    std::string lane_id;
    std::string name;
    bool is_residential_zone;
    double current_noise_db;
    double max_safe_noise_db;
    double footfall_density_p_m2;
    double max_safe_density;
    double parking_saturation_pct;
    double feeder_loading_pct;
};

struct VetoDecision {
    bool is_vetoed;
    std::string reason;
    std::string violated_constraint;
    double resident_burden_score; // Must be <= 0.0 for approval
};

class ResidentShield {
public:
    ResidentShield();

    void register_lane(const LaneStatus& lane);
    void update_lane_telemetry(const std::string& lane_id, double noise_db, double density, double parking_pct, double feeder_pct);

    // Evaluate proposed nudge or routing change
    // Auto-vetoes if it would increase noise/footfall in any residential alley or after 22:00 (night hours)
    VetoDecision evaluate_nudge(const std::string& target_lane_id,
                                double projected_added_footfall,
                                int64_t current_time_sec) const;

    const std::vector<LaneStatus>& get_all_lanes() const { return lanes_; }

private:
    std::vector<LaneStatus> lanes_;
};

} // namespace taal::guardrails

