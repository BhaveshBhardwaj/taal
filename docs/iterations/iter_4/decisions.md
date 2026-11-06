# Architectural Decision Records (ADR) - TAAL System
## Iteration 4 Archive (Layers 4 & 5: Guardrails, Privacy & Mohalla Governance)

All ADRs 4.1 through 4.5 successfully executed and verified against test suite `tests/test_guardrails_govern.cpp`.
Test Status: 100% Passed.

### Decision 4.1: Resident Shield Multi-Metric Hard Auto-Veto Policy
- **Context**: In traditional smart city routing, when arterial streets become congested, algorithms divert traffic into quiet residential lanes, destroying residents' quality of life.
- **Decision**: Developed `ResidentShield` maintaining an immutable safety ledger across lanes and feeders. Hard automated vetoes trigger if:
  1. Any footfall is routed into residential mohallas after 22:00 (Night-Hour Curfew).
  2. Projected residential alley density exceeds safe limits (0.7-0.8 persons/$m^2$).
  3. Projected noise exceeds 55 dB.
  4. Feeder transformer loading exceeds 92%.
- **Validation**: Midday arterial routing approved (0 burden score); 23:00 Ballimaran routing immediately vetoed (`NIGHT_HOURS_RESIDENTIAL_PROTECTION`); high-density surge into Dariba Kalan havelis immediately vetoed (`RESIDENTIAL_DENSITY_EXCEEDED`).

### Decision 4.2: Regret Budget & Guaranteed Swap-Back Pass
- **Context**: Visitors will not risk following an alternative recommendation if they fear being stranded with a disappointing trip.
- **Decision**: Established a formal regret budget ($\epsilon_{\text{regret}} \le 0.35$). Every accepted nudge automatically issues a cryptographic `SwapBackPass` enabling a free, priority-lane return to the original monument within 2 hours.
- **Validation**: Token `SWAPBACK-5001` issued with priority code `VIP-GATE-101` and successfully redeemed.

### Decision 4.3: Herding Detector with HHI, Entropy Floor & Automated Kill-Switch
- **Context**: If 1,000 visitors are redirected away from Red Fort to Sunder Nursery, Sunder Nursery quickly becomes the new congested bottleneck (secondary herding).
- **Decision**: Implemented real-time concentration tracking via Herfindahl-Hirschman Index ($HHI = \sum p_i^2$) and Shannon Entropy ($H = -\sum p_i \ln p_i$). An automated kill-switch freezes recommendations for any site capturing $> 40\%$ share or if $HHI \ge 0.38$.
- **Validation**: Balanced distribution ($HHI = 0.224, H = 1.550$) cleared safely; when Sunder Nursery captured 86.5% share, kill-switch engaged instantly, freezing recommendations to that site.

### Decision 4.4: Differential Privacy Engine Aligned with India's DPDP Act 2023
- **Context**: The Digital Personal Data Protection Act 2023 mandates strict privacy protections. Urban sensing must never track individual user trajectories, MAC addresses, or phone numbers.
- **Decision**: Built a privacy engine utilizing the Laplace mechanism:
  $$\tilde{M} = M + \text{Laplace}\left(0, \frac{\Delta S}{\epsilon}\right)$$
  with $\epsilon = 0.5$. Rejects all PII and produces aggregate-only telemetry with verifiable DPDP audit hashes.
- **Validation**: Privatized headcount (1250 $\to$ 1252) and density (2.45 $\to$ 2.35) while generating compliant audit token `DPDP_OK_0xa2a25b818ed2fcf4`.

### Decision 4.5: Mohalla Council Democratic Governance & Revenue Share
- **Context**: Top-down municipal decisions often shift negative externalities onto citizens without local consent.
- **Decision**: Constituted a Mohalla Council body comprising RWAs (Ballimaran, Dariba Kalan), Trader Associations (Chandni Chowk Sarv Vyapar Mandal, Khari Baoli Guild), and E-Rickshaw Unions. The Council possesses binding veto authority and receives a mandatory 15% revenue share from all Phase Credit circulation.
- **Validation**:
  - Democratic ratification of night food walk proposal (3-0 vote).
  - Binding RWA veto exercised by Ballimaran RWA killing an unwanted alley shuttle bypass.
  - Received Rs 75,000 (15% statutory share from Rs 5,00,000 Phase Credit turnover) and disbursed Rs 30,000 for Ballimaran heritage solar streetlights.

