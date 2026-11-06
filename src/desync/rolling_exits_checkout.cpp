#include "desync/rolling_exits_checkout.hpp"
#include <sstream>

namespace taal::desync {

ExitAsExperience::ExitAsExperience() {
    waves_.push_back({
        1,
        "Crimson Red",
        0,
        "Express Exit: Direct Lahore Gate concourse to Chandni Chowk Metro",
        "Lahore Gate North",
        0
    });
    waves_.push_back({
        2,
        "Emerald Green",
        12,
        "Heritage Taste: Chatta Chowk Mughal handicraft walkway + Rose & Khus Sherbet tasting",
        "Chatta Chowk Arcade",
        0
    });
    waves_.push_back({
        3,
        "Sapphire Blue",
        24,
        "Architectural Twilight: Meena Bazaar illuminated photo pavilions + Restrooms buffer",
        "Meena Bazaar South",
        0
    });
    waves_.push_back({
        4,
        "Gold Amber",
        36,
        "Nocturnal Serenade: Outer Moat garden stroll with live classical Shehnai recital",
        "Delhi Gate East",
        0
    });
}

ExitWaveInfo ExitAsExperience::assign_ticket_wave(int ticket_id, const std::string& /*preferred_interest*/) {
    int idx = (ticket_id % 4);
    waves_[idx].allocated_tickets++;
    return waves_[idx];
}

std::vector<ExitWaveInfo> ExitAsExperience::get_schedule(int64_t show_end_sec) const {
    std::vector<ExitWaveInfo> sched = waves_;
    for (auto& w : sched) {
        int64_t exit_time = show_end_sec + (w.delay_minutes * 60);
        w.experience_theme += " [Release: " + format_time_hhmmss(exit_time) + "]";
    }
    return sched;
}

std::vector<StaggeredGateProtocol> ExitAsExperience::generate_staggered_gate_plan(int64_t show_end_sec) const {
    std::vector<StaggeredGateProtocol> plan;
    plan.push_back({"Zone_A", "Lahore Gate North", show_end_sec, show_end_sec + 1800, "Crimson Red"});
    plan.push_back({"Zone_B", "Chatta Chowk Arcade", show_end_sec + 720, show_end_sec + 2700, "Emerald Green"});
    plan.push_back({"Zone_C", "Meena Bazaar South", show_end_sec + 1440, show_end_sec + 3600, "Sapphire Blue"});
    plan.push_back({"Zone_D", "Delhi Gate East", show_end_sec + 2160, show_end_sec + 4500, "Gold Amber"});
    return plan;
}

FreeHandsCheckout::FreeHandsCheckout() {
    add_hotel("hotel_bloom_paharganj", "Bloomrooms @ New Delhi Station", "Paharganj");
    add_hotel("hotel_ginger_railway", "Ginger Hotel New Delhi", "Ajmeri Gate");
    add_hotel("hotel_alois_aerocity", "Aloft New Delhi Aerocity", "Aerocity");
    add_hotel("hotel_haveli_dharampura", "Haveli Dharampura Heritage Hotel", "Chandni Chowk");
}

void FreeHandsCheckout::add_hotel(const std::string& hotel_id, const std::string& hotel_name, const std::string& area) {
    hotels_.push_back({hotel_id, hotel_name, area});
}

LockerReservation FreeHandsCheckout::book_free_hands_checkout(const std::string& hotel_id,
                                                            const std::string& guest_name,
                                                            int band_index,
                                                            const std::string& desired_metro) {
    reservation_counter_++;
    std::string rid = "LOCKER-" + std::to_string(reservation_counter_);

    // Bands: 1: 10:30 (37800s), 2: 11:15 (40500s), 3: 12:00 (43200s), 4: 12:45 (45900s)
    int64_t checkout_sec = 37800 + ((band_index - 1) % 4) * 2700;
    int64_t pickup_sec = checkout_sec + (5 * 3600); // 5 hours free wandering until 4 PM+

    int locker_num = 100 + (reservation_counter_ % 80);

    LockerReservation res = {
        rid,
        hotel_id,
        guest_name,
        desired_metro,
        locker_num,
        checkout_sec,
        pickup_sec,
        true
    };

    reservations_.push_back(res);
    return res;
}

int FreeHandsCheckout::get_available_lockers(const std::string& /*station_id*/) const {
    return std::max(0, 150 - static_cast<int>(reservations_.size()));
}

} // namespace taal::desync

