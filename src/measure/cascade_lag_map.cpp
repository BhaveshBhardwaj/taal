#include "measure/cascade_lag_map.hpp"
#include <cmath>
#include <queue>
#include <algorithm>
#include <sstream>

namespace taal::measure {

CascadeLagMap::CascadeLagMap() {
    setup_delhi_chandni_chowk_corridor();
}

void CascadeLagMap::setup_delhi_chandni_chowk_corridor() {
    nodes_.clear();
    edges_.clear();

    // 1. Red Fort Sound & Light Show Exit
    nodes_.push_back({
        "red_fort_show",
        "Red Fort Sound & Light Show Grounds",
        "monument",
        5000.0, 0.0, 0.0, 4500.0, false
    });

    // 2. Lahore Gate & Chatta Chowk bottleneck
    nodes_.push_back({
        "lahore_gate_crush",
        "Lahore Gate Bottleneck (+8m)",
        "gate",
        180.0, // max throughput persons/min
        0.0, 0.0, 160.0, false
    });

    // 3. Chandni Chowk Metro & E-Rickshaw Stand
    nodes_.push_back({
        "erickshaw_metro_queue",
        "Chandni Chowk Metro & Rickshaw Stand (+20m)",
        "transit",
        220.0, // boarding capacity persons/min
        0.0, 0.0, 200.0, false
    });

    // 4. EV Charging Hubs
    nodes_.push_back({
        "ev_charging_hub",
        "Chandni Chowk EV & Rickshaw Fast Chargers (+50m)",
        "ev_charging",
        650.0, // kW capacity
        150.0, 150.0, 580.0, false
    });

    // 5. Feeder Transformer
    nodes_.push_back({
        "feeder_transformer_11kv",
        "11kV Distribution Feeder Transformer (+90m)",
        "grid_feeder",
        1000.0, // kVA capacity
        420.0, 420.0, 750.0, false
    });

    // Edges with exact PDF delays:
    // show ends -> +8 min gate crush -> +20 min e-rickshaw queue -> +50 min charger spike -> +90 min feeder load
    edges_.push_back({
        "red_fort_show",
        "lahore_gate_crush",
        8.0 * 60.0, // 480 seconds (8 min)
        120.0,
        0.95 // 95% pass through Lahore Gate
    });

    edges_.push_back({
        "lahore_gate_crush",
        "erickshaw_metro_queue",
        12.0 * 60.0, // 720 seconds (+12 min -> cumulative +20 min)
        180.0,
        0.85 // 85% head to Chandni Chowk metro & e-rickshaws
    });

    edges_.push_back({
        "erickshaw_metro_queue",
        "ev_charging_hub",
        30.0 * 60.0, // 1800 seconds (+30 min -> cumulative +50 min)
        360.0,
        0.16 // kW fast-charging power demand induced per passenger
    });

    edges_.push_back({
        "ev_charging_hub",
        "feeder_transformer_11kv",
        40.0 * 60.0, // 2400 seconds (+40 min -> cumulative +90 min)
        480.0,
        0.95 // kVA transformer draw per kW charging
    });
}

void CascadeLagMap::add_node(const SystemNode& node) {
    nodes_.push_back(node);
}

void CascadeLagMap::add_edge(const CascadeEdge& edge) {
    edges_.push_back(edge);
}

void CascadeLagMap::inject_impulse(const std::string& source_node_id, double magnitude, int64_t timestamp_sec) {
    impulses_.push_back({source_node_id, magnitude, timestamp_sec});
}

double CascadeLagMap::get_cumulative_lag_minutes(const std::string& from_node, const std::string& to_node) const {
    if (from_node == to_node) return 0.0;

    std::unordered_map<std::string, double> min_lag;
    for (const auto& n : nodes_) {
        min_lag[n.id] = 1e9;
    }
    min_lag[from_node] = 0.0;

    // Simple shortest path on DAG
    for (size_t iter = 0; iter < nodes_.size(); ++iter) {
        for (const auto& e : edges_) {
            if (min_lag[e.from_node] < 1e8) {
                double cand = min_lag[e.from_node] + (e.mean_lag_sec / 60.0);
                if (cand < min_lag[e.to_node]) {
                    min_lag[e.to_node] = cand;
                }
            }
        }
    }
    return min_lag[to_node] < 1e8 ? min_lag[to_node] : -1.0;
}

std::unordered_map<std::string, double> CascadeLagMap::predict_loads_at(int64_t target_time_sec) const {
    std::unordered_map<std::string, double> loads;
    for (const auto& node : nodes_) {
        loads[node.id] = node.current_load; // baseline load
    }

    // Gaussian dispersion response for each impulse
    for (const auto& imp : impulses_) {
        // Calculate propagation along edges
        // Map from node_id -> pair(mean_delay_sec, std_dev_sec, cum_fraction)
        std::unordered_map<std::string, std::tuple<double, double, double>> node_responses;
        node_responses[imp.source_id] = {0.0, 60.0, 1.0};

        std::queue<std::string> q;
        q.push(imp.source_id);

        while (!q.empty()) {
            std::string curr = q.front();
            q.pop();

            auto [curr_mean, curr_std, curr_frac] = node_responses[curr];

            for (const auto& edge : edges_) {
                if (edge.from_node == curr) {
                    double nxt_mean = curr_mean + edge.mean_lag_sec;
                    double nxt_std = std::sqrt(curr_std * curr_std + edge.std_lag_sec * edge.std_lag_sec);
                    double nxt_frac = curr_frac * edge.transfer_ratio;

                    node_responses[edge.to_node] = {nxt_mean, nxt_std, nxt_frac};
                    q.push(edge.to_node);
                }
            }
        }

        // Apply Gaussian profile: amplitude * exp(-(t - (t0 + mean))^2 / (2*std^2))
        for (const auto& [nid, resp] : node_responses) {
            auto [mean_lag, std_lag, frac] = resp;
            double peak_time = static_cast<double>(imp.timestamp_sec) + mean_lag;
            double dt = static_cast<double>(target_time_sec) - peak_time;
            double pulse = std::exp(-(dt * dt) / (2.0 * std_lag * std_lag));

            // Added load peak
            double added_load = imp.magnitude * frac * pulse;
            loads[nid] += added_load;
        }
    }

    return loads;
}

std::vector<DownstreamAlert> CascadeLagMap::generate_early_warnings(int64_t current_time_sec) const {
    std::vector<DownstreamAlert> alerts;

    for (const auto& imp : impulses_) {
        for (const auto& node : nodes_) {
            if (node.id == imp.source_id) continue;

            double lag_min = get_cumulative_lag_minutes(imp.source_id, node.id);
            if (lag_min <= 0.0) continue;

            int64_t expected_peak_sec = imp.timestamp_sec + static_cast<int64_t>(lag_min * 60.0);
            int64_t lead_time = expected_peak_sec - current_time_sec;

            if (lead_time > 0) {
                // Predict load at expected peak
                auto loads = predict_loads_at(expected_peak_sec);
                double pred = loads[node.id];
                double headroom = node.overload_threshold - pred;
                bool will_breach = (pred >= node.overload_threshold);

                std::string rec;
                if (node.system_type == "grid_feeder") {
                    rec = will_breach ? "TRIGGER COMMUNITY BATTERY BUFFER & STAGGER FAST CHARGERS"
                                      : "Normal grid headroom maintained";
                } else if (node.system_type == "transit") {
                    rec = will_breach ? "DISPATCH FEEDER BUS SATELLITE TO CHANDNI CHOWK METRO"
                                      : "Transit platform buffer nominal";
                } else if (node.system_type == "gate") {
                    rec = will_breach ? "DEPLOY ROLLING EXIT PROTOCOL & HOLD ZONE B TICKETS"
                                      : "Gate flow within safe density";
                } else {
                    rec = "Monitor downstream queue flow";
                }

                alerts.push_back({
                    expected_peak_sec,
                    lead_time,
                    node.id,
                    node.name,
                    pred,
                    headroom,
                    will_breach,
                    rec
                });
            }
        }
    }

    return alerts;
}

} // namespace taal::measure

