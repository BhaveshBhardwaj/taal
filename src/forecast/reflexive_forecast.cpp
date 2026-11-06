#include "forecast/reflexive_forecast.hpp"
#include <cmath>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace taal::forecast {

ReflexiveForecast::ReflexiveForecast() {
    // Seed default Delhi nodes
    add_node("india_gate", "India Gate Central Vista", 4000.0, 7500.0);
    add_node("red_fort", "Red Fort Heritage Precinct", 3500.0, 4800.0);
    add_node("sunder_nursery", "Sunder Nursery Heritage Park", 2500.0, 900.0);
    add_node("humayuns_tomb", "Humayun's Tomb Mughal Gardens", 2000.0, 800.0);
    add_node("lodhi_garden", "Lodhi Garden Green Corridor", 3000.0, 1100.0);
}

void ReflexiveForecast::add_node(const std::string& id, const std::string& name, double capacity, double initial_demand) {
    nodes_.push_back({id, name, capacity, initial_demand});
}

void ReflexiveForecast::clear() {
    nodes_.clear();
}

std::vector<NodeAdvisory> ReflexiveForecast::compute_naive_forecast() const {
    // Naive forecast: reports raw demand/capacity ratio as congestion.
    // In reality, this causes 100% of elastic users to run away from high-score nodes and crush low-score nodes!
    std::vector<NodeAdvisory> results;
    for (const auto& n : nodes_) {
        double score = n.initial_demand / n.capacity;
        if (score > 1.0) score = 1.0;
        results.push_back({
            n.id,
            n.name,
            n.capacity,
            n.initial_demand,
            score,
            n.initial_demand,
            (n.initial_demand > n.capacity)
        });
    }
    return results;
}

ReflexiveForecastResult ReflexiveForecast::compute_fixed_point_forecast(double elasticity,
                                                                        double damping,
                                                                        int max_iters,
                                                                        double tolerance) const {
    size_t m = nodes_.size();
    if (m == 0) return {false, 0, 0.0, {}, "No nodes defined"};

    // Initial visitor distribution x^(0)
    std::vector<double> x(m);
    double total_visitors = 0.0;
    for (size_t i = 0; i < m; ++i) {
        x[i] = nodes_[i].initial_demand;
        total_visitors += x[i];
    }

    // Guidance score vector g in [0, 1]
    std::vector<double> g(m, 0.5);
    for (size_t i = 0; i < m; ++i) {
        g[i] = std::clamp(x[i] / (nodes_[i].capacity * 1.5), 0.0, 1.0);
    }

    int iter = 0;
    double max_drift = 0.0;
    bool converged = false;

    while (iter < max_iters) {
        iter++;
        max_drift = 0.0;

        // 1. Behavioral response to current published guidance g:
        // Visitors divert away from high congestion guidance towards lower congestion guidance
        double avg_guidance = std::accumulate(g.begin(), g.end(), 0.0) / static_cast<double>(m);

        std::vector<double> x_response(m);
        double total_diverted = 0.0;

        // Calculate elastic shifting
        std::vector<double> shifts(m, 0.0);
        for (size_t i = 0; i < m; ++i) {
            // Sensitivity relative to average corridor advisory
            double rel_signal = g[i] - avg_guidance;
            double elastic_shift = elasticity * nodes_[i].initial_demand * rel_signal;
            shifts[i] = elastic_shift;
            total_diverted += elastic_shift;
        }

        // Apply conservation of visitors: shifted users are redistributed to lower guidance nodes
        double rebalance_per_node = total_diverted / static_cast<double>(m);
        for (size_t i = 0; i < m; ++i) {
            x_response[i] = nodes_[i].initial_demand - shifts[i] + rebalance_per_node;
            if (x_response[i] < 100.0) x_response[i] = 100.0; // minimum baseline
        }

        // Rescale to maintain exact total visitor population
        double current_sum = std::accumulate(x_response.begin(), x_response.end(), 0.0);
        for (size_t i = 0; i < m; ++i) {
            x_response[i] = (x_response[i] / current_sum) * total_visitors;
        }

        // 2. Fixed point update with damping:
        // g_new = (1 - alpha)*g + alpha * (x_response / capacity)
        for (size_t i = 0; i < m; ++i) {
            double target_g = std::clamp(x_response[i] / (nodes_[i].capacity * 1.2), 0.0, 1.0);
            double g_next = (1.0 - damping) * g[i] + damping * target_g;
            double drift = std::abs(g_next - g[i]);
            if (drift > max_drift) max_drift = drift;
            g[i] = g_next;
            x[i] = x_response[i];
        }

        if (max_drift < tolerance) {
            converged = true;
            break;
        }
    }

    std::vector<NodeAdvisory> advisories;
    std::ostringstream ss;
    ss << "Reflexive Fixed-Point Forecast (" << (converged ? "Converged" : "Damped")
       << " in " << iter << " iters, drift=" << std::scientific << std::setprecision(2) << max_drift << "):\n";

    for (size_t i = 0; i < m; ++i) {
        bool spillover = (x[i] > nodes_[i].capacity);
        advisories.push_back({
            nodes_[i].id,
            nodes_[i].name,
            nodes_[i].capacity,
            nodes_[i].initial_demand,
            g[i],
            x[i],
            spillover
        });

        ss << "  " << std::left << std::setw(30) << nodes_[i].name
           << " Init Demand: " << std::setw(6) << static_cast<int>(nodes_[i].initial_demand)
           << " -> Eq. Equilibrium: " << std::setw(6) << static_cast<int>(x[i])
           << " (Cap: " << std::setw(5) << static_cast<int>(nodes_[i].capacity) << ")"
           << " Advisory Score: " << std::fixed << std::setprecision(2) << g[i]
           << (spillover ? " [OVERLOAD BUFFERED]" : " [BALANCED]") << "\n";
    }

    return {converged, iter, max_drift, advisories, ss.str()};
}

} // namespace taal::forecast

