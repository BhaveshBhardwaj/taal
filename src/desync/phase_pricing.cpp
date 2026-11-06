#include "desync/phase_pricing.hpp"
#include <cmath>
#include <numeric>
#include <algorithm>

namespace taal::desync {

PhasePricing::PhasePricing(double base_reward_rate)
    : base_reward_rate_(base_reward_rate) {
    seed_chandni_chowk_traders();
}

void PhasePricing::seed_chandni_chowk_traders() {
    merchants_.clear();
    merchants_.push_back({
        "dmrc_smart_card",
        "Delhi Metro Rail Corporation (Smart Card Recharge)",
        "dmrc_metro",
        "Chandni Chowk / Lal Qila Metro Stations",
        0, 0.0
    });
    merchants_.push_back({
        "giani_di_hatti",
        "Giani di Hatti (Rabri Falooda)",
        "heritage_sweets",
        "Fatehpuri Masjid, Chandni Chowk",
        0, 0.0
    });
    merchants_.push_back({
        "paranthe_wali_gali",
        "Pt. Kanhaiyalal Durgaprasad Dixit (Paranthe)",
        "street_food",
        "Paranthe Wali Gali, Chandni Chowk",
        0, 0.0
    });
    merchants_.push_back({
        "kanwarji_confectioners",
        "Kanwarji Bhagirath Mal (Dalbiji & Gulab Jamun)",
        "heritage_sweets",
        "Opposite Central Baptist Church, Chandni Chowk",
        0, 0.0
    });
    merchants_.push_back({
        "dariba_kalan_silver",
        "Dariba Kalan Heritage Silver & Attar Guild",
        "handloom",
        "Dariba Kalan Lane",
        0, 0.0
    });
}

void PhasePricing::set_current_phase_distribution(const std::vector<double>& active_phases) {
    current_phases_ = active_phases;
}

PhasePricingQuote PhasePricing::calculate_quote(int64_t requested_time_sec) const {
    double phase = seconds_to_phase(requested_time_sec);

    if (current_phases_.empty()) {
        return {
            requested_time_sec,
            phase,
            0.0,
            0.0,
            static_cast<int>(base_reward_rate_),
            0.0,
            "Anytime",
            0
        };
    }

    // Compute baseline R
    double sum_cos = 0.0, sum_sin = 0.0;
    for (double phi : current_phases_) {
        sum_cos += std::cos(phi);
        sum_sin += std::sin(phi);
    }
    size_t n = current_phases_.size();
    double r_curr = std::sqrt(sum_cos * sum_cos + sum_sin * sum_sin) / static_cast<double>(n);

    // Compute R with new user's decision
    double new_cos = sum_cos + std::cos(phase);
    double new_sin = sum_sin + std::sin(phase);
    double r_new = std::sqrt(new_cos * new_cos + new_sin * new_sin) / static_cast<double>(n + 1);

    double delta_r = r_new - r_curr;

    int earned_credits = 0;
    double surcharge_inr = 0.0;
    std::string shift_window = "";
    int shift_reward = 0;

    // If delta_r < 0, decision actively desynchronizes peak! Reward with Phase Credits!
    if (delta_r < 0.0) {
        // Credits scale with negative delta R
        double bonus = std::abs(delta_r) * 1000.0;
        earned_credits = static_cast<int>(base_reward_rate_ + bonus);
    } else {
        // Synchronizing decision: user is contributing to unison
        surcharge_inr = 40.0 * (delta_r * 500.0);
        if (surcharge_inr > 60.0) surcharge_inr = 60.0;

        // Propose off-peak shift (e.g. 45 min earlier or later)
        int64_t shifted_time = requested_time_sec + 2700; // +45 min
        shift_window = format_time_hhmmss(shifted_time);
        shift_reward = 80; // earn 80 credits if shifted
    }

    return {
        requested_time_sec,
        phase,
        r_curr,
        delta_r,
        earned_credits,
        surcharge_inr,
        shift_window,
        shift_reward
    };
}

void PhasePricing::add_wallet(const std::string& user_id, int initial_balance) {
    wallets_[user_id] = initial_balance;
}

int PhasePricing::get_balance(const std::string& user_id) const {
    auto it = wallets_.find(user_id);
    return it != wallets_.end() ? it->second : 0;
}

void PhasePricing::credit_wallet(const std::string& user_id, int credits, const std::string& /*reason*/) {
    wallets_[user_id] += credits;
}

bool PhasePricing::redeem_credits(const std::string& user_id, const std::string& merchant_id, int credits) {
    auto it = wallets_.find(user_id);
    if (it == wallets_.end() || it->second < credits) {
        return false;
    }
    it->second -= credits;

    for (auto& m : merchants_) {
        if (m.id == merchant_id) {
            m.total_credits_redeemed += credits;
            m.inr_reimbursed += static_cast<double>(credits); // 1 Phase Credit = ₹1 INR
            return true;
        }
    }
    return true;
}

} // namespace taal::desync

