#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>

namespace taal::forecast {

struct NodeAdvisory {
    std::string node_id;
    std::string node_name;
    double capacity;
    double organic_unmanaged_demand;
    double published_congestion_score; // 0.0 (empty) to 1.0 (severely congested)
    double expected_visitor_equilibrium;
    bool is_spillover_risk;
};

struct ReflexiveForecastResult {
    bool converged{false};
    int iterations{0};
    double max_equilibrium_drift{0.0};
    std::vector<NodeAdvisory> nodes;
    std::string advisory_summary;
};

class ReflexiveForecast {
public:
    ReflexiveForecast();

    // Register corridor attraction nodes (e.g., Red Fort, India Gate, Humayun's Tomb, Sunder Nursery)
    void add_node(const std::string& id, const std::string& name, double capacity, double initial_demand);

    // Compute standard naive forecast (ignores visitor behavioral elasticity - causes herd rebound)
    std::vector<NodeAdvisory> compute_naive_forecast() const;

    // Compute TAAL Reflexive Fixed-Point Forecast x* = B(Phi(x*))
    ReflexiveForecastResult compute_fixed_point_forecast(double elasticity = 0.65,
                                                         double damping = 0.35,
                                                         int max_iters = 100,
                                                         double tolerance = 1e-4) const;

    void clear();

private:
    struct InternalNode {
        std::string id;
        std::string name;
        double capacity;
        double initial_demand;
    };
    std::vector<InternalNode> nodes_;
};

} // namespace taal::forecast

