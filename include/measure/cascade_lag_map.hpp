#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

namespace taal::measure {

struct SystemNode {
    std::string id;
    std::string name;
    std::string system_type; // "monument", "gate", "transit", "ev_charging", "grid_feeder"
    double nominal_capacity; // e.g., persons/min, charging kW, transformer kVA
    double current_load;
    double peak_load;
    double overload_threshold;
    bool is_overloaded{false};
};

struct CascadeEdge {
    std::string from_node;
    std::string to_node;
    double mean_lag_sec;    // Mean propagation lag in seconds
    double std_lag_sec;     // Delay jitter standard deviation
    double transfer_ratio;  // Fraction of load moving to downstream node
};

struct DownstreamAlert {
    int64_t expected_peak_time_sec;
    int64_t lead_time_sec;
    std::string target_node_id;
    std::string target_node_name;
    double predicted_shock_load;
    double capacity_headroom;
    bool will_breach_capacity;
    std::string recommendation;
};

class CascadeLagMap {
public:
    CascadeLagMap();

    // Configure corridor nodes and cascade edges
    void setup_delhi_chandni_chowk_corridor();

    void add_node(const SystemNode& node);
    void add_edge(const CascadeEdge& edge);

    // Inject an initial impulse at a source node (e.g. show ends at t0 with 3500 visitors)
    void inject_impulse(const std::string& source_node_id, double magnitude, int64_t timestamp_sec);

    // Propagate shock forward in time and predict loads across all nodes at target_time_sec
    std::unordered_map<std::string, double> predict_loads_at(int64_t target_time_sec) const;

    // Generate grid early warning alerts for DISCOMs & transit operators
    std::vector<DownstreamAlert> generate_early_warnings(int64_t current_time_sec) const;

    // Cumulative lag from source to target in minutes
    double get_cumulative_lag_minutes(const std::string& from_node, const std::string& to_node) const;

    const std::vector<SystemNode>& get_nodes() const { return nodes_; }
    const std::vector<CascadeEdge>& get_edges() const { return edges_; }

private:
    std::vector<SystemNode> nodes_;
    std::vector<CascadeEdge> edges_;
    
    // Stored shock impulses: (source_node, magnitude, timestamp_sec)
    struct Impulse {
        std::string source_id;
        double magnitude;
        int64_t timestamp_sec;
    };
    std::vector<Impulse> impulses_;
};

} // namespace taal::measure

