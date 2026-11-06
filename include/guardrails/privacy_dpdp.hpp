#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <random>

namespace taal::guardrails {

struct TelemetryBatch {
    std::string zone_id;
    int raw_headcount;
    double raw_density;
    int64_t timestamp_sec;
};

struct SanitizedTelemetry {
    std::string zone_id;
    int obfuscated_headcount;
    double obfuscated_density;
    double privacy_epsilon_used;
    std::string dpdp_compliance_hash;
    bool is_dpdp_compliant;
};

class PrivacyDPDP {
public:
    explicit PrivacyDPDP(double epsilon = 0.5, uint32_t seed = 42);

    // Apply Laplace Differential Privacy to crowd count
    int privatize_count(int true_count);

    // Apply Laplace Differential Privacy to density
    double privatize_density(double true_density);

    // Ingest and sanitize batch data under DPDP Act 2023
    SanitizedTelemetry process_telemetry(const TelemetryBatch& raw);

    // Verify compliance status
    bool verify_audit_token(const SanitizedTelemetry& data) const;

private:
    double epsilon_;
    std::mt19937 rng_;
};

} // namespace taal::guardrails

