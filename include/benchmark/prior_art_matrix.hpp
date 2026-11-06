#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::benchmark {

struct ComparisonSystem {
    std::string system_name;
    std::string category;
    std::string measurement_primitive; // e.g. "Headcount / Volume" vs "Phase Coherence R(t)"
    std::string cross_system_scope;    // e.g. "Siloed to Roads" vs "Metro -> Rickshaw -> Charger -> Feeder"
    std::string herding_mitigation;    // e.g. "None (Algorithmic Herding)" vs "Quota Lottery & Diversity Floor"
    std::string feedback_model;        // e.g. "Open-loop (Ignored)" vs "Reflexive Fixed-Point Solver"
    std::string resident_protection;   // e.g. "Cost Shifted to Alleys" vs "Resident Shield Hard Auto-Veto"
    std::string energy_coupling;       // e.g. "Unaware" vs "IEEE C57.91 Thermal + Battery Swap VPP"
    double novelty_score;              // Composite novelty score
};

class PriorArtMatrix {
public:
    PriorArtMatrix();

    // Generate formatted markdown or text table comparing all systems
    std::string generate_comparison_table() const;

    // Verify TAAL's unique architectural advantages
    bool verify_world_first_claims(std::string& out_proof) const;

    const std::vector<ComparisonSystem>& get_systems() const { return systems_; }

private:
    std::vector<ComparisonSystem> systems_;
};

} // namespace taal::benchmark

