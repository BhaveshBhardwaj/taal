#pragma once

#include "core/types.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace taal::govern {

enum class MemberType {
    RWA_RESIDENT,
    TRADER_ASSOCIATION,
    TRANSIT_UNION
};

struct CouncilMember {
    std::string id;
    std::string name;
    std::string organization; // e.g., "Ballimaran RWA", "Chandni Chowk Sarv Vyapar Mandal"
    MemberType type;
    int voting_weight;
};

struct PolicyProposal {
    std::string proposal_id;
    std::string title;
    std::string description;
    std::string target_corridor_zone;
    bool is_vetoed{false};
    int votes_for{0};
    int votes_against{0};
    bool is_ratified{false};
    std::string resolution_notes;
};

struct CommunityFundLedger {
    double total_revenue_collected_inr{0.0};
    double council_revenue_share_pct{15.0}; // 15% statutory share
    double fund_balance_inr{0.0};
    double total_disbursed_inr{0.0};
    std::vector<std::string> funded_projects;
};

class MohallaCouncil {
public:
    MohallaCouncil();

    // Register representative member
    void register_member(const CouncilMember& member);

    // Submit a policy for Mohalla Council review
    std::string submit_policy(const std::string& title,
                              const std::string& description,
                              const std::string& target_zone);

    // Council voting: cast vote (supports supermajority checking)
    bool cast_vote(const std::string& proposal_id, const std::string& member_id, bool approve);

    // Exercise statutory RWA/Trader hard veto
    bool exercise_council_veto(const std::string& proposal_id, const std::string& member_id, const std::string& veto_reason);

    // Deposit Phase Credit revenue share into Mohalla Community Fund
    void deposit_revenue_share(double phase_credit_turnover_inr);

    // Disburse funds for local community improvement (e.g. solar lighting, alley battery swap kiosks)
    bool disburse_fund(const std::string& project_name, double amount_inr);

    // Query status
    const PolicyProposal* get_proposal(const std::string& proposal_id) const;
    const CommunityFundLedger& get_fund_ledger() const { return fund_ledger_; }
    const std::vector<CouncilMember>& get_members() const { return members_; }

private:
    std::vector<CouncilMember> members_;
    std::vector<PolicyProposal> proposals_;
    CommunityFundLedger fund_ledger_;
    int proposal_counter_{100};
};

} // namespace taal::govern

