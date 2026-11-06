#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <random>

namespace taal::desync {

struct DestinationNode {
    std::string id;
    std::string name;
    std::string category;      // "mughal_heritage", "nature_gardens", "bazaar_walk", "riverside"
    double rating_5_star;      // Verified visitor rating out of 5.0
    double avg_dwell_minutes;  // Mean dwell duration
    double aesthetic_score;    // 0.0 to 1.0
    double current_occupancy_pct;
    int max_quota_per_hour;
    int allocated_this_hour;
};

struct TwinMomentOption {
    DestinationNode destination;
    double equal_delight_score; // 0.0 to 1.0 (relative to target peak experience)
    std::string recommended_time_window;
    std::string highlight_perk; // e.g. "Sunset illumination + Rooftop Qawwali"
};

class PolyrhythmMenu {
public:
    PolyrhythmMenu();

    // Register a destination node
    void add_destination(const DestinationNode& node);

    // Compute Equal-Delight Score between reference target and candidate
    static double compute_equal_delight_score(const DestinationNode& reference,
                                             const DestinationNode& candidate);

    // Feature 8: Generate "Twin Moments" Polyrhythm Menu (4-6 equal alternatives)
    std::vector<TwinMomentOption> generate_twin_moments(const std::string& target_id) const;

    // Feature 9: Lottery Suggestions (quota-based stochastic anti-herding recommender)
    // Enforces diversity floor so no single node receives all traffic
    TwinMomentOption draw_lottery_suggestion(const std::string& target_id,
                                             double diversity_floor = 0.15,
                                             uint32_t seed = 0);

    // Check if quotas are balanced
    std::vector<std::pair<std::string, double>> get_allocation_distribution() const;

    void reset_hourly_quotas();

private:
    std::vector<DestinationNode> destinations_;
    std::mt19937 rng_;
};

} // namespace taal::desync

