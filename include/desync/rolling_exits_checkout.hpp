#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::desync {

struct ExitWaveInfo {
    int wave_index;             // 1, 2, 3, 4
    std::string color_name;     // "Crimson Red", "Emerald Green", "Sapphire Blue", "Gold Amber"
    int delay_minutes;          // 0, 12, 24, 36 minutes
    std::string experience_theme; // e.g., "Sherbet tasting + Chatta Chowk walk"
    std::string zone_gate;      // "Lahore Gate West", "Delhi Gate South", etc.
    int allocated_tickets;
};

struct StaggeredGateProtocol {
    std::string zone_id;
    std::string gate_name;
    int64_t opening_time_sec;
    int64_t closing_time_sec;
    std::string active_wave;
};

class ExitAsExperience {
public:
    ExitAsExperience();

    // Assign visitor ticket to one of 4 colored waves
    ExitWaveInfo assign_ticket_wave(int ticket_id, const std::string& preferred_interest = "");

    // Get current wave schedule for a show ending at show_end_sec
    std::vector<ExitWaveInfo> get_schedule(int64_t show_end_sec) const;

    // Gate staggered closing plan
    std::vector<StaggeredGateProtocol> generate_staggered_gate_plan(int64_t show_end_sec) const;

private:
    std::vector<ExitWaveInfo> waves_;
};

struct LockerReservation {
    std::string reservation_id;
    std::string hotel_id;
    std::string guest_name;
    std::string metro_station_id;
    int locker_number;
    int64_t checkout_slot_sec;
    int64_t pickup_deadline_sec;
    bool is_claimed{false};
};

class FreeHandsCheckout {
public:
    FreeHandsCheckout();

    // Register hotel with rolling bands
    void add_hotel(const std::string& hotel_id, const std::string& hotel_name, const std::string& area);

    // Book rolling checkout band and reserve metro locker
    LockerReservation book_free_hands_checkout(const std::string& hotel_id,
                                              const std::string& guest_name,
                                              int band_index, // 1: 10:30, 2: 11:15, 3: 12:00, 4: 12:45
                                              const std::string& desired_metro = "chandni_chowk_metro");

    // Query locker availability
    int get_available_lockers(const std::string& station_id) const;

private:
    struct HotelEntry {
        std::string id;
        std::string name;
        std::string area;
    };
    std::vector<HotelEntry> hotels_;
    std::vector<LockerReservation> reservations_;
    int reservation_counter_{2000};
};

} // namespace taal::desync

