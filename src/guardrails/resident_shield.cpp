#include "guardrails/resident_shield.hpp"
#include <sstream>

namespace taal::guardrails {

ResidentShield::ResidentShield() {
    // 1. Commercial / Arterial thoroughfares (tolerates higher transit)
    register_lane({
        "netaji_subhash_marg",
        "Netaji Subhash Marg (Main Concourse)",
        false, // Commercial arterial
        68.0, 85.0, 1.8, 4.0, 75.0, 65.0
    });
    register_lane({
        "chandni_chowk_pedestrian",
        "Chandni Chowk Main Pedestrian Mall",
        false, // Commercial pedestrian zone
        62.0, 80.0, 1.5, 3.5, 30.0, 70.0
    });

    // 2. Sensitive Residential Mohallas (Strict Shield Active)
    register_lane({
        "ballimaran_residential",
        "Gali Qasim Jan / Ballimaran (Residential Mohalla)",
        true, // Residential!
        51.0, 55.0, 0.4, 0.8, 60.0, 72.0
    });
    register_lane({
        "dariba_kalan_residential",
        "Dariba Kalan (Inner Residential Haveli Clusters)",
        true, // Residential!
        48.0, 53.0, 0.3, 0.7, 50.0, 68.0
    });
    register_lane({
        "kinari_bazaar_residential",
        "Kinari Bazaar Upper Residential Quarters",
        true, // Residential!
        50.0, 54.0, 0.4, 0.8, 45.0, 65.0
    });
}

void ResidentShield::register_lane(const LaneStatus& lane) {
    lanes_.push_back(lane);
}

void ResidentShield::update_lane_telemetry(const std::string& lane_id, double noise_db, double density, double parking_pct, double feeder_pct) {
    for (auto& l : lanes_) {
        if (l.lane_id == lane_id) {
            l.current_noise_db = noise_db;
            l.footfall_density_p_m2 = density;
            l.parking_saturation_pct = parking_pct;
            l.feeder_loading_pct = feeder_pct;
            break;
        }
    }
}

VetoDecision ResidentShield::evaluate_nudge(const std::string& target_lane_id,
                                            double projected_added_footfall,
                                            int64_t current_time_sec) const {
    const LaneStatus* lane = nullptr;
    for (const auto& l : lanes_) {
        if (l.lane_id == target_lane_id) {
            lane = &l;
            break;
        }
    }

    if (!lane) {
        return {true, "Target corridor lane not recognized in safety registry", "UNKNOWN_LANE", 1.0};
    }

    int64_t t_day = current_time_sec % SECONDS_PER_DAY;
    if (t_day < 0) t_day += SECONDS_PER_DAY;
    int hour = static_cast<int>(t_day / 3600);
    bool is_night_hour = (hour >= 22 || hour < 6);

    // Hard Rule 1: Auto-veto any nudge sending tourists into residential mohallas after 22:00
    if (lane->is_residential_zone && is_night_hour && projected_added_footfall > 0.0) {
        return {
            true,
            "AUTO-VETO: Night-hour curfew (post 22:00) active for residential mohalla " + lane->name,
            "NIGHT_HOURS_RESIDENTIAL_PROTECTION",
            2.5
        };
    }

    // Hard Rule 2: Auto-veto any nudge that increases load on residential lane beyond safe density
    if (lane->is_residential_zone) {
        double new_density = lane->footfall_density_p_m2 + (projected_added_footfall / 200.0);
        if (new_density > lane->max_safe_density) {
            return {
                true,
                "AUTO-VETO: Residential footfall density breach in " + lane->name + " (Projected: " + std::to_string(new_density).substr(0,4) + " p/m2 > Safe: " + std::to_string(lane->max_safe_density).substr(0,4) + ")",
                "RESIDENTIAL_DENSITY_EXCEEDED",
                1.8
            };
        }

        // Hard Rule 3: Auto-veto any noise spillover above 55 dB in residential zones
        double projected_noise = lane->current_noise_db + (projected_added_footfall * 0.05);
        if (projected_noise > lane->max_safe_noise_db) {
            return {
                true,
                "AUTO-VETO: Acoustic noise limit breach in residential lane " + lane->name,
                "RESIDENTIAL_ACOUSTIC_CEILING",
                1.5
            };
        }
    }

    // Hard Rule 4: Auto-veto if local feeder transformer exceeds 92% loading
    if (lane->feeder_loading_pct > 92.0 && projected_added_footfall > 50.0) {
        return {
            true,
            "AUTO-VETO: Feeder transformer at " + std::to_string(lane->feeder_loading_pct).substr(0,4) + "% loading threshold",
            "FEEDER_TRANSFORMER_STRESS",
            1.2
        };
    }

    // Passed all guardrails: Zero burden shifted to residents!
    return {
        false,
        "APPROVED: Nudge complies with Resident Shield (Zero residential burden shift)",
        "NONE",
        0.0
    };
}

} // namespace taal::guardrails

