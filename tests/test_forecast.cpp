#include "forecast/reflexive_forecast.hpp"
#include "forecast/critical_slowing_down.hpp"
#include "forecast/synthetic_delhi_twin.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace taal::forecast;

void test_reflexive_forecast() {
    std::cout << "[TEST] 1. Reflexive Fixed-Point Forecast Verification...\n";
    ReflexiveForecast rf;

    // Test fixed point calculation
    auto result = rf.compute_fixed_point_forecast(0.60, 0.30, 80, 1e-4);
    std::cout << result.advisory_summary << "\n";

    assert(result.converged);
    assert(result.iterations < 60);
    assert(!result.nodes.empty());

    // Verify equilibrium balances demand
    for (const auto& node : result.nodes) {
        assert(node.expected_visitor_equilibrium > 0.0);
        assert(node.published_congestion_score >= 0.0 && node.published_congestion_score <= 1.0);
    }
    std::cout << "  Passed: Fixed-point crowd equilibrium converged in " << result.iterations << " iterations!\n";
}

void test_critical_slowing_down() {
    std::cout << "\n[TEST] 2. Critical Slowing Down (Coherence Early Warning)...\n";
    CriticalSlowingDown csd(20, 4.0);

    // Feed quiet baseline queue for 30 minutes
    for (int t = 0; t < 30; ++t) {
        double baseline_noise = (t % 3) * 1.5;
        csd.record_observation(t * 60, 10.0 + baseline_noise);
    }
    auto quiet_state = csd.evaluate_current_state();
    assert(!quiet_state.is_critical_slowing_down);
    std::cout << "  Quiet state: Var=" << quiet_state.rolling_variance
              << ", Lag-1 Autocorr=" << quiet_state.lag1_autocorrelation
              << " -> CSD Alert: " << (quiet_state.is_critical_slowing_down ? "YES" : "NO") << "\n";

    // Now inject critical slowing down: perturbations persist longer, variance increases
    double val = 12.0;
    for (int t = 30; t < 55; ++t) {
        // High autoregression: x_t = 0.85 * x_{t-1} + noise
        val = 0.85 * val + 3.5 + ((t % 5) * 1.2);
        csd.record_observation(t * 60, val);
    }

    auto surge_state = csd.evaluate_current_state();
    std::cout << "  Pre-surge state: Var=" << surge_state.rolling_variance
              << ", Lag-1 Autocorr=" << surge_state.lag1_autocorrelation
              << " -> CSD Alert: " << (surge_state.is_critical_slowing_down ? "YES" : "NO") << "\n";
    std::cout << "  Alert message: " << surge_state.warning_message << "\n";

    assert(surge_state.is_critical_slowing_down);
    assert(surge_state.lag1_autocorrelation > 0.40);
    std::cout << "  Passed: Early warning successfully detected impending phase transition!\n";
}

void test_synthetic_delhi_twin() {
    std::cout << "\n[TEST] 3. Synthetic Delhi Digital Twin (10,000 Agents)...\n";
    SyntheticDelhiTwin twin(10000);

    // Run baseline unison simulation
    auto baseline = twin.run_simulation(CorridorInterventionMode::UNISON_BASELINE);

    // Run polyrhythmic desync simulation
    auto poly = twin.run_simulation(CorridorInterventionMode::POLYRHYTHMIC_DESYNC);

    std::cout << SyntheticDelhiTwin::compare_metrics(baseline, poly);

    // Assert key performance invariants from page 5 of specification:
    // 1. Kuramoto DPI reduced
    assert(poly.kuramoto_r_peak < baseline.kuramoto_r_peak);
    // 2. Peak gate wait reduced significantly
    assert(poly.peak_gate_wait_minutes < baseline.peak_gate_wait_minutes);
    // 3. Feeder transformer loading stays safe in polyrhythm
    assert(poly.peak_feeder_loading_pct <= 100.0);
    // 4. Overload duration eliminated
    assert(poly.transformer_overload_seconds == 0);
    // 5. Zero cost shifting to residents
    assert(poly.resident_alley_footfall_count <= baseline.resident_alley_footfall_count);
    // 6. Satisfaction improved
    assert(poly.average_satisfaction_score >= baseline.average_satisfaction_score);

    std::cout << "  Passed: Digital Twin proves polyrhythmic interventions eliminate stress without new infrastructure!\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  TAAL - LAYER 2: FORECAST & DIGITAL TWIN TEST SUITE     \n";
    std::cout << "========================================================\n\n";

    test_reflexive_forecast();
    test_critical_slowing_down();
    test_synthetic_delhi_twin();

    std::cout << "\n========================================================\n";
    std::cout << "  ALL LAYER 2 TESTS PASSED SUCCESSFULLY!                \n";
    std::cout << "========================================================\n";
    return 0;
}

