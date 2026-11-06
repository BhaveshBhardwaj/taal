#include "transit_tourism/dmrc_dual_station.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace taal::transit_tourism {

DmrcDualStation::DmrcDualStation() {
    yellow_station_ = {
        "chandni_chowk_yellow",
        "Chandni Chowk Metro Station",
        "Yellow Line (HUDA City Centre - Samaypur Badli)",
        2.8,  // Overcrowded baseline
        2.5,
        3.5,  // 3.5 min headway
        750.0,// 750m walk
        480   // 8 min queue
    };

    violet_station_ = {
        "lal_qila_violet",
        "Lal Qila Metro Station",
        "Violet Line (Kashmere Gate - Raja Nahar Singh)",
        0.8,  // Low density baseline
        2.5,
        4.5,  // 4.5 min headway
        380.0,// Only 380m walk (closer!)
        90    // 1.5 min queue
    };
}

void DmrcDualStation::update_station_telemetry(const std::string& station_id,
                                              double platform_density,
                                              double headway_min,
                                              int queue_wait_sec) {
    if (station_id == "chandni_chowk_yellow" || station_id == "chandni_chowk") {
        yellow_station_.platform_density_p_m2 = platform_density;
        yellow_station_.headway_minutes = headway_min;
        yellow_station_.queue_wait_seconds = queue_wait_sec;
    } else if (station_id == "lal_qila_violet" || station_id == "lal_qila") {
        violet_station_.platform_density_p_m2 = platform_density;
        violet_station_.headway_minutes = headway_min;
        violet_station_.queue_wait_seconds = queue_wait_sec;
    }
}

TransitSplitRecommendation DmrcDualStation::evaluate_transit_split(int incoming_exit_crowd) const {
    double density_diff = yellow_station_.platform_density_p_m2 - violet_station_.platform_density_p_m2;
    
    // Sigmoid load splitting function
    double gamma = 1.8;
    double split_violet = 1.0 / (1.0 + std::exp(-gamma * density_diff));
    split_violet = std::clamp(split_violet, 0.15, 0.85);

    bool crush_active = (yellow_station_.platform_density_p_m2 > yellow_station_.max_safe_density_p_m2);
    int incentive_credits = 0;
    std::string rec_id = yellow_station_.station_id;
    std::string rec_name = yellow_station_.name;

    if (crush_active || split_violet > 0.50) {
        rec_id = violet_station_.station_id;
        rec_name = violet_station_.name;
        incentive_credits = 40; // ₹40 Phase Credit reward for using Lal Qila station
    }

    int diverted_passengers = static_cast<int>(incoming_exit_crowd * split_violet);

    std::ostringstream ss_en, ss_hi;
    ss_en << "DMRC Smart Transit: Chandni Chowk Yellow Line is at "
          << std::fixed << std::setprecision(1) << yellow_station_.platform_density_p_m2
          << " p/m² (Wait: " << (yellow_station_.queue_wait_seconds / 60) << "m). "
          << "Walk 380m to Lal Qila Metro (Violet Line) for 1-minute boarding + earn "
          << incentive_credits << " Phase Credits!";

    ss_hi << "डीएमआरसी स्मार्ट निर्देश: चाँदनी चौक मेट्रो (येलो लाइन) पर भारी भीड़ है। "
          << "कृपया केवल 380 मीटर दूर लाल किला मेट्रो (वायलेट लाइन) का उपयोग करें और तुरंत प्रवेश पाएं + "
          << incentive_credits << " फ़ेज़ क्रेडिट्स प्राप्त करें!";

    return {
        rec_id,
        rec_name,
        split_violet * 100.0,
        incentive_credits,
        ss_en.str(),
        ss_hi.str(),
        crush_active
    };
}

} // namespace taal::transit_tourism

