# TAAL Project Engineering Rules and Protocols

## 1. Core Principles & Philosophy
- **Unison to Polyrhythm**: The central thesis of TAAL (Turning Delhi's Unison into Polyrhythm) is that urban infrastructure stress in Delhi is driven by the *phase coherence of decisions* (unison), not raw headcount. Stress is managed by breaking synchrony across time, space, and modal channels.
- **Hypothesis Proof**: Every phase calculation must quantify how dispersion in phase space mitigates physical strain on metro platforms, feeder transformers, e-rickshaw stands, and residential mohallas.
- **Zero Cost Shifting to Residents**: Guardrails must strictly prevent tourist/commuter traffic and transformer loads from displacing onto residential mohallas (`Resident Shield` hard veto).

## 2. Iteration Governance
- **Pre-Iteration Protocol**: Before starting any iteration, an authoritative `srs.md` (Software Requirements Specification) MUST be created/updated specifying:
  - Exact functional and non-functional requirements for the iteration.
  - Mathematical models, formulas, and state representations.
  - Data structures, interfaces, and component boundaries.
  - Verification & validation test gates (expected outputs, thresholds, invariants).
- **Post-Iteration Protocol**: After completing each iteration, the engineering outcomes MUST be committed and archived:
  - `decisions.md`: Architectural Decision Records (ADRs) explaining technical trade-offs, rationale, alternatives considered, and validation proofs.
  - Iteration snapshots of `srs.md` and verification test logs stored in an iteration history repository/directory (`docs/iterations/iter_X/`).
  - Update top-level `srs.md` and `decisions.md` to reflect the current state of the system.

## 3. Architecture & Language Standards
- **High-Performance Core**:
  - The computational core, physics & phase dynamics (Kuramoto Order Parameter $R(t)$), critical slowing down early warning, synthetic digital twin agent-based model (ABM), cascade lag propagation, reflexive fixed-point solver, and guardrails must be implemented in **modern C++ (C++20/C++23)** for optimal latency, memory efficiency, and deterministic simulation scaling to $100,000+$ agents.
  - Code must be strictly modular, split into clean headers (`.hpp`) and source implementations (`.cpp`), with zero circular dependencies and clear domain separation across the 5 canonical layers:
    1. `Layer 1: Measure Synchrony` (`include/measure/`, `src/measure/`)
    2. `Layer 2: Forecast` (`include/forecast/`, `src/forecast/`)
    3. `Layer 3: Break the Synchrony` (`include/desync/`, `src/desync/`)
    4. `Layer 4: Guardrails` (`include/guardrails/`, `src/guardrails/`)
    5. `Layer 5: Govern` (`include/govern/`, `src/govern/`)
- **Integration & Presentation**:
  - Provide a unified CLI interface and high-performance JSON / REST / Visualizer bridge for interactive simulation, demonstration, and validation dashboard.
  - Support full end-to-end reproducible scenarios covering the Chandni Chowk - Red Fort corridor pilot.

## 4. Feature Coverage Rules (Mandatory 20 Features)
1. **Decision-Phase Index (DPI)**: $R(t) = \frac{1}{N}\left|\sum_{j=1}^N e^{i\theta_j}\right|$ on 24h clock $[0, 2\pi)$. Sync-vs-Volume regression test.
2. **Pacemaker Audit**: Rank Delhi synchronizing clocks (sunset, Light & Sound show end, Akshardham water show, noon checkout, aarti) by phase-locking strength.
3. **Cascade Lag Map**: Directed acyclic graph of delay timings ($show \xrightarrow{+8m} gate \xrightarrow{+20m} rickshaw \xrightarrow{+50m} charger \xrightarrow{+90m} feeder$).
4. **Common-Forcing Model**: Ambient heat ($45^\circ\text{C}+$), AQI/GRAP stages, fog, festivals modeled as external forcing terms $F(t)$ distinct from organic sync.
5. **Reflexive Forecast**: Fixed-point solver $\hat{x} = \Phi(\hat{x})$ preventing crowd rebound rushes.
6. **Coherence Early-Warning**: Critical slowing down detection (variance $\sigma^2 \uparrow$ and lag-1 autocorrelation $\rho_1 \uparrow$).
7. **Synthetic Delhi Twin**: Chandni Chowk corridor agent simulation with Metro stations, e-rickshaws, battery swapping, 11kV distribution transformers, residents.
8. **Polyrhythm Menu ("Twin Moments")**: Equal-Delight Score recommendation across 4-6 heritage alternatives (Humayun's Tomb, Sunder Nursery, Purana Qila, Lodhi Garden).
9. **Lottery Suggestions**: Quota-based anti-herding recommendation lottery with hard diversity floor.
10. **Phase Pricing**: Pricing/rebate determined by marginal contribution to coherence $\Delta R(t)$; Phase Credits ledger for Metro and local Chandni Chowk vendors.
11. **Slot-Swap Exchange**: Peer-to-peer time-slot order book for tour operators and schools.
12. **Exit-as-Experience**: 4 colored rolling waves with zoned buffer experiences (sherbet, heritage walk, restrooms) and staggered gate closings.
13. **Free-Hands Checkout**: Rolling checkout bands (11:00, 11:45, 12:30, etc.) + Metro station luggage locker reservations.
14. **Itinerary-Embedded Charging**: Cascade-aware e-rickshaw swap scheduling & community battery buffer at overloaded transformers.
15. **Low-Tech Channels**: Color stubs, PA broadcasts, Hindi/English IVR/WhatsApp engine, rickshaw stand captain protocols.
16. **Resident Shield**: Multi-metric ledger (noise, footfall, parking, night hours, outage risk); automatic veto when residential threshold delta $> 0$.
17. **Regret Budget + Swap-Back Guarantee**: Quantified regret bounds with guaranteed free swap-back if alternate underdelivers.
18. **Herding Detector**: Herfindahl-Hirschman Index (HHI) & Shannon entropy floor with automatic kill-switch.
19. **Privacy by Design**: DPDP Act 2023 compliance, aggregate telemetry, differential privacy ($\epsilon, \delta$-Laplace noise).
20. **Mohalla Council**: Democratic RWA and trader veto power, Phase Credit revenue distribution, audit trails.

## 5. Verification & Validation Metrics
Every release must compute and report:
- $\Delta \text{DPI}$ reduction at pacemaker peaks.
- Peak queue time, platform density, and feeder transformer loading $\%$.
- Effective Capacity gain without new physical infrastructure.
- Resident Ledger Delta $\le 0$.
- Trip Satisfaction Delta $\ge 0$.
- Suggestion Entropy $\ge \text{Entropy Floor}$ ($HHI \le HHI_{max}$).

