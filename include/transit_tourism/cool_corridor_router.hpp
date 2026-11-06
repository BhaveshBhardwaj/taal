#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::transit_tourism {

struct WalkingSegment {
    std::string from_landmark;
    std::string to_landmark;
    double distance_meters;
    double shade_percentage;     // 0.0 (open asphalt) to 1.0 (fully covered arcade)
    bool has_evaporative_cooling; // Water misting or fountain microclimate
    std::string heritage_vibe;
};

struct CoolRoutePlan {
    std::string route_name;
    double total_distance_meters;
    double perceived_temperature_c;
    double temperature_reduction_c;
    int est_walk_minutes;
    double heatstroke_risk_score; // 0.0 (safe) to 1.0 (dangerous)
    std::vector<std::string> landmarks_en_route;
    std::string route_guidance;
};

class CoolCorridorRouter {
public:
    CoolCorridorRouter();

    void add_segment(const WalkingSegment& segment);

    // Compute route comparing Direct Open Road vs Shaded Heritage Cool Corridor
    CoolRoutePlan plan_cool_corridor(const std::string& start_node,
                                     const std::string& end_node,
                                     double ambient_temp_c) const;

    CoolRoutePlan plan_standard_open_route(const std::string& start_node,
                                           const std::string& end_node,
                                           double ambient_temp_c) const;

private:
    std::vector<WalkingSegment> segments_;
};

} // namespace taal::transit_tourism

