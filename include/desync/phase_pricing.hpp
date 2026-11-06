#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace taal::desync {

struct PhasePricingQuote {
    int64_t arrival_time_sec;
    double proposed_phase;
    double current_kuramoto_r;
    double marginal_coherence_delta; // Delta R: negative = desynchronizes (good), positive = synchronizes (bad)
    int phase_credits_earned;        // Positive reward credits
    double fare_surcharge_inr;       // Surcharge if entering during peak lockstep
    std::string recommended_shift_window;
    int shift_reward_credits;
};

struct MerchantPartner {
    std::string id;
    std::string name;
    std::string category;            // "heritage_sweets", "street_food", "handloom", "dmrc_metro"
    std::string location;
    int total_credits_redeemed;
    double inr_reimbursed;
};

class PhasePricing {
public:
    explicit PhasePricing(double base_reward_rate = 50.0);

    // Register active phases in corridor
    void set_current_phase_distribution(const std::vector<double>& active_phases);

    // Calculate phase pricing quote based on marginal contribution to coherence
    PhasePricingQuote calculate_quote(int64_t requested_time_sec) const;

    // Phase credits wallet management
    void add_wallet(const std::string& user_id, int initial_balance = 0);
    int get_balance(const std::string& user_id) const;
    void credit_wallet(const std::string& user_id, int credits, const std::string& reason);
    bool redeem_credits(const std::string& user_id, const std::string& merchant_id, int credits);

    // Register local merchants
    void register_merchant(const MerchantPartner& merchant);
    const std::vector<MerchantPartner>& get_merchants() const { return merchants_; }
    void seed_chandni_chowk_traders();

private:
    double base_reward_rate_;
    std::vector<double> current_phases_;
    std::unordered_map<std::string, int> wallets_;
    std::vector<MerchantPartner> merchants_;
};

} // namespace taal::desync

