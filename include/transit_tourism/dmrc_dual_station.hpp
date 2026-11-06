#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::transit_tourism {

struct DmrcStationState {
    std::string station_id;
    std::string name;
    std::string metro_line;       // "Yellow Line" or "Violet Line"
    double platform_density_p_m2; // Current passenger density
    double max_safe_density_p_m2; // 2.5 persons/m2 safe ceiling
    double headway_minutes;       // Train interval
    double walk_distance_meters;  // Distance from Red Fort exit
    int queue_wait_seconds;
};

struct TransitSplitRecommendation {
    std::string recommended_station_id;
    std::string recommended_station_name;
    double split_percentage_to_violet;
    int phase_credits_incentive;
    std::string passenger_guidance_en;
    std::string passenger_guidance_hi;
    bool is_crush_mitigation_active;
};

class DmrcDualStation {
public:
    DmrcDualStation();

    void update_station_telemetry(const std::string& station_id,
                                  double platform_density,
                                  double headway_min,
                                  int queue_wait_sec);

    // Compute dynamic load splitting between Yellow Line (Chandni Chowk) and Violet Line (Lal Qila)
    TransitSplitRecommendation evaluate_transit_split(int incoming_exit_crowd) const;

    const DmrcStationState& get_yellow_station() const { return yellow_station_; }
    const DmrcStationState& get_violet_station() const { return violet_station_; }

private:
    DmrcStationState yellow_station_; // Chandni Chowk
    DmrcStationState violet_station_; // Lal Qila
};

} // namespace taal::transit_tourism

