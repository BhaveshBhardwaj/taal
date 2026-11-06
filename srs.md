# Software Requirements Specification (SRS) - TAAL System
**Document Version**: 6.0.0 (Iteration 6: Advanced Delhi Innovation — Transport, Energy & Tourism Deepening)  
**Project**: TAAL: Turning Delhi's Unison into Polyrhythm  
**National Track**: Transport, Energy & Tourism · Local Community Level Innovation (REG-010-OP)  
**Pilot Corridor**: Chandni Chowk – Red Fort Heritage Corridor, Old Delhi  

---

## 1. Context & Problem Statement
Destinations are stressed less by how many tourists arrive than by how many decide the same thing at the same minute: sunrise points, show endings, and checkout times synchronize strangers into surges that cascade from roads to chargers to the grid. 

TAAL hypothesizes that the **phase coherence of visitor decisions predicts infrastructure stress better than volume does**. 
The objective of Iteration 6 is to deepen the system specifically for Delhi’s coupled **Transport, Energy, and Tourism** ecosystem by implementing 6 advanced, community-rooted innovations:

1. **Feature 21: Transformer Thermal Physics & Extreme Ambient Derating (Energy)**: IEEE C57.91 / IEC 60076-7 thermal differential model capturing transformer oil and hot-spot winding temperature under Delhi’s $45^\circ\text{C}+$ ambient heat and post-show charger spikes.
2. **Feature 22: DMRC Dual-Station Transit Headway Splitter (Transport)**: Coordinates pedestrian exit waves between Chandni Chowk (Yellow Line) and Lal Qila (Violet Line) stations based on real-time train headways, eliminating single-station crushes.
3. **Feature 23: E-Rickshaw Battery Swap Virtual Power Plant (Energy + Transport)**: Orchestrates distributed battery swapping kiosks (Mori Gate, Kashmere Gate) as a 1.2 MWh Virtual Power Plant (VPP), absorbing solar overgeneration by day and shedding grid load during the evening exit cascade.
4. **Feature 24: Cool-Corridor Microclimate Routing (Tourism + Climate Resilience)**: Maps Old Delhi’s shaded covered *katras* (Katra Neel, Kinari Bazaar, Haveli Dharampura arcades) offering a $4^\circ\text{C} - 6^\circ\text{C}$ temperature reduction over open asphalt, protecting visitors from Delhi heatwaves while dispersing arrival phases.
5. **Feature 25: Mohalla Merchant Dynamic Credit Auction (Local Community & Tourism)**: Enables Old Delhi micro-merchants (Khari Baoli spice traders, Paranthe Wali Gali, Dariba Kalan jewellers) to sponsor off-peak Phase Credit boosters, redirecting footfall during slow afternoon hours.
6. **Feature 26: Prior-Art Novelty & Academic Comparison Matrix**: Formal algorithmic benchmarks proving TAAL’s world-first phase coherence formulation against Google Maps, Disney Virtual Queue, Singapore ERP 2.0, and standard EV smart charging.

---

## 2. Mathematical & Engineering Formulations

### 2.1 Feature 21: Transformer Thermal Physics Model
- According to IEEE Std C57.91, hot-spot winding temperature $\Theta_{hs}(t)$ is:
  $$\Theta_{hs}(t) = \Theta_a(t) + \Delta\Theta_{to}(t) + \Delta\Theta_{hs}(t)$$
  where $\Theta_a$ is Delhi ambient temperature (reaching $45^\circ\text{C}$ to $47^\circ\text{C}$ in summer), $\Delta\Theta_{to}$ is top-oil rise over ambient, and $\Delta\Theta_{hs}$ is hot-spot rise over top-oil.
- Effective thermal capacity derating:
  $$kVA_{\text{derated}} = kVA_{\text{rated}} \cdot \sqrt{\frac{\Theta_{\text{max\_allowed}} - \Theta_a(t)}{\Delta\Theta_{\text{rated\_rise}}}}$$
