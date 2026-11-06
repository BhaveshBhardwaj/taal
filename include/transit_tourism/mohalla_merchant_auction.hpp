#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::transit_tourism {

struct MerchantBoosterBid {
    std::string bid_id;
    std::string merchant_id;
    std::string shop_name;
    std::string category;             // "spices", "heritage_food", "jewelry", "handicrafts"
    int64_t target_start_sec;         // e.g. 14:30:00 (52200s)
    int64_t target_end_sec;           // e.g. 17:00:00 (61200s)
    int bonus_phase_credits;          // Extra credits offered (e.g. +40 credits)
    std::string complimentary_perk;   // e.g. "Free kulhad masala chai + 10% heritage discount"
    int max_redemptions;
    int claimed_redemptions;
    bool is_active;
};

class MohallaMerchantAuction {
public:
    MohallaMerchantAuction();

    // Merchant submits an off-peak footfall booster bid
    std::string submit_booster_bid(const MerchantBoosterBid& bid);

    // Get active boosters for a visitor deciding an itinerary at timestamp_sec
    std::vector<MerchantBoosterBid> query_active_boosters(int64_t timestamp_sec) const;

    // Visitor claims a booster voucher
    bool claim_booster(const std::string& bid_id, const std::string& user_id);

    const std::vector<MerchantBoosterBid>& get_all_bids() const { return bids_; }

private:
    std::vector<MerchantBoosterBid> bids_;
    int bid_counter_{300};
};

} // namespace taal::transit_tourism

