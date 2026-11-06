#include "govern/mohalla_council.hpp"
#include <sstream>

namespace taal::govern {

MohallaCouncil::MohallaCouncil() {
    // Seed canonical Delhi Mohalla Council stakeholders:
    // RWAs (Residents)
    register_member({
        "rwa_ballimaran",
        "Gali Qasim Jan RWA President",
        "Ballimaran Resident Welfare Association",
        MemberType::RWA_RESIDENT,
        1
    });
    register_member({
        "rwa_dariba_kalan",
        "Dariba Kalan Haveli Residents Council",
        "Dariba Kalan RWA",
        MemberType::RWA_RESIDENT,
        1
    });

    // Trader Associations (Vyapar Mandals)
    register_member({
        "traders_chandni_chowk",
        "Chandni Chowk Sarv Vyapar Mandal General Secretary",
        "Chandni Chowk Traders' Association",
        MemberType::TRADER_ASSOCIATION,
        1
    });
    register_member({
        "traders_fatehpuri",
        "Fatehpuri & Khari Baoli Spice Merchants Union",
        "Khari Baoli Traders' Guild",
        MemberType::TRADER_ASSOCIATION,
        1
    });

    // Transit Unions
    register_member({
        "erickshaw_union",
        "Netaji Subhash Marg E-Rickshaw Operators Union Head",
        "Delhi E-Vehicle Drivers Ekta Manch",
        MemberType::TRANSIT_UNION,
        1
    });
}

void MohallaCouncil::register_member(const CouncilMember& member) {
    members_.push_back(member);
}

std::string MohallaCouncil::submit_policy(const std::string& title,
                                          const std::string& description,
                                          const std::string& target_zone) {
    proposal_counter_++;
    std::string pid = "MOHALLA-PROP-" + std::to_string(proposal_counter_);
    proposals_.push_back({
        pid,
        title,
        description,
        target_zone,
        false,
        0,
        0,
        false,
        "Under Council Review"
    });
    return pid;
}

bool MohallaCouncil::cast_vote(const std::string& proposal_id, const std::string& member_id, bool approve) {
    bool member_exists = false;
    for (const auto& m : members_) {
        if (m.id == member_id) {
            member_exists = true;
            break;
        }
    }
    if (!member_exists) return false;

    for (auto& p : proposals_) {
        if (p.proposal_id == proposal_id) {
            if (p.is_vetoed) return false;

            if (approve) p.votes_for++;
            else p.votes_against++;

            // Check if supermajority achieved (>60% of 5 members = >= 3 votes)
            if (p.votes_for >= 3) {
                p.is_ratified = true;
                p.resolution_notes = "Ratified by democratic majority of Mohalla Council";
            } else if (p.votes_against >= 3) {
                p.is_ratified = false;
                p.resolution_notes = "Rejected by majority of Mohalla Council";
            }
            return true;
        }
    }
    return false;
}

bool MohallaCouncil::exercise_council_veto(const std::string& proposal_id,
                                          const std::string& member_id,
                                          const std::string& veto_reason) {
    bool authorized = false;
    std::string member_org = "";
    for (const auto& m : members_) {
        if (m.id == member_id) {
            authorized = true;
            member_org = m.organization;
            break;
        }
    }
    if (!authorized) return false;

    for (auto& p : proposals_) {
        if (p.proposal_id == proposal_id) {
            p.is_vetoed = true;
            p.is_ratified = false;
            p.resolution_notes = "HARD VETO EXERCISED by " + member_org + ": " + veto_reason;
            return true;
        }
    }
    return false;
}

void MohallaCouncil::deposit_revenue_share(double phase_credit_turnover_inr) {
    double share = phase_credit_turnover_inr * (fund_ledger_.council_revenue_share_pct / 100.0);
    fund_ledger_.total_revenue_collected_inr += phase_credit_turnover_inr;
    fund_ledger_.fund_balance_inr += share;
}

bool MohallaCouncil::disburse_fund(const std::string& project_name, double amount_inr) {
    if (fund_ledger_.fund_balance_inr < amount_inr) {
        return false;
    }
    fund_ledger_.fund_balance_inr -= amount_inr;
    fund_ledger_.total_disbursed_inr += amount_inr;
    fund_ledger_.funded_projects.push_back(project_name + " (Rs " + std::to_string(static_cast<int>(amount_inr)) + ")");
    return true;
}

const PolicyProposal* MohallaCouncil::get_proposal(const std::string& proposal_id) const {
    for (const auto& p : proposals_) {
        if (p.proposal_id == proposal_id) return &p;
    }
    return nullptr;
}

} // namespace taal::govern

