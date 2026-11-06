#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <memory>

namespace taal::desync {

enum class SwapStatus {
    PENDING,
    MATCHED,
    EXECUTED,
    CANCELLED
};

struct SlotOrder {
    std::string order_id;
    std::string operator_name;      // e.g. "Delhi Tourism Coach #4", "DPS R.K. Puram School Trip"
    std::string location_id;        // e.g. "red_fort_entry"
    int64_t current_slot_sec;       // e.g. 10:00:00 (36000s)
    int64_t desired_slot_sec;       // e.g. 14:30:00 (52200s)
    int party_size;                 // Number of visitors
    SwapStatus status{SwapStatus::PENDING};
    std::string matched_order_id;
};

struct SwapMatchResult {
    bool match_found{false};
    std::string order_a_id;
    std::string order_b_id;
    int total_visitors_swapped{0};
    int64_t slot_a_sec{0};
    int64_t slot_b_sec{0};
    std::string match_summary;
};

class SlotSwapExchange {
public:
    SlotSwapExchange() = default;

    // Submit an order to trade slots
    std::string submit_order(const std::string& operator_name,
                            const std::string& location_id,
                            int64_t current_slot_sec,
                            int64_t desired_slot_sec,
                            int party_size);

    // Cancel order
    bool cancel_order(const std::string& order_id);

    // Run matching engine to clear peer-to-peer swaps
    std::vector<SwapMatchResult> execute_matching_cycle();

    // Query active orders
    const std::vector<SlotOrder>& get_orders() const { return orders_; }

    // Summary of cleared swaps
    size_t get_total_swaps_cleared() const { return cleared_swaps_count_; }

private:
    std::vector<SlotOrder> orders_;
    size_t order_counter_{1000};
    size_t cleared_swaps_count_{0};
};

} // namespace taal::desync

