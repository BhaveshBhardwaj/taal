#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace taal::guardrails {

struct SwapBackPass {
    std::string token_id;
    std::string user_id;
    std::string original_site;
    std::string accepted_alternate_site;
    int64_t issued_at_sec;
    int64_t expiry_sec;
    bool is_redeemed{false};
    double max_regret_bound;
    std::string priority_gate_code;
};

class RegretBudget {
public:
    explicit RegretBudget(double max_regret_bound = 0.35);

    // Issue a guaranteed swap-back token with every nudge
    SwapBackPass issue_nudge_guarantee(const std::string& user_id,
                                       const std::string& original_site,
                                       const std::string& alternate_site,
                                       int64_t current_time_sec);

    // Redeem swap back if alternate site underdelivered
    bool redeem_swap_back(const std::string& token_id, int64_t current_time_sec);

    // Get active swap-back tokens count
    size_t get_active_tokens_count() const;

private:
    double max_regret_bound_;
    std::unordered_map<std::string, SwapBackPass> active_tokens_;
    int token_counter_{5000};
};

struct HerdingState {
    double herfindahl_index; // HHI in [1/K, 1.0]
    double shannon_entropy;   // H(p)
    double entropy_floor;     // Minimum permissible entropy
    bool is_kill_switch_active;
    std::string hot_alternative_id; // Site triggering kill-switch if any
    std::string status_summary;
};

class HerdingDetector {
public:
    explicit HerdingDetector(double hhi_threshold = 0.38, double entropy_floor_margin = 0.40);

    // Record an adopted recommendation
    void record_adoption(const std::string& alternative_id);

    // Clear buffer
    void reset();

    // Evaluate concentration and check kill-switch
    HerdingState evaluate_herding() const;

    // Is a specific destination currently blocked by the kill-switch?
    bool is_destination_blocked(const std::string& destination_id) const;

private:
    double hhi_threshold_;
    double entropy_floor_margin_;
    std::unordered_map<std::string, int> adoptions_;
};

} // namespace taal::guardrails

