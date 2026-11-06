#include "guardrails/regret_and_herding.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace taal::guardrails {

RegretBudget::RegretBudget(double max_regret_bound)
    : max_regret_bound_(max_regret_bound) {}

SwapBackPass RegretBudget::issue_nudge_guarantee(const std::string& user_id,
                                                 const std::string& original_site,
                                                 const std::string& alternate_site,
                                                 int64_t current_time_sec) {
    token_counter_++;
    std::string tid = "SWAPBACK-" + std::to_string(token_counter_);
    int64_t expiry = current_time_sec + 7200; // 2 hours validity

    SwapBackPass pass = {
        tid,
        user_id,
        original_site,
        alternate_site,
        current_time_sec,
        expiry,
        false,
        max_regret_bound_,
        "VIP-GATE-" + std::to_string(100 + (token_counter_ % 50))
    };

    active_tokens_[tid] = pass;
    return pass;
}

bool RegretBudget::redeem_swap_back(const std::string& token_id, int64_t current_time_sec) {
    auto it = active_tokens_.find(token_id);
    if (it == active_tokens_.end()) return false;
    if (it->second.is_redeemed || current_time_sec > it->second.expiry_sec) {
        return false;
    }
    it->second.is_redeemed = true;
    return true;
}

size_t RegretBudget::get_active_tokens_count() const {
    size_t count = 0;
    for (const auto& [_, p] : active_tokens_) {
        if (!p.is_redeemed) count++;
    }
    return count;
}

HerdingDetector::HerdingDetector(double hhi_threshold, double entropy_floor_margin)
    : hhi_threshold_(hhi_threshold), entropy_floor_margin_(entropy_floor_margin) {}

void HerdingDetector::record_adoption(const std::string& alternative_id) {
    adoptions_[alternative_id]++;
}

void HerdingDetector::reset() {
    adoptions_.clear();
}

HerdingState HerdingDetector::evaluate_herding() const {
    size_t k = adoptions_.size();
    if (k < 2) {
        return {0.0, 0.0, 0.0, false, "", "Insufficient alternative data points"};
    }

    int total = 0;
    for (const auto& [_, count] : adoptions_) {
        total += count;
    }

    if (total == 0) {
        return {0.0, 0.0, 0.0, false, "", "Zero adoptions logged"};
    }

    double hhi = 0.0;
    double entropy = 0.0;
    std::string hot_site = "";
    double max_share = 0.0;

    for (const auto& [id, count] : adoptions_) {
        double p = static_cast<double>(count) / static_cast<double>(total);
        hhi += p * p;
        if (p > 1e-9) {
            entropy -= p * std::log(p);
        }
        if (p > max_share) {
            max_share = p;
            hot_site = id;
        }
    }

    double max_possible_entropy = std::log(static_cast<double>(k));
    double floor_thresh = std::max(0.1, max_possible_entropy - entropy_floor_margin_);

    // Kill-switch engages if HHI exceeds threshold or entropy drops below floor or single site > 42%
    bool kill_switch = (hhi >= hhi_threshold_) || (entropy < floor_thresh) || (max_share >= 0.42);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    if (kill_switch) {
        ss << "[HERDING KILL-SWITCH ENGAGED] Concentration breach: HHI=" << hhi
           << " (Limit: " << hhi_threshold_ << "), Entropy=" << entropy
           << " (Floor: " << floor_thresh << "). Destination [" << hot_site
           << "] captured " << std::setprecision(1) << (max_share * 100.0)
           << "% share! Recommendations FROZEN to prevent secondary surge!";
    } else {
        ss << "Smooth dispersion maintained: HHI=" << hhi
           << ", Entropy=" << entropy << " (Floor: " << floor_thresh << ")";
    }

    return {
        hhi,
        entropy,
        floor_thresh,
        kill_switch,
        kill_switch ? hot_site : "",
        ss.str()
    };
}

bool HerdingDetector::is_destination_blocked(const std::string& destination_id) const {
    auto state = evaluate_herding();
    return state.is_kill_switch_active && (state.hot_alternative_id == destination_id);
}

} // namespace taal::guardrails

