#include "measure/pacemaker_audit.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace taal::measure {

PacemakerAudit::PacemakerAudit() {
    load_delhi_canonical_pacemakers();
}

void PacemakerAudit::register_pacemaker(const Pacemaker& pm) {
    pacemakers_.push_back(pm);
}

void PacemakerAudit::load_delhi_canonical_pacemakers() {
    pacemakers_.clear();

    // 1. Red Fort Sound & Light Show End (21:00 = 75600s)
    pacemakers_.push_back({
        "red_fort_sound_light",
        "Red Fort Sound & Light Show End",
        "Chandni Chowk / Red Fort",
        PacemakerType::MONUMENT_SHOW,
        75600, // 21:00:00
        2400,  // +/- 20 min window
        seconds_to_phase(75600),
        0.0, 0, 0.0,
        "Exit-as-Experience: Split into 4 color waves with sherbet/heritage buffer zones"
    });

    // 2. Akshardham Water Show End (20:15 = 72900s)
    pacemakers_.push_back({
        "akshardham_water_show",
        "Akshardham Sahaj Anand Water Show End",
        "Akshardham Complex",
        PacemakerType::MONUMENT_SHOW,
        72900, // 20:15:00
        1800,  // +/- 15 min window
        seconds_to_phase(72900),
        0.0, 0, 0.0,
        "Staggered exit ticketing with interactive laser fountain delay exhibits"
    });

    // 3. Hotel Checkout Rush (11:30 = 41400s)
    pacemakers_.push_back({
        "hotel_checkout_noon",
        "Hotel Noon Checkout Sync",
        "Paharganj / Aerocity",
        PacemakerType::HOTEL_CHECKOUT,
        41400, // 11:30:00
        3600,  // +/- 30 min window
        seconds_to_phase(41400),
        0.0, 0, 0.0,
        "Free-Hands Checkout: Rolling checkout bands + Metro luggage lockers"
    });

    // 4. India Gate Evening Heat Break (19:00 = 68400s)
    pacemakers_.push_back({
        "india_gate_heat_break",
        "India Gate Post-Heat Arrival",
        "Central Vista / India Gate",
        PacemakerType::WEATHER_BREAK,
        68400, // 19:00:00
        5400,  // +/- 45 min window
        seconds_to_phase(68400),
        0.0, 0, 0.0,
        "Polyrhythm Menu Twin Moments: Spread across Sunder Nursery & Lodhi Gardens"
    });

    // 5. Monument Sunset Closing (17:45 = 63900s)
    pacemakers_.push_back({
        "monuments_sunset_close",
        "Monuments Sunset Gate Closures",
        "Delhi ASI Monuments",
        PacemakerType::MONUMENT_CLOSING,
        63900, // 17:45:00
        2400,  // +/- 20 min window
        seconds_to_phase(63900),
        0.0, 0, 0.0,
        "Flexible twilight opening hours under solar LED illumination"
    });

    // 6. Evening Aarti / Prayers (19:15 = 69300s)
    pacemakers_.push_back({
        "yamuna_ghat_aarti",
        "Yamuna Ghat & Temple Evening Aarti",
        "Kashmere Gate / Nigambodh",
        PacemakerType::RELIGIOUS_CEREMONY,
        69300, // 19:15:00
        1800,
        seconds_to_phase(69300),
        0.0, 0, 0.0,
        "Distributed neighborhood aarti webcasting and staggered ghat access"
    });

    // 7. Last Metro Departures (23:15 = 83700s)
    pacemakers_.push_back({
        "last_metro_rush",
        "Metro Final Train Curfew Rush",
        "Chandni Chowk / Kashmere Gate Metro",
        PacemakerType::TRANSIT_CURFEW,
        83700, // 23:15:00
        2400,
        seconds_to_phase(83700),
        0.0, 0, 0.0,
        "Demand-responsive last-mile night feeder shuttles to ease final train stampede"
    });
}

void PacemakerAudit::record_decision(int64_t timestamp_sec, const std::string& near_location) {
    observations_.push_back({timestamp_sec, near_location});
}

void PacemakerAudit::audit_and_rank() {
    for (auto& pm : pacemakers_) {
        double sum_cos = 0.0;
        size_t count = 0;
        int64_t half_window = pm.window_span_sec / 2;
        int64_t w_start = pm.nominal_time_sec - half_window;
        int64_t w_end = pm.nominal_time_sec + half_window;

        for (const auto& obs : observations_) {
            int64_t t_day = obs.first % SECONDS_PER_DAY;
            if (t_day < 0) t_day += SECONDS_PER_DAY;

            if (t_day >= w_start && t_day <= w_end) {
                // If location matches or is general
                if (obs.second.empty() || pm.location.find(obs.second) != std::string::npos ||
                    obs.second.find("all") != std::string::npos) {
                    double phase = seconds_to_phase(t_day);
                    double phase_diff = phase - pm.phase_target;
                    sum_cos += std::cos(phase_diff);
                    count++;
                }
            }
        }

        pm.observed_visitors = count;
        if (count > 0) {
            double avg_cos = sum_cos / static_cast<double>(count);
            pm.coupling_strength = std::clamp(avg_cos, 0.0, 1.0);
            pm.peak_surge_rate = static_cast<double>(count) / (static_cast<double>(pm.window_span_sec) / 60.0);
        } else {
            pm.coupling_strength = 0.0;
            pm.peak_surge_rate = 0.0;
        }
    }

    // Rank descending by coupling strength * visitors
    std::sort(pacemakers_.begin(), pacemakers_.end(), [](const Pacemaker& a, const Pacemaker& b) {
        if (std::abs(a.coupling_strength - b.coupling_strength) > 0.02) {
            return a.coupling_strength > b.coupling_strength;
        }
        return a.observed_visitors > b.observed_visitors;
    });
}

const std::vector<Pacemaker>& PacemakerAudit::get_ranked_pacemakers() const {
    return pacemakers_;
}

const Pacemaker* PacemakerAudit::get_pacemaker(const std::string& id) const {
    for (const auto& pm : pacemakers_) {
        if (pm.id == id) return &pm;
    }
    return nullptr;
}

std::string PacemakerAudit::generate_audit_report() const {
    std::ostringstream ss;
    ss << "========================================================================================\n"
       << "                           TAAL PACEMAKER AUDIT REPORT                                  \n"
       << "========================================================================================\n"
       << std::left << std::setw(28) << "Pacemaker Clock"
       << std::setw(12) << "Time"
       << std::setw(12) << "Lock (Kp)"
       << std::setw(10) << "Count"
       << std::setw(14) << "Surge/min"
       << "Structural Redesign Action\n"
       << "----------------------------------------------------------------------------------------\n";

    for (const auto& pm : pacemakers_) {
        ss << std::left << std::setw(28) << pm.name.substr(0, 27)
           << std::setw(12) << format_time_hhmmss(pm.nominal_time_sec)
           << std::fixed << std::setprecision(3) << std::setw(12) << pm.coupling_strength
           << std::setw(10) << pm.observed_visitors
           << std::setprecision(1) << std::setw(14) << pm.peak_surge_rate
           << pm.redesign_action << "\n";
    }
    ss << "========================================================================================\n";
    return ss.str();
}

} // namespace taal::measure

