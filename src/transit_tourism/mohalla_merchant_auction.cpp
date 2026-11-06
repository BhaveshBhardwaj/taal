#include "transit_tourism/mohalla_merchant_auction.hpp"

namespace taal::transit_tourism {

MohallaMerchantAuction::MohallaMerchantAuction() {
    // Seed authentic Old Delhi heritage traders
    bids_.push_back({
        "BID-301",
        "paranthe_wali_gali_pt_gaya",
        "Pt. Gaya Prasad Shiv Charan (Est. 1872)",
        "heritage_food",
        52200, // 14:30:00
        61200, // 17:00:00
        40,    // +40 Phase Credits
        "Complimentary Rabri Lassi + 10% Heritage discount",
        50,
        14,
        true
    });

    bids_.push_back({
        "BID-302",
        "khari_baoli_spice_mehlawat",
        "Mehlawat Spices & Dry Fruits (Khari Baoli)",
        "spices",
        46800, // 13:00:00
        57600, // 16:00:00
        50,    // +50 Phase Credits
        "Free Kashmiri Saffron sample pouch + Aromatics tour",
        40,
        9,
        true
    });

    bids_.push_back({
        "BID-303",
        "dariba_kalan_gulabsingh",
        "Gulab Singh Johrimal Attar & Ittar (Est. 1816)",
        "perfumes",
        54000, // 15:00:00
        64800, // 18:00:00
        35,    // +35 Phase Credits
        "Complimentary Ruh Gulab attar vial on purchases > Rs 300",
        60,
        22,
        true
    });
}

std::string MohallaMerchantAuction::submit_booster_bid(const MerchantBoosterBid& bid) {
    bid_counter_++;
    std::string bid_id = "BID-" + std::to_string(bid_counter_);
    MerchantBoosterBid entry = bid;
    entry.bid_id = bid_id;
    entry.is_active = true;
    bids_.push_back(entry);
    return bid_id;
}

std::vector<MerchantBoosterBid> MohallaMerchantAuction::query_active_boosters(int64_t timestamp_sec) const {
    int64_t t_day = timestamp_sec % SECONDS_PER_DAY;
    if (t_day < 0) t_day += SECONDS_PER_DAY;

    std::vector<MerchantBoosterBid> matches;
    for (const auto& b : bids_) {
        if (b.is_active && t_day >= b.target_start_sec && t_day <= b.target_end_sec) {
            if (b.claimed_redemptions < b.max_redemptions) {
                matches.push_back(b);
            }
        }
    }
    return matches;
}

bool MohallaMerchantAuction::claim_booster(const std::string& bid_id, const std::string& /*user_id*/) {
    for (auto& b : bids_) {
        if (b.bid_id == bid_id && b.is_active) {
            if (b.claimed_redemptions < b.max_redemptions) {
                b.claimed_redemptions++;
                return true;
            }
        }
    }
    return false;
}

} // namespace taal::transit_tourism

