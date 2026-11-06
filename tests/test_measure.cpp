#include "measure/kuramoto_dpi.hpp"
#include "measure/pacemaker_audit.hpp"
#include "measure/cascade_lag_map.hpp"
#include "measure/common_forcing_model.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace taal;
using namespace taal::measure;

void test_kuramoto_dpi() {
    std::cout << "[TEST] 1. Kuramoto DPI Mathematical Verification...\n";

    // Test 1: Uniformly desynchronized phases -> r must be ~ 0.0
    std::vector<double> uniform_phases = {0.0, PI * 0.5, PI, PI * 1.5};
    auto r_uniform = KuramotoDPI::compute(uniform_phases);
    assert(r_uniform.r < 1e-6);
    std::cout << "  Passed: Uniformly scattered phases -> r = " << r_uniform.r << " (~0.0)\n";

    // Test 2: Identical lockstep phases -> r must be 1.0
    std::vector<double> lockstep_phases = {1.2, 1.2, 1.2, 1.2, 1.2};
    auto r_lockstep = KuramotoDPI::compute(lockstep_phases);
    assert(std::abs(r_lockstep.r - 1.0) < 1e-6);
    assert(std::abs(r_lockstep.mean_phase - 1.2) < 1e-6);
    std::cout << "  Passed: Lockstep phases -> r = " << r_lockstep.r << " (1.0)\n";

    // Test 3: Sync-vs-Volume hypothesis test
    std::vector<double> volume, dpi, stress;
    KuramotoDPI::generate_synthetic_validation_dataset(24 * 7, volume, dpi, stress);
    auto sync_test = KuramotoDPI::run_sync_vs_volume_test(volume, dpi, stress);

    std::cout << "  " << sync_test.summary_proof << "\n";
    assert(sync_test.r2_volume_and_dpi > sync_test.r2_volume_only);
    assert(sync_test.rmse_volume_and_dpi < sync_test.rmse_volume_only);
    assert(sync_test.hypothesis_confirmed);
    std::cout << "  Passed: Sync-vs-Volume hypothesis strictly confirmed!\n";
}

void test_pacemaker_audit() {
    std::cout << "\n[TEST] 2. Pacemaker Audit Verification...\n";
    PacemakerAudit audit;

    // Simulate 2000 visitors exiting Red Fort show at 21:00 (75600s)
    for (int i = 0; i < 2000; ++i) {
        int jitter = (i % 600) - 300; // within +/- 5 minutes
        audit.record_decision(75600 + jitter, "Chandni Chowk / Red Fort");
    }

    // Simulate 800 guests checking out of hotels around 11:30 (41400s)
    for (int i = 0; i < 800; ++i) {
        int jitter = (i % 1200) - 600;
        audit.record_decision(41400 + jitter, "Paharganj / Aerocity");
    }

    audit.audit_and_rank();
    const auto& ranked = audit.get_ranked_pacemakers();
    assert(!ranked.empty());
    
    // Top pacemaker should be Red Fort show with high coupling strength
    assert(ranked[0].id == "red_fort_sound_light");
    assert(ranked[0].coupling_strength > 0.90);
    assert(ranked[0].observed_visitors == 2000);
    std::cout << audit.generate_audit_report();
    std::cout << "  Passed: Red Fort identified as primary synchronizer with Kp = "
              << ranked[0].coupling_strength << "\n";
}

