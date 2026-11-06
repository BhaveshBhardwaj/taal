#include "desync/itinerary_charging_lowtech.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace taal::desync {

ItineraryEmbeddedCharging::ItineraryEmbeddedCharging(double battery_capacity_kwh, double max_discharge_kw)
    : capacity_kwh_(battery_capacity_kwh),
      soc_kwh_(battery_capacity_kwh * 0.90), // Starts 90% charged
      max_discharge_kw_(max_discharge_kw),
      current_discharge_kw_(0.0) {}

BatterySwapEvent ItineraryEmbeddedCharging::schedule_swap_relay(const std::string& vehicle_id,
                                                               int64_t show_exit_sec,
                                                               double current_soc_pct) {
    // Cascade Lag Map: rickshaw recharge demand spikes at +50 min (3000s) from show exit
    int64_t swap_time = show_exit_sec + 3000;
    double kw_avoided = 3.5; // Avoids 3.5 kW fast-charger grid spike per vehicle

    return {
        vehicle_id,
        "Chandni Chowk Metro Kiosk #2",
        swap_time,
        current_soc_pct,
        98.0, // Swapped to fully fresh battery
        kw_avoided
    };
}

CommunityBatteryStatus ItineraryEmbeddedCharging::trigger_peak_shaving(double incoming_feeder_kva,
                                                                       double transformer_limit_kva) {
    if (incoming_feeder_kva > transformer_limit_kva) {
        double deficit = incoming_feeder_kva - transformer_limit_kva;
        current_discharge_kw_ = std::min(max_discharge_kw_, deficit);
        is_discharging_ = (current_discharge_kw_ > 0.0);
        total_shaved_kwh_ += (current_discharge_kw_ * 0.25); // for 15-min interval
        soc_kwh_ = std::max(0.0, soc_kwh_ - (current_discharge_kw_ * 0.25));
    } else {
        current_discharge_kw_ = 0.0;
        is_discharging_ = false;
    }

    return {
        capacity_kwh_,
        soc_kwh_,
        max_discharge_kw_,
        current_discharge_kw_,
        is_discharging_,
        current_discharge_kw_
    };
}

void ItineraryEmbeddedCharging::recharge_community_battery(double kwh) {
    soc_kwh_ = std::min(capacity_kwh_, soc_kwh_ + kwh);
}

CommunityBatteryStatus ItineraryEmbeddedCharging::get_battery_status() const {
    return {
        capacity_kwh_,
        soc_kwh_,
        max_discharge_kw_,
        current_discharge_kw_,
        is_discharging_,
        total_shaved_kwh_
    };
}

std::string LowTechChannels::format_ticket_stub(int ticket_id, const std::string& color_wave,
                                                int64_t scheduled_exit_sec, const std::string& perk) {
    std::ostringstream ss;
    ss << "┌────────────────────────────────────────────────────────┐\n"
       << "│              DMRC & ASI - TAAL TICKETING               │\n"
       << "│ TICKET #" << std::setw(6) << ticket_id << "   EXIT WAVE: [" << std::setw(14) << color_wave << "]      │\n"
       << "│ Scheduled Exit: " << format_time_hhmmss(scheduled_exit_sec) << "                                │\n"
       << "│ Buffer Perk: " << std::left << std::setw(42) << perk.substr(0, 41) << "│\n"
       << "│ * Show this colored stub at Lahore Gate for priority   │\n"
       << "└────────────────────────────────────────────────────────┘\n";
    return ss.str();
}

std::vector<BilingualMessage> LowTechChannels::generate_pa_announcements(const std::string& active_wave,
                                                                         const std::string& venue) {
    std::vector<BilingualMessage> msgs;

    // Hindi announcement
    std::string hi = "कृपया ध्यान दें: " + venue + " के दर्शकगण जिनके पास " + active_wave +
                     " रंग का टिकट है, वे कृपया लाहौरी गेट निकास द्वार की ओर प्रस्थान करें। अन्य दर्शक कृपया छत्ता चौक में ठंडे शरबत का आनंद लें।";

    // English announcement
    std::string en = "Attention please: Visitors at " + venue + " holding " + active_wave +
                     " tickets may now proceed towards Lahore Gate exit. Guests in subsequent waves, please enjoy heritage walkways and refreshments.";

    msgs.push_back({"hi", "PA_SYSTEM", hi, 0});
    msgs.push_back({"en", "PA_SYSTEM", en, 0});
    return msgs;
}

std::string LowTechChannels::handle_bot_query(const std::string& query, const std::string& lang) {
    if (lang == "hi" || query.find("लाल किला") != std::string::npos || query.find("कहाँ") != std::string::npos) {
        return "नमस्ते! TAAL सेवा में आपका स्वागत है। लाल किला शो के बाद सुंदर नर्सरी या हुमायूँ के मकबरे पर जाएँ, जहाँ कोई भीड़ नहीं है। चाँदनी चौक मेट्रो गेट 2 खुला है। फ़्री हैंड्स लॉकर के लिए 1 दबाएँ।";
    }
    return "Welcome to TAAL! Red Fort exit wave active. Visit Sunder Nursery or Humayun's Tomb for tranquil evening views. Metro Gate 2 is clear. Reply 1 for Metro luggage locker voucher.";
}

BilingualMessage LowTechChannels::dispatch_captain_directive(const std::string& stand_name,
                                                            int incoming_passengers,
                                                            int staggered_wave_min) {
    std::ostringstream ss;
    ss << "[RICKSHAW STAND CAPTAIN DIRECTIVE: " << stand_name << "] "
       << incoming_passengers << " passengers arriving across 4 waves staggered by "
       << staggered_wave_min << " mins. Deploy 60 e-rickshaws to Gate 2. Battery swap hub active at Mori Gate.";
    return {"en", "RICKSHAW_CAPTAIN", ss.str(), 0};
}

} // namespace taal::desync

