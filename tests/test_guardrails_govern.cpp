#include "guardrails/resident_shield.hpp"
#include "guardrails/regret_and_herding.hpp"
#include "guardrails/privacy_dpdp.hpp"
#include "govern/mohalla_council.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace taal::guardrails;
using namespace taal::govern;

void test_resident_shield() {
    std::cout << "[TEST] 1. Resident Shield Auto-Veto Verification (Feature 16)...\n";
    ResidentShield shield;

    // Test Case A: Commercial arterial midday nudge -> Should be APPROVED
    int64_t midday_t = 50400; // 14:00:00
    auto dec_arterial = shield.evaluate_nudge("netaji_subhash_marg", 200.0, midday_t);
    std::cout << "  Case A (Arterial Midday Nudge): " << dec_arterial.reason << "\n";
    assert(!dec_arterial.is_vetoed);
    assert(dec_arterial.resident_burden_score == 0.0);

    // Test Case B: Redirecting tourist footfall into residential Ballimaran alley at 23:00 (Night hour)
    int64_t night_t = 82800; // 23:00:00
    auto dec_night = shield.evaluate_nudge("ballimaran_residential", 150.0, night_t);
    std::cout << "  Case B (Residential Night Nudge at 23:00): " << dec_night.reason << "\n";
    assert(dec_night.is_vetoed);
    assert(dec_night.violated_constraint == "NIGHT_HOURS_RESIDENTIAL_PROTECTION");

    // Test Case C: Dense footfall surge into Dariba Kalan exceeding safe residential density
    auto dec_density = shield.evaluate_nudge("dariba_kalan_residential", 400.0, midday_t);
    std::cout << "  Case C (Overload into Dariba Kalan): " << dec_density.reason << "\n";
    assert(dec_density.is_vetoed);
    assert(dec_density.violated_constraint == "RESIDENTIAL_DENSITY_EXCEEDED");

    std::cout << "  Passed: Resident Shield strictly auto-vetoes any negative burden on residents!\n";
}

void test_regret_and_herding() {
    std::cout << "\n[TEST] 2. Regret Budget & Herding Detector (Features 17 & 18)...\n";
    
    // Feature 17: Regret Budget & Swap-Back Guarantee
    RegretBudget regret(0.30);
    int64_t now_t = 72000;
    auto token = regret.issue_nudge_guarantee("visitor_ananya", "red_fort", "sunder_nursery", now_t);
    std::cout << "  Swap-Back Token Issued: " << token.token_id
              << ", Priority Gate: " << token.priority_gate_code
              << ", Regret Bound: " << token.max_regret_bound << "\n";
    assert(regret.get_active_tokens_count() == 1);

    // Redeem swap-back
    bool redeemed = regret.redeem_swap_back(token.token_id, now_t + 1800);
    assert(redeemed);
    assert(regret.get_active_tokens_count() == 0);
    std::cout << "  Swap-back redeemed: Visitor seamlessly transferred back with priority!\n";

    // Feature 18: Herding Detector & Automated Kill-Switch
    HerdingDetector detector(0.38, 0.40);

    // Scenario A: Balanced adoption across 5 alternatives
    detector.record_adoption("sunder_nursery");
    detector.record_adoption("sunder_nursery");
    detector.record_adoption("humayuns_tomb");
    detector.record_adoption("humayuns_tomb");
    detector.record_adoption("purana_qila");
    detector.record_adoption("lodhi_garden");
    detector.record_adoption("safdarjung_tomb");

    auto balanced_state = detector.evaluate_herding();
    std::cout << "  Balanced Scenario: " << balanced_state.status_summary << "\n";
    assert(!balanced_state.is_kill_switch_active);

    // Scenario B: Excessive concentration (herding surge onto Sunder Nursery)
    for (int i = 0; i < 30; ++i) {
        detector.record_adoption("sunder_nursery");
    }

    auto herded_state = detector.evaluate_herding();
    std::cout << "  Concentrated Scenario: " << herded_state.status_summary << "\n";
    assert(herded_state.is_kill_switch_active);
    assert(herded_state.hot_alternative_id == "sunder_nursery");
    assert(detector.is_destination_blocked("sunder_nursery"));

    std::cout << "  Passed: Herding Detector tripped automated kill-switch on secondary surge!\n";
}

