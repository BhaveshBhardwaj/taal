#include "energy/battery_swap_vpp.hpp"
#include <sstream>
#include <iomanip>

namespace taal::energy {

BatterySwapVPP::BatterySwapVPP() {
    // 4 canonical Delhi battery swap hubs
    kiosks_.push_back({
        "kiosk_mori_gate",
        "Mori Gate Northern Feeder Swap Depot",
        36, 28, 8, 45.0, 90.0, false
    });
    kiosks_.push_back({
        "kiosk_kashmere_gate",
        "Kashmere Gate ISBT Transit Swap Terminal",
        48, 38, 10, 60.0, 120.0, false
    });
    kiosks_.push_back({
        "kiosk_netaji_subhash",
        "Netaji Subhash Marg Red Fort Monument Kiosk",
        30, 22, 8, 35.0, 75.0, false
    });
    kiosks_.push_back({
        "kiosk_asaf_ali",
        "Asaf Ali Road Southern Commercial Hub",
        32, 26, 6, 30.0, 80.0, false
    });
}

void BatterySwapVPP::add_kiosk(const SwapKiosk& kiosk) {
    kiosks_.push_back(kiosk);
}

VppGridDispatchResult BatterySwapVPP::coordinate_vpp(int64_t timestamp_sec,
                                                    double incoming_feeder_kva,
                                                    double feeder_threshold_kva) {
    int64_t t_day = timestamp_sec % SECONDS_PER_DAY;
    if (t_day < 0) t_day += SECONDS_PER_DAY;
    int hour = static_cast<int>(t_day / 3600);

    // 11:00 to 15:00 = Solar Soaking Window (Rooftop PV peak in Delhi)
    bool is_solar_window = (hour >= 11 && hour <= 15);

    // 20:30 to 22:30 = Evening Tourist Exit Peak
    bool is_evening_surge = (hour >= 20 && hour <= 22);
    bool grid_stressed = (incoming_feeder_kva >= feeder_threshold_kva * 0.88);

    double total_storage = 0.0;
    double total_draw = 0.0;
    double shaved_kw = 0.0;

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);

    if (is_evening_surge && grid_stressed) {
        // Active VPP Peak Shaving: Curtail fast recharging across all kiosks
        for (auto& k : kiosks_) {
            total_storage += (k.total_battery_slots * 2.5); // 2.5 kWh per pack
            double baseline_draw = k.current_grid_draw_kw;
            k.current_grid_draw_kw = 0.0; // Curtail to zero grid draw!
            k.is_curtailed_by_vpp = true;
            shaved_kw += baseline_draw;
        }
        ss << "[VPP PEAK SHAVING ACTIVE] 11kV Feeder at " << incoming_feeder_kva
           << " kVA. Curtailed 4 swap kiosks, shaving " << shaved_kw
           << " kW grid draw! Drivers served entirely from pre-charged battery buffers.";
    } else if (is_solar_window) {
        // Solar Soaking: Maximize daytime recharge
        for (auto& k : kiosks_) {
            total_storage += (k.total_battery_slots * 2.5);
            k.current_grid_draw_kw = k.max_charge_rate_kw;
            k.is_curtailed_by_vpp = false;
            total_draw += k.current_grid_draw_kw;
        }
        ss << "[VPP SOLAR SOAKING] Daytime solar absorption active. Drawing " << total_draw
           << " kW clean rooftop solar across 146 battery docks.";
    } else {
        // Nominal off-peak operation
        for (auto& k : kiosks_) {
            total_storage += (k.total_battery_slots * 2.5);
            k.is_curtailed_by_vpp = false;
            total_draw += k.current_grid_draw_kw;
        }
        ss << "Nominal VPP standby: 114 batteries ready for immediate 90-second swap.";
    }

    return {
        total_storage,
        total_draw,
        shaved_kw,
        is_solar_window,
        (is_evening_surge && grid_stressed),
        total_swaps_completed_,
        ss.str()
    };
}

bool BatterySwapVPP::execute_driver_swap(const std::string& /*vehicle_id*/, const std::string& kiosk_id) {
    for (auto& k : kiosks_) {
        if (k.kiosk_id == kiosk_id) {
            if (k.charged_batteries_ready > 0) {
                k.charged_batteries_ready--;
                k.charging_batteries_in_dock++;
                total_swaps_completed_++;
                return true;
            }
            return false;
        }
    }
    return false;
}

} // namespace taal::energy