void test_cascade_lag_map() {
    std::cout << "\n[TEST] 3. Cascade Lag Map Propagation...\n";
    CascadeLagMap map;

    // Check cumulative lags match specification
    double lag_gate = map.get_cumulative_lag_minutes("red_fort_show", "lahore_gate_crush");
    double lag_transit = map.get_cumulative_lag_minutes("red_fort_show", "erickshaw_metro_queue");
    double lag_charger = map.get_cumulative_lag_minutes("red_fort_show", "ev_charging_hub");
    double lag_feeder = map.get_cumulative_lag_minutes("red_fort_show", "feeder_transformer_11kv");

    std::cout << "  Lags from Red Fort Show Exit:\n"
              << "    -> Gate Crush:           " << lag_gate << " min (Target: 8 min)\n"
              << "    -> E-rickshaw / Metro:   " << lag_transit << " min (Target: 20 min)\n"
              << "    -> EV Charger Cluster:   " << lag_charger << " min (Target: 50 min)\n"
              << "    -> Feeder Transformer:   " << lag_feeder << " min (Target: 90 min)\n";

    assert(std::abs(lag_gate - 8.0) < 1e-4);
    assert(std::abs(lag_transit - 20.0) < 1e-4);
    assert(std::abs(lag_charger - 50.0) < 1e-4);
    assert(std::abs(lag_feeder - 90.0) < 1e-4);

    // Inject 3500 visitors show ending at 21:00 (75600s)
    int64_t show_end_t = 75600;
    map.inject_impulse("red_fort_show", 3500.0, show_end_t);

    // Check transformer load at show end vs at peak lag (+90 min = 81000s)
    auto loads_at_show = map.predict_loads_at(show_end_t);
    auto loads_at_peak = map.predict_loads_at(show_end_t + 90 * 60);

    double baseline_trans = loads_at_show["feeder_transformer_11kv"];
    double peak_trans = loads_at_peak["feeder_transformer_11kv"];

    std::cout << "  Feeder Transformer at Show End: " << baseline_trans << " kVA\n"
              << "  Feeder Transformer at Peak (+90m): " << peak_trans << " kVA\n";
    assert(peak_trans > baseline_trans + 100.0);

    // Check early warning generation
    auto alerts = map.generate_early_warnings(show_end_t);
    bool found_feeder_alert = false;
    for (const auto& a : alerts) {
        if (a.target_node_id == "feeder_transformer_11kv") {
            found_feeder_alert = true;
            std::cout << "  Early Warning Alert for DISCOM: Target = " << a.target_node_name
                      << ", Lead Time = " << (a.lead_time_sec / 60) << " minutes"
                      << ", Breach = " << (a.will_breach_capacity ? "YES" : "NO")
                      << ", Action: " << a.recommendation << "\n";
            assert(a.lead_time_sec == 90 * 60);
        }
    }
    assert(found_feeder_alert);
    std::cout << "  Passed: Cascade Lag Map accurately predicted 90m grid warning!\n";
}

void test_common_forcing() {
    std::cout << "\n[TEST] 4. Common-Forcing Model Verification...\n";

    // Case 1: Extreme summer heat 46°C in Delhi
    DelhiEnvironmentState heat_env;
    heat_env.temperature_celsius = 46.0;
    heat_env.aqi = 150;
    auto dec_heat = CommonForcingModel::decompose(0.85, heat_env);

    std::cout << "  Scenario A: Extreme Delhi Summer Heat (46°C)\n"
              << "    Total R: " << dec_heat.total_r
              << ", Forcing Index: " << dec_heat.forcing_index
              << ", Forced R: " << dec_heat.forced_r
              << ", Organic R: " << dec_heat.organic_r
              << " (Driver: " << dec_heat.primary_forcing_driver << ")\n";
    assert(dec_heat.forcing_index > 0.80);
    assert(dec_heat.organic_r < dec_heat.total_r);

    // Case 2: GRAP-IV Winter Smog with Ramlila
    DelhiEnvironmentState fest_env;
    fest_env.temperature_celsius = 22.0;
    fest_env.aqi = 475; // GRAP-IV
    fest_env.active_festival = DelhiFestival::RAMLILA_LAL_QILA;
    auto dec_fest = CommonForcingModel::decompose(0.92, fest_env);

    std::cout << "  Scenario B: GRAP-IV Smog + Ramlila Festival\n"
              << "    Total R: " << dec_fest.total_r
              << ", Forcing Index: " << dec_fest.forcing_index
              << ", Forced R: " << dec_fest.forced_r
              << ", Organic R: " << dec_fest.organic_r
              << " (Driver: " << dec_fest.primary_forcing_driver << ")\n";
    assert(dec_fest.forcing_index > 0.85);

    std::cout << "  Passed: Common-Forcing Model cleanly separated environmental drivers!\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  TAAL - LAYER 1: MEASURE SYNCHRONY TEST SUITE          \n";
    std::cout << "========================================================\n\n";

    test_kuramoto_dpi();
    test_pacemaker_audit();
    test_cascade_lag_map();
    test_common_forcing();

    std::cout << "\n========================================================\n";
    std::cout << "  ALL LAYER 1 TESTS PASSED SUCCESSFULLY!                \n";
    std::cout << "========================================================\n";
    return 0;
}