void test_privacy_dpdp() {
    std::cout << "\n[TEST] 3. Privacy by Design & DPDP Act 2023 (Feature 19)...\n";
    PrivacyDPDP privacy(0.5, 42);

    TelemetryBatch raw_batch = {
        "chandni_chowk_gate_2",
        1250,
        2.45,
        75600
    };

    auto sanitized = privacy.process_telemetry(raw_batch);
    std::cout << "  Raw Count: " << raw_batch.raw_headcount
              << " -> Differential Privacy Count: " << sanitized.obfuscated_headcount
              << " (Epsilon: " << sanitized.privacy_epsilon_used << ")\n"
              << "  Raw Density: " << raw_batch.raw_density
              << " -> Differential Privacy Density: " << sanitized.obfuscated_density << "\n"
              << "  DPDP Token: " << sanitized.dpdp_compliance_hash << "\n";

    assert(privacy.verify_audit_token(sanitized));
    assert(sanitized.is_dpdp_compliant);
    assert(std::abs(sanitized.obfuscated_headcount - raw_batch.raw_headcount) < 25);
    std::cout << "  Passed: Differential privacy preserves statistical utility while ensuring mathematical DPDP compliance!\n";
}

void test_mohalla_council() {
    std::cout << "\n[TEST] 4. Mohalla Council Governance & Revenue Share (Feature 20)...\n";
    MohallaCouncil council;

    // Check initialized members
    assert(council.get_members().size() == 5);

    // Proposal 1: Staggered Night Food Walk along Chatta Chowk
    std::string p1 = council.submit_policy(
        "Staggered Night Food Walk",
        "Introduce 4 wave culinary walk along Chatta Chowk with local merchant credits",
        "chandni_chowk_pedestrian"
    );

    // Vote: 3 approvals -> Ratified!
    council.cast_vote(p1, "traders_chandni_chowk", true);
    council.cast_vote(p1, "traders_fatehpuri", true);
    council.cast_vote(p1, "erickshaw_union", true);

    auto prop1 = council.get_proposal(p1);
    assert(prop1->is_ratified);
    std::cout << "  Proposal 1 Status: " << prop1->resolution_notes << "\n";

    // Proposal 2: Route Tourist Shuttles through Ballimaran Residential Lane
    std::string p2 = council.submit_policy(
        "Ballimaran Alley Shuttle Bypass",
        "Divert e-rickshaws through Gali Qasim Jan to bypass main road traffic",
        "ballimaran_residential"
    );

    // RWA exercises binding veto!
    bool vetoed = council.exercise_council_veto(p2, "rwa_ballimaran", "Severe residential disturbance and lane congestion");
    assert(vetoed);
    auto prop2 = council.get_proposal(p2);
    assert(prop2->is_vetoed);
    assert(!prop2->is_ratified);
    std::cout << "  Proposal 2 Status: " << prop2->resolution_notes << "\n";

    // Revenue Share: 15% of Phase Credit turnover deposited into Mohalla Community Fund
    council.deposit_revenue_share(500000.0); // Rs 5 Lakhs economic circulation
    const auto& ledger = council.get_fund_ledger();
    std::cout << "  Mohalla Community Fund Balance (15% statutory share): Rs "
              << ledger.fund_balance_inr << " INR\n";
    assert(ledger.fund_balance_inr == 75000.0);

    // Disburse funds for community solar alley lighting
    bool disbursed = council.disburse_fund("Ballimaran Heritage Solar Streetlights", 30000.0);
    assert(disbursed);
    assert(council.get_fund_ledger().fund_balance_inr == 45000.0);
    std::cout << "  Fund disbursed: Rs 30,000 for Ballimaran Solar Streetlights. Remaining: Rs "
              << council.get_fund_ledger().fund_balance_inr << "\n";

    std::cout << "  Passed: Mohalla Council holds true democratic veto rights and revenue sovereignty!\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "  TAAL - LAYERS 4 & 5: GUARDRAILS & GOVERNANCE SUITE    \n";
    std::cout << "========================================================\n\n";

    test_resident_shield();
    test_regret_and_herding();
    test_privacy_dpdp();
    test_mohalla_council();

    std::cout << "\n========================================================\n";
    std::cout << "  ALL LAYERS 4 & 5 TESTS PASSED SUCCESSFULLY!           \n";
    std::cout << "========================================================\n";
    return 0;
}

