#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::energy {

struct SwapKiosk {
    std::string kiosk_id;
    std::string location_name;
    int total_battery_slots;
    int charged_batteries_ready;
    int charging_batteries_in_dock;
    double current_grid_draw_kw;
    double max_charge_rate_kw;
    bool is_curtailed_by_vpp{false};
};

struct VppGridDispatchResult {
    double total_vpp_storage_kwh;
    double total_current_draw_kw;
    double shaved_grid_power_kw;
    bool is_solar_soaking_active;
    bool is_peak_shaving_active;
    int active_swaps_completed;
    std::string vpp_action_summary;
};

class BatterySwapVPP {
public:
    BatterySwapVPP();

    void add_kiosk(const SwapKiosk& kiosk);

    // Coordinate the network under given time of day and grid stress level
    VppGridDispatchResult coordinate_vpp(int64_t timestamp_sec, double incoming_feeder_kva, double feeder_threshold_kva);

    // Driver swap event (90-second automated exchange)
    bool execute_driver_swap(const std::string& vehicle_id, const std::string& kiosk_id);

    const std::vector<SwapKiosk>& get_kiosks() const { return kiosks_; }

private:
    std::vector<SwapKiosk> kiosks_;
    int total_swaps_completed_{0};
};

} // namespace taal::energy

