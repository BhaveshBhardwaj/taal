#include "transit_tourism/cool_corridor_router.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace taal::transit_tourism {

CoolCorridorRouter::CoolCorridorRouter() {
    // Shaded Heritage Covered Katras & Arcades
    segments_.push_back({
        "Red Fort Lahore Gate",
        "Chatta Chowk Covered Mughal Arcade",
        180.0, 1.00, true, "17th-century vaulted bazaar ceiling with cross-ventilation"
    });
    segments_.push_back({
        "Chatta Chowk Covered Mughal Arcade",
        "Dariba Kalan Covered Jewelry Arcade",
        260.0, 0.85, true, "Narrow haveli shaded corridor with attar perfumers"
    });
    segments_.push_back({
        "Dariba Kalan Covered Jewelry Arcade",
        "Kinari Bazaar Shaded Katras",
        220.0, 0.90, false, "Cloth canopy shading with continuous passive cooling"
    });
    segments_.push_back({
        "Kinari Bazaar Shaded Katras",
        "Chandni Chowk Metro Gate 1",
        240.0, 0.80, true, "Misted pedestrian colonnade"
    });
}

void CoolCorridorRouter::add_segment(const WalkingSegment& segment) {
    segments_.push_back(segment);
}

CoolRoutePlan CoolCorridorRouter::plan_cool_corridor(const std::string& /*start_node*/,
                                                    const std::string& /*end_node*/,
                                                    double ambient_temp_c) const {
    double total_dist = 0.0;
    double weighted_shade = 0.0;
    std::vector<std::string> landmarks;

    for (const auto& s : segments_) {
        total_dist += s.distance_meters;
        weighted_shade += s.distance_meters * s.shade_percentage;
        landmarks.push_back(s.from_landmark);
    }
    landmarks.push_back("Chandni Chowk Metro Gate 1");

    double avg_shade = (total_dist > 0) ? (weighted_shade / total_dist) : 0.85;
    // Delhi katra microclimate: 5.5°C cooling from masonry shade + 1.0°C misting
    double temp_reduction = (avg_shade * 5.5) + 0.8;
    double perceived_temp = ambient_temp_c - temp_reduction;

    int walk_mins = static_cast<int>(std::round(total_dist / 65.0)); // 65m per min leisurely walk
    double heat_risk = std::clamp((perceived_temp - 32.0) / 15.0, 0.0, 1.0);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "Cool-Corridor Plan: Shaded via Chatta Chowk & Dariba Kalan (" << total_dist
       << "m, " << walk_mins << " mins). Temperature reduced by -" << temp_reduction
       << "°C (Perceived: " << perceived_temp << "°C vs Ambient " << ambient_temp_c << "°C).";

    return {
        "Old Delhi Shaded Katra Heritage Cool-Corridor",
        total_dist,
        perceived_temp,
        temp_reduction,
        walk_mins,
        heat_risk,
        landmarks,
        ss.str()
    };
}

CoolRoutePlan CoolCorridorRouter::plan_standard_open_route(const std::string& /*start_node*/,
                                                          const std::string& /*end_node*/,
                                                          double ambient_temp_c) const {
    // Open road along Netaji Subhash Marg: 0% shade, black asphalt radiative heating (+4°C)
    double dist = 750.0;
    double perceived = ambient_temp_c + 3.5; // Asphalt radiant heat
    int walk_mins = 11;
    double heat_risk = std::clamp((perceived - 32.0) / 15.0, 0.0, 1.0);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "Open Asphalt Route: Direct along Netaji Subhash Marg (750m, 11 mins). "
       << "Zero shade! Perceived heat: " << perceived << "°C (+3.5°C asphalt radiation). "
       << "HIGH HEATSTROKE RISK!";

    return {
        "Direct Open Arterial (Netaji Subhash Marg)",
        dist,
        perceived,
        0.0,
        walk_mins,
        heat_risk,
        {"Red Fort Lahore Gate", "Netaji Subhash Marg Open Road", "Chandni Chowk Metro Gate 5"},
        ss.str()
    };
}

} // namespace taal::transit_tourism

