#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::measure {

enum class GRAPStage {
    NONE = 0,
    STAGE_I_POOR = 1,        // AQI 201-300
    STAGE_II_VERY_POOR = 2,  // AQI 301-400
    STAGE_III_SEVERE = 3,    // AQI 401-450
    STAGE_IV_SEVERE_PLUS = 4 // AQI 450+
};

enum class DelhiFestival {
    NONE,
    DIWALI_SHOPPING_RUSH,
    RAMLILA_LAL_QILA,
    REPUBLIC_DAY_SECURITY_CURFEW,
    EID_JAMA_MASJID,
    INDEPENDENCE_DAY_RED_FORT
};

struct DelhiEnvironmentState {
    double temperature_celsius{32.0};
    int aqi{180};
    GRAPStage grap_stage{GRAPStage::NONE};
    double visibility_meters{5000.0}; // < 200m = dense fog
    DelhiFestival active_festival{DelhiFestival::NONE};
    int64_t timestamp_sec{0};
};

struct SynchronyDecomposition {
    double total_r{0.0};       // Total observed Kuramoto R(t)
    double forcing_index{0.0};  // External forcing strength F_ext in [0, 1]
    double forced_r{0.0};       // Component driven by weather/GRAP/festivals
    double organic_r{0.0};      // Actionable human/institutional synchrony R_org
    std::string primary_forcing_driver;
};

class CommonForcingModel {
public:
    CommonForcingModel() = default;

    // Calculate external environmental forcing factor F_ext in [0, 1]
    static double compute_forcing_index(const DelhiEnvironmentState& env,
                                       std::string& out_primary_driver);

    // Decompose total observed Kuramoto R into forced vs organic synchrony
    static SynchronyDecomposition decompose(double total_r, const DelhiEnvironmentState& env);

    // Helper to determine GRAP stage from AQI
    static GRAPStage get_grap_from_aqi(int aqi);
    static std::string grap_stage_to_string(GRAPStage stage);
    static std::string festival_to_string(DelhiFestival festival);
};

} // namespace taal::measure

