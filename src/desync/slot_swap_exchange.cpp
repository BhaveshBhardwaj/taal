#include "desync/slot_swap_exchange.hpp"
#include <sstream>
#include <cmath>

namespace taal::desync {

std::string SlotSwapExchange::submit_order(const std::string& operator_name,
                                          const std::string& location_id,
                                          int64_t current_slot_sec,
                                          int64_t desired_slot_sec,
                                          int party_size) {
    order_counter_++;
    std::string oid = "SWAP-" + std::to_string(order_counter_);
    orders_.push_back({
        oid,
        operator_name,
        location_id,
        current_slot_sec,
        desired_slot_sec,
        party_size,
        SwapStatus::PENDING,
        ""
    });
    return oid;
}

bool SlotSwapExchange::cancel_order(const std::string& order_id) {
    for (auto& o : orders_) {
        if (o.order_id == order_id && o.status == SwapStatus::PENDING) {
            o.status = SwapStatus::CANCELLED;
            return true;
        }
    }
    return false;
}

std::vector<SwapMatchResult> SlotSwapExchange::execute_matching_cycle() {
    std::vector<SwapMatchResult> matches;

    // Bilateral matching: find pairs (i, j) where:
    // i's current == j's desired (within 30 mins) AND
    // j's current == i's desired (within 30 mins) AND
    // locations match AND sizes match within 20%
    size_t n = orders_.size();
    for (size_t i = 0; i < n; ++i) {
        if (orders_[i].status != SwapStatus::PENDING) continue;

        for (size_t j = i + 1; j < n; ++j) {
            if (orders_[j].status != SwapStatus::PENDING) continue;
            if (orders_[i].location_id != orders_[j].location_id) continue;

            // Check timing compatibility (within 30-min window = 1800s)
            bool time_compat_1 = std::abs(orders_[i].current_slot_sec - orders_[j].desired_slot_sec) <= 1800;
            bool time_compat_2 = std::abs(orders_[j].current_slot_sec - orders_[i].desired_slot_sec) <= 1800;

            if (time_compat_1 && time_compat_2) {
                // Capacity compatibility within 25%
                int max_sz = std::max(orders_[i].party_size, orders_[j].party_size);
                int min_sz = std::min(orders_[i].party_size, orders_[j].party_size);
                if (static_cast<double>(min_sz) / max_sz >= 0.70) {
                    // Match found!
                    orders_[i].status = SwapStatus::MATCHED;
                    orders_[j].status = SwapStatus::MATCHED;
                    orders_[i].matched_order_id = orders_[j].order_id;
                    orders_[j].matched_order_id = orders_[i].order_id;

                    cleared_swaps_count_++;

                    std::ostringstream ss;
                    ss << "Peer Swap Cleared: [" << orders_[i].operator_name << " (" << orders_[i].party_size << "p)] "
                       << format_time_hhmmss(orders_[i].current_slot_sec) << " <==> ["
                       << orders_[j].operator_name << " (" << orders_[j].party_size << "p)] "
                       << format_time_hhmmss(orders_[j].current_slot_sec);

                    matches.push_back({
                        true,
                        orders_[i].order_id,
                        orders_[j].order_id,
                        orders_[i].party_size + orders_[j].party_size,
                        orders_[i].current_slot_sec,
                        orders_[j].current_slot_sec,
                        ss.str()
                    });
                    break;
                }
            }
        }
    }

    return matches;
}

} // namespace taal::desync

