#pragma once

#include "core/types.hpp"
#include <vector>
#include <string>
#include <memory>
#include <random>

namespace taal::forecast {

enum class AgentRole {
    TOURIST_HERITAGE,
    METRO_COMMUTER,
    ERICKSHAW_OPERATOR,
    LOCAL_RESIDENT
};

enum class CorridorInterventionMode {
    UNISON_BASELINE,        // Status quo: everyone acts at once (show end, noon checkout)
    POLYRHYTHMIC_DESYNC     // TAAL enabled: staggered waves, twin moments, battery swap relay
};

struct AgentState {
    int id;
    AgentRole role;
    std::string current_node;
    int64_t target_exit_time_sec;
    double battery_soc_pct;       // For e-rickshaws (0 to 100%)
    double patience_budget_sec;
    double satisfaction_score;   // 0.0 to 1.0
    bool has_completed_trip{false};
};

struct CorridorSimulationMetrics {
    double peak_gate_wait_minutes{0.0};
    double peak_metro_platform_density{0.0};  // persons per m2
    double peak_erickshaw_queue_minutes{0.0};
    double peak_feeder_loading_pct{0.0};      // % of 1000 kVA rated capacity
    double resident_alley_noise_index{0.0};   // dB or relative index
    double resident_alley_footfall_count{0.0};
    double average_satisfaction_score{0.0};
    int effective_capacity_served{0};         // Visitors served before critical threshold breach
    int transformer_overload_seconds{0};
    double kuramoto_r_peak{0.0};
};

class SyntheticDelhiTwin {
public:
    explicit SyntheticDelhiTwin(size_t agent_count = 10000);

    // Initialize agent population with Delhi demographic distributions
    void initialize_agents(uint32_t seed = 42);

    // Run 24-hour corridor agent-based simulation
    CorridorSimulationMetrics run_simulation(CorridorInterventionMode mode,
                                             int time_step_seconds = 30);

    // Retrieve comparison summary between Unison and Polyrhythm
    static std::string compare_metrics(const CorridorSimulationMetrics& baseline,
                                       const CorridorSimulationMetrics& polyrhythm);

private:
    size_t agent_count_;
    std::vector<AgentState> agents_;
};

} // namespace taal::forecast

