#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::energy {

struct TransformerThermalState {
    double ambient_temp_c;          // Delhi ambient temperature (up to 47°C)
    double load_kva;                // Current electrical load in kVA
    double rated_capacity_kva;      // Nameplate rating (e.g. 1000 kVA at 30°C)
    double effective_derated_kva;   // Thermally derated capacity under ambient temp
    double top_oil_temp_c;          // Top oil temperature
    double hot_spot_temp_c;         // Hot-spot winding temperature Theta_hs
    double relative_aging_factor;   // Normal life = 1.0; > 2.0 = accelerated degradation
    bool is_thermal_overload;
    bool is_accelerated_aging;
    std::string risk_advisory;
};

class TransformerThermalModel {
public:
    explicit TransformerThermalModel(double rated_kva = 1000.0,
                                    double rated_temp_rise = 65.0,
                                    double max_hot_spot_continuous = 110.0);

    // Calculate effective thermal capacity under Delhi ambient heat
    double compute_derated_capacity(double ambient_temp_c) const;

    // Evaluate real-time thermal physics state according to IEEE C57.91
    TransformerThermalState evaluate_state(double current_load_kva, double ambient_temp_c) const;

    // Predict time (in minutes) until hot-spot insulation breach under current shock
    double predict_time_to_thermal_limit_minutes(double current_load_kva, double ambient_temp_c) const;

private:
    double rated_kva_;
    double rated_temp_rise_;
    double max_hot_spot_continuous_;
};

} // namespace taal::energy

