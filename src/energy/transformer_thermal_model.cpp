#include "energy/transformer_thermal_model.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace taal::energy {

TransformerThermalModel::TransformerThermalModel(double rated_kva,
                                                double rated_temp_rise,
                                                double max_hot_spot_continuous)
    : rated_kva_(rated_kva),
      rated_temp_rise_(rated_temp_rise),
      max_hot_spot_continuous_(max_hot_spot_continuous) {}

double TransformerThermalModel::compute_derated_capacity(double ambient_temp_c) const {
    // Reference ambient temperature is 30°C according to IS 2026 / IEC 60076 / IEEE C57.91.
    // In Delhi summers, for every 1°C ambient above 30°C, oil natural (ONAN) distribution transformers
    // suffer ~1.5% thermal derating due to reduced convective air-oil temperature differential.
    if (ambient_temp_c <= 30.0) {
        return rated_kva_;
    }
    double delta_ambient = ambient_temp_c - 30.0;
    double derating_fraction = delta_ambient * 0.015; // 1.5% per °C above 30°C
    if (derating_fraction > 0.40) derating_fraction = 0.40; // cap at 40% max derating

    return rated_kva_ * (1.0 - derating_fraction);
}

TransformerThermalState TransformerThermalModel::evaluate_state(double current_load_kva, double ambient_temp_c) const {
    double derated_cap = compute_derated_capacity(ambient_temp_c);
    double load_ratio = (derated_cap > 0) ? (current_load_kva / rated_kva_) : 1.0;

    // IEEE C57.91 calculations:
    // Top-oil rise = rated_top_oil_rise * (load_ratio)^0.8
    double rated_top_oil_rise = 55.0;
    double top_oil_rise = rated_top_oil_rise * std::pow(load_ratio, 1.6);
    double top_oil_temp = ambient_temp_c + top_oil_rise;

    // Hot-spot winding gradient over top oil = rated_winding_grad * (load_ratio)^1.6
    double rated_winding_grad = 23.0;
    double hot_spot_grad = rated_winding_grad * std::pow(load_ratio, 1.6);
    double hot_spot_temp = top_oil_temp + hot_spot_grad;

    // Relative aging rate according to Arrhenius equation:
    // V = exp(15000/383 - 15000/(Theta_hs + 273.15))
    // Normal life is defined at 110°C (383.15 K) where V = 1.0
    double kelvin_hs = hot_spot_temp + 273.15;
    double v_aging = std::exp((15000.0 / 383.15) - (15000.0 / kelvin_hs));

    bool is_thermal_overload = (current_load_kva > derated_cap) || (hot_spot_temp > max_hot_spot_continuous_);
    bool is_accelerated_aging = (v_aging > 1.8);

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (hot_spot_temp > 120.0) {
        ss << "CRITICAL THERMAL RISK: Hot-spot at " << hot_spot_temp
           << "°C! Insulation aging factor " << std::setprecision(1) << v_aging
           << "x! Trigger BESS discharge immediately!";
    } else if (is_thermal_overload) {
        ss << "THERMAL DERATING OVERLOAD: Load " << current_load_kva
           << " kVA exceeds Delhi derated capacity (" << derated_cap << " kVA at "
           << ambient_temp_c << "°C ambient)";
    } else {
        ss << "Nominal thermal state: Hot-spot " << hot_spot_temp << "°C (Safe limit: "
           << max_hot_spot_continuous_ << "°C). Derated headroom: " << (derated_cap - current_load_kva) << " kVA";
    }

    return {
        ambient_temp_c,
        current_load_kva,
        rated_kva_,
        derated_cap,
        top_oil_temp,
        hot_spot_temp,
        v_aging,
        is_thermal_overload,
        is_accelerated_aging,
        ss.str()
    };
}

double TransformerThermalModel::predict_time_to_thermal_limit_minutes(double current_load_kva, double ambient_temp_c) const {
    double derated_cap = compute_derated_capacity(ambient_temp_c);
    if (current_load_kva <= derated_cap) {
        return 999.0; // Steady state within limits
    }
    // Thermal time constant tau for oil distribution transformers is ~1.5 - 2.5 hours (90-150 mins)
    double overload_ratio = current_load_kva / derated_cap;
    double time_mins = 120.0 / std::pow(overload_ratio, 2.5);
    return std::max(5.0, time_mins);
}

} // namespace taal::energy