- In Delhi at $\Theta_a = 45^\circ\text{C}$, nominal 1,000 kVA rating collapses to $\sim 780$ kVA!
- The model continuously outputs relative aging rate $V = \exp\left(\frac{15000}{383} - \frac{15000}{\Theta_{hs} + 273}\right)$ and triggers early-warning battery injection before insulation degradation occurs.

### 2.2 Feature 22: DMRC Dual-Station Headway Synchronization
- Let $W_{\text{Yellow}}$ and $W_{\text{Violet}}$ be queue wait times at Chandni Chowk and Lal Qila stations.
- Split ratio $S_{\text{Violet}} \in [0, 1]$ is dynamically tuned:
  $$S_{\text{Violet}} = \frac{1}{1 + \exp\left(-\gamma \cdot (\text{Density}_{\text{Yellow}} - \text{Density}_{\text{Violet}})\right)}$$
- If Chandni Chowk Yellow Line platform density $> 2.0$ persons/$m^2$, visitors are directed with +40 Phase Credits to Lal Qila Violet Line (400m stroll), reducing platform density by $\sim 45\%$.

### 2.3 Feature 23: E-Rickshaw Battery Swap Virtual Power Plant (VPP)
- Aggregates $M$ battery swap kiosks (each with 30 battery slots):
  - State: Total Stored Energy (kWh), Charging Draw (kW), Discharge/Standby Capacity.
- Peak Shaving Protocol: When 11kV feeder load $> 85\%$ of derated capacity, charging is throttled to 0 kW and fully charged batteries supply DC microgrids or driver swaps without drawing from the grid.

### 2.4 Feature 24: Cool-Corridor Microclimate Routing
- Graph $G = (V, E)$ where each edge $e$ has length $L_e$, shade factor $S_e \in [0, 1]$, and perceived temperature:
  $$T_{\text{perceived}}(e) = T_{\text{ambient}} - (S_e \cdot 5.5^\circ\text{C}) - (W_e \cdot 1.5^\circ\text{C})$$
  where $W_e$ is passive evaporative cooling / misting index in covered katras.
- Multi-objective Dijkstra routing finds paths minimizing heat stress and phase alignment.

### 2.5 Feature 25: Mohalla Merchant Booster Auction
- Shopkeepers post incentive bids $(B_j, \Delta t_j)$ offering credit multipliers:
  $$\text{Effective\_Credits} = \text{Base\_Credits} + B_j$$
- Guarantees footfall is funneled into verified local family-owned shops rather than multinational chains.

### 2.6 Feature 26: Prior-Art Novelty Matrix
- Evaluates and benchmarks 6 distinct dimensions against:
  1. Google Maps / Navigation (headcount routing $\implies$ herding failure)
  2. Singapore ERP 2.0 (linear tolling $\implies$ economic inequality, no phase consideration)
  3. Disney Virtual Queue / Timed Ticketing (siloed to single venue $\implies$ no cross-system grid coupling)
  4. Standard EV Smart Charging (price-based $\implies$ blind to visitor cultural pacemakers)
  5. TAAL (phase coherence measurement, cross-system DAG cascade, anti-herding lottery, Resident Shield).

---

## 3. Directory Layout & Modular Additions
- `include/energy/` & `src/energy/`:
  - `transformer_thermal_model.hpp` / `.cpp` (Feature 21)
  - `battery_swap_vpp.hpp` / `.cpp` (Feature 23)
- `include/transit_tourism/` & `src/transit_tourism/`:
  - `dmrc_dual_station.hpp` / `.cpp` (Feature 22)
  - `cool_corridor_router.hpp` / `.cpp` (Feature 24)
  - `mohalla_merchant_auction.hpp` / `.cpp` (Feature 25)
- `include/benchmark/` & `src/benchmark/`:
  - `prior_art_matrix.hpp` / `.cpp` (Feature 26)
- Comprehensive test suite: `tests/test_delhi_innovation.cpp`
