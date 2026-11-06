#include "desync/polyrhythm_menu.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>

namespace taal::desync {

PolyrhythmMenu::PolyrhythmMenu() : rng_(42) {
    // Seed canonical Delhi alternatives
    add_destination({
        "red_fort",
        "Red Fort (Lal Qila)",
        "mughal_heritage",
        4.6, 120.0, 0.95, 0.90, 800, 0
    });
    add_destination({
        "humayuns_tomb",
        "Humayun's Tomb Mughal Gardens",
        "mughal_heritage",
        4.7, 105.0, 0.96, 0.35, 600, 0
    });
    add_destination({
        "sunder_nursery",
        "Sunder Nursery Heritage Biodiversity Park",
        "nature_gardens",
        4.8, 130.0, 0.98, 0.40, 750, 0
    });
    add_destination({
        "purana_qila",
        "Purana Qila & Lake Boating",
        "mughal_heritage",
        4.5, 90.0, 0.88, 0.30, 500, 0
    });
    add_destination({
        "lodhi_garden",
        "Lodhi Garden Historic Domes",
        "nature_gardens",
        4.7, 85.0, 0.92, 0.45, 650, 0
    });
    add_destination({
        "safdarjung_tomb",
        "Safdarjung Tomb Twilight Gardens",
        "mughal_heritage",
        4.4, 75.0, 0.86, 0.25, 450, 0
    });
}

void PolyrhythmMenu::add_destination(const DestinationNode& node) {
    destinations_.push_back(node);
}

double PolyrhythmMenu::compute_equal_delight_score(const DestinationNode& ref,
                                                   const DestinationNode& cand) {
    // Rating parity (out of 5.0): weight 0.35
    double diff_rating = std::abs(ref.rating_5_star - cand.rating_5_star);
    double score_rating = 1.0 - (diff_rating / 5.0);

    // Dwell parity: weight 0.35
    double max_dwell = std::max(ref.avg_dwell_minutes, cand.avg_dwell_minutes);
    double score_dwell = (max_dwell > 0) ? (1.0 - (std::abs(ref.avg_dwell_minutes - cand.avg_dwell_minutes) / max_dwell)) : 1.0;

    // Aesthetic & vibe affinity: weight 0.30
    double score_aesthetic = 1.0 - std::abs(ref.aesthetic_score - cand.aesthetic_score);

    double eds = (0.35 * score_rating) + (0.35 * score_dwell) + (0.30 * score_aesthetic);
    return std::clamp(eds, 0.0, 1.0);
}

std::vector<TwinMomentOption> PolyrhythmMenu::generate_twin_moments(const std::string& target_id) const {
    const DestinationNode* ref = nullptr;
    for (const auto& d : destinations_) {
        if (d.id == target_id) {
            ref = &d;
            break;
        }
    }
    if (!ref) return {};

    std::vector<TwinMomentOption> options;
    for (const auto& d : destinations_) {
        if (d.id == target_id) continue; // skip self

        double eds = compute_equal_delight_score(*ref, d);
        if (eds >= 0.78) { // Strict delight threshold
            std::string time_win = "17:30 - 19:30 (Golden Hour Window)";
            std::string perk = "Sunset illumination + tranquil crowd density";
            if (d.id == "sunder_nursery") {
                perk = "Mughal water-canals, artisanal café, zero gate queues";
            } else if (d.id == "humayuns_tomb") {
                perk = "Charbagh garden twilight walk with illuminated sandstone arches";
            } else if (d.id == "purana_qila") {
                perk = "Ancient ramparts walk + evening lake breeze";
            } else if (d.id == "lodhi_garden") {
                perk = "15th-century Sayyid & Lodi architectural walk in lush greenery";
            }

            options.push_back({d, eds, time_win, perk});
        }
    }

    // Sort by Equal Delight Score descending
    std::sort(options.begin(), options.end(), [](const TwinMomentOption& a, const TwinMomentOption& b) {
        return a.equal_delight_score > b.equal_delight_score;
    });

    return options;
}

TwinMomentOption PolyrhythmMenu::draw_lottery_suggestion(const std::string& target_id,
                                                         double diversity_floor,
                                                         uint32_t seed) {
    if (seed != 0) rng_.seed(seed);

    auto candidates = generate_twin_moments(target_id);
    if (candidates.empty()) {
        throw std::runtime_error("No available twin moments for target");
    }

    size_t k = candidates.size();
    std::vector<double> weights(k, 0.0);

    // Calculate quota-adjusted weight for each candidate
    for (size_t i = 0; i < k; ++i) {
        // Find destination in internal list to check capacity
        int rem_quota = 100;
        for (const auto& d : destinations_) {
            if (d.id == candidates[i].destination.id) {
                rem_quota = std::max(0, d.max_quota_per_hour - d.allocated_this_hour);
                break;
            }
        }

        double eds = candidates[i].equal_delight_score;
        double base_prob = std::exp(eds * 2.0) * (static_cast<double>(rem_quota) / 100.0);
        weights[i] = base_prob;
    }

    // Apply hard diversity floor so no single node receives all recommendations
    double sum_w = 0.0;
    for (double w : weights) sum_w += w;
    if (sum_w < 1e-6) {
        for (size_t i = 0; i < k; ++i) weights[i] = 1.0 / static_cast<double>(k);
    } else {
        for (size_t i = 0; i < k; ++i) {
            double normalized = weights[i] / sum_w;
            weights[i] = std::max(diversity_floor, normalized);
        }
    }

    // Stochastic draw
    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    size_t chosen_idx = dist(rng_);

    // Increment allocation
    for (auto& d : destinations_) {
        if (d.id == candidates[chosen_idx].destination.id) {
            d.allocated_this_hour++;
            break;
        }
    }

    return candidates[chosen_idx];
}

std::vector<std::pair<std::string, double>> PolyrhythmMenu::get_allocation_distribution() const {
    int total = 0;
    for (const auto& d : destinations_) {
        total += d.allocated_this_hour;
    }
    std::vector<std::pair<std::string, double>> dist;
    for (const auto& d : destinations_) {
        double pct = (total > 0) ? (static_cast<double>(d.allocated_this_hour) / total) * 100.0 : 0.0;
        dist.push_back({d.name, pct});
    }
    return dist;
}

void PolyrhythmMenu::reset_hourly_quotas() {
    for (auto& d : destinations_) {
        d.allocated_this_hour = 0;
    }
}

} // namespace taal::desync

