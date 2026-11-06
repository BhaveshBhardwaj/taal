#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <memory>

namespace taal::measure {

enum class PacemakerType {
    MONUMENT_SHOW,     // Sound & Light show, water show
    MONUMENT_CLOSING,  // Sunset gate closures
    HOTEL_CHECKOUT,    // Noon checkout synchronizer
    RELIGIOUS_CEREMONY,// Aarti or prayer
    TRANSIT_CURFEW,    // Last metro train
    WEATHER_BREAK      // Evening heat break
};

struct Pacemaker {
    std::string id;
    std::string name;
    std::string location;
    PacemakerType type;
    int64_t nominal_time_sec; // Seconds from midnight
    int64_t window_span_sec;  // Capture window span
    double phase_target;      // Target phase theta* in [0, 2*PI)
    
    // Audit metrics
    double coupling_strength{0.0}; // K_p in [0, 1]
    size_t observed_visitors{0};
    double peak_surge_rate{0.0};  // visitors per minute
    std::string redesign_action;  // Structural redesign suggestion
};

class PacemakerAudit {
public:
    PacemakerAudit();

    // Register a custom pacemaker
    void register_pacemaker(const Pacemaker& pm);

    // Seed standard Delhi pacemakers from the specification
    void load_delhi_canonical_pacemakers();

    // Feed observed decision timestamp
    void record_decision(int64_t timestamp_sec, const std::string& near_location = "");

    // Compute coupling strength K_p for all registered pacemakers
    void audit_and_rank();

    // Retrieve all pacemakers ranked by coupling strength descending
    const std::vector<Pacemaker>& get_ranked_pacemakers() const;

    // Get specific pacemaker by ID
    const Pacemaker* get_pacemaker(const std::string& id) const;

    // Output formatted report
    std::string generate_audit_report() const;

private:
    std::vector<Pacemaker> pacemakers_;
    std::vector<std::pair<int64_t, std::string>> observations_; // timestamp, location
};

} // namespace taal::measure

