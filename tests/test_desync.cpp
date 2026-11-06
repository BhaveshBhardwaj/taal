#include "desync/polyrhythm_menu.hpp"
#include "desync/phase_pricing.hpp"
#include "desync/slot_swap_exchange.hpp"
#include "desync/rolling_exits_checkout.hpp"
#include "desync/itinerary_charging_lowtech.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace taal::desync;

void test_polyrhythm_menu_and_lottery() {
    std::cout << "[TEST] 1. Polyrhythm Menu & Lottery Suggestions (Features 8 & 9)...\n";
    PolyrhythmMenu menu;

    // Feature 8: Twin moments for Red Fort
    auto twins = menu.generate_twin_moments("red_fort");
    assert(!twins.empty());
    std::cout << "  Twin Moments generated for Red Fort (" << twins.size() << " alternatives):\n";
    for (const auto& opt : twins) {
        std::cout << "    - " << opt.destination.name
                  << " [Equal Delight Score: " << opt.equal_delight_score << "] -> "
                  << opt.highlight_perk << "\n";
        assert(opt.equal_delight_score >= 0.78);
    }

    // Feature 9: Lottery Suggestions with hard diversity floor
    std::unordered_map<std::string, int> draw_counts;
    int draws = 500;
    for (int i = 0; i < draws; ++i) {
        auto pick = menu.draw_lottery_suggestion("red_fort", 0.12, 100 + i);
        draw_counts[pick.destination.id]++;
    }

    std::cout << "  Lottery distribution across 500 draws (Anti-Herding Proof):\n";
    for (const auto& [id, count] : draw_counts) {
        double pct = (static_cast<double>(count) / draws) * 100.0;
        std::cout << "    * " << id << ": " << count << " draws (" << pct << "%)\n";
        // Diversity floor check: each alternative gets at least 8% of suggestions
        assert(pct >= 8.0);
    }
    std::cout << "  Passed: Polyrhythm Menu & Lottery verified with active diversity floor!\n";
}

void test_phase_pricing() {
    std::cout << "\n[TEST] 2. Phase Pricing & Credits Ledger (Feature 10)...\n";
    PhasePricing pricing;

    // Simulate existing corridor phase distribution clustered around 21:00 (75600s)
    std::vector<double> existing_phases;
    for (int i = -30; i <= 30; ++i) {
        existing_phases.push_back(taal::seconds_to_phase(75600 + i * 15));
    }
    pricing.set_current_phase_distribution(existing_phases);

    // Case A: User requests arrival at 21:00 (reinforces unison -> positive delta R)
    auto quote_sync = pricing.calculate_quote(75600);
    std::cout << "  Case A (Lockstep Arrival at 21:00): Delta R = "
              << quote_sync.marginal_coherence_delta
              << ", Surcharge = Rs " << quote_sync.fare_surcharge_inr
              << ", Shift Incentive: " << quote_sync.shift_reward_credits
              << " credits at " << quote_sync.recommended_shift_window << "\n";
    assert(quote_sync.marginal_coherence_delta > 0.0);
    assert(quote_sync.shift_reward_credits > 0);

    // Case B: User requests arrival at 16:00 (disperses peak -> negative delta R)
    auto quote_desync = pricing.calculate_quote(57600);
    std::cout << "  Case B (Desynchronizing Arrival at 16:00): Delta R = "
              << quote_desync.marginal_coherence_delta
              << ", Earned Credits = " << quote_desync.phase_credits_earned << " credits\n";
    assert(quote_desync.marginal_coherence_delta < 0.0);
    assert(quote_desync.phase_credits_earned > 0);

    // Case C: Wallet & Merchant Redemption at Chandni Chowk
    pricing.add_wallet("user_rohit", 0);
    pricing.credit_wallet("user_rohit", quote_desync.phase_credits_earned, "Off-peak visit bonus");
    assert(pricing.get_balance("user_rohit") == quote_desync.phase_credits_earned);

    bool redeemed = pricing.redeem_credits("user_rohit", "giani_di_hatti", 50);
    assert(redeemed);
    assert(pricing.get_balance("user_rohit") == quote_desync.phase_credits_earned - 50);
    std::cout << "  Passed: Phase Credits redeemed at Giani di Hatti (Fatehpuri, Chandni Chowk)!\n";
}

