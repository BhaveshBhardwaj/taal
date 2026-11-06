#include "measure/common_forcing_model.hpp"
#include <algorithm>
#include <cmath>

namespace taal::measure {

GRAPStage CommonForcingModel::get_grap_from_aqi(int aqi) {
    if (aqi >= 450) return GRAPStage::STAGE_IV_SEVERE_PLUS;
    if (aqi >= 401) return GRAPStage::STAGE_III_SEVERE;
    if (aqi >= 301) return GRAPStage::STAGE_II_VERY_POOR;
    if (aqi >= 201) return GRAPStage::STAGE_I_POOR;
    return GRAPStage::NONE;
}

std::string CommonForcingModel::grap_stage_to_string(GRAPStage stage) {
    switch (stage) {
        case GRAPStage::STAGE_IV_SEVERE_PLUS: return "GRAP-IV (Severe+ >450)";
        case GRAPStage::STAGE_III_SEVERE:      return "GRAP-III (Severe 401-450)";
        case GRAPStage::STAGE_II_VERY_POOR:   return "GRAP-II (Very Poor 301-400)";
        case GRAPStage::STAGE_I_POOR:        return "GRAP-I (Poor 201-300)";
        default:                              return "GRAP None (Nominal)";
    }
}

std::string CommonForcingModel::festival_to_string(DelhiFestival festival) {
    switch (festival) {
        case DelhiFestival::DIWALI_SHOPPING_RUSH:          return "Diwali Chandni Chowk Bazaar Rush";
        case DelhiFestival::RAMLILA_LAL_QILA:              return "Ramlila Lal Qila Grounds";
        case DelhiFestival::REPUBLIC_DAY_SECURITY_CURFEW:  return "Republic Day High-Alert Protocol";
        case DelhiFestival::EID_JAMA_MASJID:               return "Eid Celebrations Jama Masjid";
        case DelhiFestival::INDEPENDENCE_DAY_RED_FORT:     return "Independence Day Red Fort Address";
        default:                                           return "Standard Non-Festival Day";
    }
}

double CommonForcingModel::compute_forcing_index(const DelhiEnvironmentState& env,
                                                std::string& out_primary_driver) {
    double f_temp = 0.0;
    // Delhi extreme heat forcing: when temp > 40°C, outdoor movement is severely compressed
    if (env.temperature_celsius >= 45.0) {
        f_temp = 0.85; // Extreme forcing into sunset hours
    } else if (env.temperature_celsius >= 40.0) {
        f_temp = 0.50 + 0.35 * ((env.temperature_celsius - 40.0) / 5.0);
    } else if (env.temperature_celsius <= 6.0) {
        // Cold wave forcing
        f_temp = 0.40;
    }

    // AQI / GRAP forcing
    double f_aqi = 0.0;
    GRAPStage stage = (env.grap_stage != GRAPStage::NONE) ? env.grap_stage : get_grap_from_aqi(env.aqi);
    switch (stage) {
        case GRAPStage::STAGE_IV_SEVERE_PLUS: f_aqi = 0.80; break;
        case GRAPStage::STAGE_III_SEVERE:     f_aqi = 0.60; break;
        case GRAPStage::STAGE_II_VERY_POOR:   f_aqi = 0.35; break;
        case GRAPStage::STAGE_I_POOR:        f_aqi = 0.15; break;
        default:                              f_aqi = 0.0;  break;
    }

    // Fog forcing
    double f_fog = 0.0;
    if (env.visibility_meters < 100.0) {
        f_fog = 0.75; // Blind fog locks all transit
    } else if (env.visibility_meters < 300.0) {
        f_fog = 0.45;
    }

    // Festival forcing
    double f_fest = 0.0;
    switch (env.active_festival) {
        case DelhiFestival::RAMLILA_LAL_QILA:
        case DelhiFestival::DIWALI_SHOPPING_RUSH:
            f_fest = 0.90;
            break;
        case DelhiFestival::INDEPENDENCE_DAY_RED_FORT:
        case DelhiFestival::REPUBLIC_DAY_SECURITY_CURFEW:
            f_fest = 0.85;
            break;
        case DelhiFestival::EID_JAMA_MASJID:
            f_fest = 0.80;
            break;
        default:
            f_fest = 0.0;
            break;
    }

    // Find dominating driver
    double max_f = f_temp;
    out_primary_driver = "Extreme Delhi Summer Heat (45°C+)";

    if (f_fest > max_f) {
        max_f = f_fest;
        out_primary_driver = festival_to_string(env.active_festival);
    }
    if (f_aqi > max_f) {
        max_f = f_aqi;
        out_primary_driver = grap_stage_to_string(stage);
    }
    if (f_fog > max_f) {
        max_f = f_fog;
        out_primary_driver = "Dense Winter Smog/Fog (<200m visibility)";
    }

    if (max_f < 0.05) {
        out_primary_driver = "Benign Weather & Normal Operations";
    }

    // Combine drivers with sub-additive saturation: 1 - prod(1 - f_i)
    double combined = 1.0 - (1.0 - f_temp) * (1.0 - f_aqi) * (1.0 - f_fog) * (1.0 - f_fest);
    return std::clamp(combined, 0.0, 0.98);
}

SynchronyDecomposition CommonForcingModel::decompose(double total_r, const DelhiEnvironmentState& env) {
    std::string primary_driver;
    double f_index = compute_forcing_index(env, primary_driver);

    // Weather/forcing induces a minimum baseline coherence r_forced
    // e.g. at 45°C heat, visitors naturally don't enter at 2 PM, forcing higher R(t)
    double forced_r = f_index * 0.75;
    if (forced_r > total_r) forced_r = total_r;

    // Organic synchrony is the residual lockstep above the environmental floor
    double denom = 1.0 - forced_r;
    double organic_r = 0.0;
    if (denom > 1e-6) {
        organic_r = (total_r - forced_r) / denom;
    }
    organic_r = std::clamp(organic_r, 0.0, 1.0);

    return {
        total_r,
        f_index,
        forced_r,
        organic_r,
        primary_driver
    };
}

} // namespace taal::measure

