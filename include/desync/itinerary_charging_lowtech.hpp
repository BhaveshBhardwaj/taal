#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::desync {

struct BatterySwapEvent {
    std::string vehicle_id;
    std::string kiosk_location;
    int64_t swap_time_sec;
    double old_soc_pct;
    double new_soc_pct;
    double kw_grid_draw_avoided;
};

struct CommunityBatteryStatus {
    double rated_capacity_kwh;
    double current_soc_kwh;
    double max_discharge_kw;
    double current_discharge_kw;
    bool is_discharging;
    double grid_peak_shaved_kw;
};

class ItineraryEmbeddedCharging {
public:
    explicit ItineraryEmbeddedCharging(double battery_capacity_kwh = 500.0, double max_discharge_kw = 300.0);

    // Book e-rickshaw swap relay based on cascade lag prediction
    BatterySwapEvent schedule_swap_relay(const std::string& vehicle_id,
                                        int64_t show_exit_sec,
                                        double current_soc_pct);

    // Trigger community battery injection to shave 11kV feeder transformer spike
    CommunityBatteryStatus trigger_peak_shaving(double incoming_feeder_kva, double transformer_limit_kva);

    // Recharging battery during off-peak night hours
    void recharge_community_battery(double kwh);

    CommunityBatteryStatus get_battery_status() const;

private:
    double capacity_kwh_;
    double soc_kwh_;
    double max_discharge_kw_;
    double current_discharge_kw_;
    bool is_discharging_{false};
    double total_shaved_kwh_{0.0};
};

struct BilingualMessage {
    std::string language; // "hi" (Hindi) or "en" (English)
    std::string channel;  // "PA_SYSTEM", "WHATSAPP_IVR", "TICKET_STUB", "RICKSHAW_CAPTAIN"
    std::string text;
    int64_t broadcast_time_sec;
};

class LowTechChannels {
public:
    LowTechChannels() = default;

    // Generate physical colored ticket stub text
    std::string format_ticket_stub(int ticket_id, const std::string& color_wave,
                                   int64_t scheduled_exit_sec, const std::string& perk);

    // Generate bilingual PA announcement for metro & monument concourse
    std::vector<BilingualMessage> generate_pa_announcements(const std::string& active_wave,
                                                            const std::string& venue = "Red Fort");

    // Interactive WhatsApp/IVR bot query in Hindi or English
    std::string handle_bot_query(const std::string& query, const std::string& lang = "hi");

    // Dispatch instruction to E-Rickshaw Stand Captain
    BilingualMessage dispatch_captain_directive(const std::string& stand_name,
                                                int incoming_passengers,
                                                int staggered_wave_min);
};

} // namespace taal::desync