void test_slot_swap_exchange() {
    std::cout << "\n[TEST] 3. Slot-Swap Exchange Barter Order Book (Feature 11)...\n";
    SlotSwapExchange exchange;

    // Operator 1: DPS School bus holds 10:00 (36000s), wants 14:00 (50400s), 75 students
    std::string o1 = exchange.submit_order("DPS School Bus", "red_fort_entry", 36000, 50400, 75);

    // Operator 2: Delhi Tourism Coach holds 14:00 (50400s), wants 10:00 (36000s), 70 tourists
    std::string o2 = exchange.submit_order("Delhi Tourism Coach #4", "red_fort_entry", 50400, 36000, 70);

    // Execute matching
    auto matches = exchange.execute_matching_cycle();
    assert(matches.size() == 1);
    std::cout << "  " << matches[0].match_summary << "\n";
    assert(matches[0].order_a_id == o1 && matches[0].order_b_id == o2);
    assert(matches[0].total_visitors_swapped == 145);
    std::cout << "  Passed: Peer-to-peer slot barter cleared 145 visitors with zero monetary fees!\n";
}

void test_rolling_exits_and_checkout() {
    std::cout << "\n[TEST] 4. Rolling Exits & Free-Hands Checkout (Features 12 & 13)...\n";
    ExitAsExperience exits;

    // Assign waves
    auto wave1 = exits.assign_ticket_wave(101);
    auto wave2 = exits.assign_ticket_wave(102);
    auto wave3 = exits.assign_ticket_wave(103);
    auto wave4 = exits.assign_ticket_wave(104);

    assert(wave1.color_name == "Emerald Green" || wave1.color_name == "Crimson Red");
    assert(wave4.delay_minutes >= 0);

    auto schedule = exits.get_schedule(75600); // 21:00 show end
    std::cout << "  Exit-as-Experience 4 Wave Schedule:\n";
    for (const auto& w : schedule) {
        std::cout << "    - Wave " << w.wave_index << " [" << w.color_name << ", +" << w.delay_minutes
                  << "m]: " << w.experience_theme << "\n";
    }

    // Free-Hands Checkout
    FreeHandsCheckout checkout;
    auto locker = checkout.book_free_hands_checkout("hotel_bloom_paharganj", "Amit Sharma", 2, "chandni_chowk_metro");
    std::cout << "  Free-Hands Checkout booked: Guest " << locker.guest_name
              << " checked out at " << taal::format_time_hhmmss(locker.checkout_slot_sec)
              << ", Locker #" << locker.locker_number << " reserved until "
              << taal::format_time_hhmmss(locker.pickup_deadline_sec) << "\n";
    assert(locker.locker_number >= 100);
    assert(checkout.get_available_lockers("chandni_chowk_metro") > 100);
    std::cout << "  Passed: Rolling exits and smart locker reservations verified!\n";
}

void test_charging_and_lowtech() {
    std::cout << "\n[TEST] 5. Itinerary Charging & Low-Tech Channels (Features 14 & 15)...\n";
    ItineraryEmbeddedCharging charging(500.0, 300.0);

    // Schedule swap relay
    auto swap = charging.schedule_swap_relay("DL-1ER-4491", 75600, 32.0);
    std::cout << "  Battery Swap scheduled at " << taal::format_time_hhmmss(swap.swap_time_sec)
              << " for vehicle " << swap.vehicle_id << " at " << swap.kiosk_location << "\n";
    assert(swap.new_soc_pct == 98.0);

    // Trigger community battery peak shaving at 11kV transformer
    auto status = charging.trigger_peak_shaving(1050.0, 850.0);
    std::cout << "  Community Battery Triggered: Grid Peak Shaved = " << status.grid_peak_shaved_kw
              << " kW, Battery SoC = " << status.current_soc_kwh << " kWh\n";
    assert(status.is_discharging);
    assert(status.grid_peak_shaved_kw == 200.0);

    // Low-tech channels
    LowTechChannels channels;
    std::string stub = channels.format_ticket_stub(4092, "Sapphire Blue", 77040, "Meena Bazaar Twilight Walk");
    std::cout << stub;

    auto pa = channels.generate_pa_announcements("Sapphire Blue", "Red Fort");
    assert(pa.size() == 2);
    std::cout << "  PA Hindi Announcement: " << pa[0].text << "\n";

    std::string hi_reply = channels.handle_bot_query("लाल किला शो के बाद क्या देखें?", "hi");
    assert(!hi_reply.empty());
    std::cout << "  WhatsApp Bot Hindi Reply: " << hi_reply << "\n";

    std::cout << "  Passed: Itinerary charging relay and low-tech communication verified!\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  TAAL - LAYER 3: BREAK THE SYNCHRONY TEST SUITE        \n";
    std::cout << "========================================================\n\n";

    test_polyrhythm_menu_and_lottery();
    test_phase_pricing();
    test_slot_swap_exchange();
    test_rolling_exits_and_checkout();
    test_charging_and_lowtech();

    std::cout << "\n========================================================\n";
    std::cout << "  ALL LAYER 3 TESTS PASSED SUCCESSFULLY!                \n";
    std::cout << "========================================================\n";
    return 0;
}

