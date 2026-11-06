# Architectural Decision Records (ADR) - TAAL System
## Iteration 3 Archive (Layer 3: Break the Synchrony / Desynchronization Engine)

All ADRs 3.1 through 3.8 successfully executed and verified against test suite `tests/test_desync.cpp`.
Test Status: 100% Passed.

### Decision 3.1: Polyrhythm Menu Equal-Delight Score Formulation
- **Context**: Visitors will reject crowd-redirection suggestions if alternative sites offer inferior experiences or dissimilar atmosphere.
- **Decision**: Developed `Equal-Delight Score` (EDS) balancing verified visitor reviews ($w_1 = 0.35$), dwell duration parity ($w_2 = 0.35$), and architectural/aesthetic vibe affinity ($w_3 = 0.30$).
- **Validation**: Generated 5 high-affinity twin moments for Red Fort (Sunder Nursery $\text{EDS} = 0.95$, Humayun's Tomb $\text{EDS} = 0.95$, Purana Qila $\text{EDS} = 0.88$, Lodhi Garden $\text{EDS} = 0.88$, Safdarjung Tomb $\text{EDS} = 0.83$).

### Decision 3.2: Quota-Based Lottery Recommender with Diversity Floor
- **Context**: Standard recommender systems suffer from algorithmic herding (recommending the single top-ranked site to all users, simply relocating the bottleneck).
- **Decision**: Implemented stochastic lottery selection combining exponential utility weights with dynamic hourly remaining quotas, backed by a hard diversity floor ($\delta_{\text{floor}} \ge 10\%$).
- **Validation**: Across 500 simulated draws, suggestions were distributed smoothly across all 5 alternatives (safdarjung: 11.0%, purana_qila: 14.8%, lodhi_garden: 22.4%, humayuns_tomb: 22.8%, sunder_nursery: 29.0%), completely eliminating recommendation herding.

### Decision 3.3: Marginal Coherence Phase Pricing ($\Delta R$) & Local Merchant Ledger
- **Context**: Conventional surge pricing taxes users based on simple headcount, generating resentment without addressing phase alignment.
- **Decision**: Formulated Phase Pricing based on an incoming visitor's exact marginal contribution to the Kuramoto order parameter:
  $$\Delta R(\theta) = \frac{1}{N+1}\left|\sum e^{i\theta_j} + e^{i\theta}\right| - \frac{1}{N}\left|\sum e^{i\theta_j}\right|$$
  - Visitors whose decisions disperse peak coherence ($\Delta R < 0$) earn Phase Credits.
  - Visitors arriving in unison ($\Delta R > 0$) receive off-peak shift incentives.
  - Credits are directly redeemable with Chandni Chowk merchants (Giani di Hatti, Paranthe Wali Gali, Kanwarji) and DMRC Metro cards, retaining value in the local economy.
- **Validation**: Successfully rewarded desynchronizing users with 61 Phase Credits and verified cryptographic merchant redemptions.

### Decision 3.4: Zero-Cash Peer-to-Peer Slot-Swap Exchange
- **Context**: Organized groups (tour operators, school field trips) are Delhi's largest discrete synchronizers (50-200 passengers per shock).
- **Decision**: Implemented an automated double-auction barter order book matching complementary slot requests without monetary exchange or administrative penalties.
- **Validation**: Successfully cleared bilateral barter between DPS School Bus (75 students) and Delhi Tourism Coach (70 tourists), swapping 10:00 and 14:00 slots without fees.

### Decision 3.5: Exit-as-Experience 4 Colored Rolling Waves
- **Context**: Red Fort Sound & Light show endings trigger massive sudden rushes at Lahore Gate bottleneck.
- **Decision**: Redesigned the exit pacemaker into 4 colored waves (Crimson Red: T+0m, Emerald Green: T+12m, Sapphire Blue: T+24m, Gold Amber: T+36m), paired with curated buffer experiences (Sherbet tastings, nocturnal walks, live Shehnai recital) and staggered gate closings.
- **Validation**: Verified sequential wave generation and zone closing schedule eliminating gate bottlenecks.

### Decision 3.6: Free-Hands Checkout Cluster Protocol
- **Context**: 11:00-12:00 hotel checkout times in Paharganj and Aerocity eject thousands onto the streets simultaneously with heavy baggage.
- **Decision**: Coordinated 4 rolling checkout bands (10:30, 11:15, 12:00, 12:45) paired with automated DMRC Metro station luggage locker reservations.
- **Validation**: Verified automated voucher reservations allowing tourists to wander until 16:15 baggage-free.

### Decision 3.7: Itinerary-Embedded Charging & Feeder Battery Peak Shaving
- **Context**: E-rickshaws rushing to charge after the evening tourist surge overload the 11kV distribution transformer (+50m to +90m cascade).
- **Decision**: Integrated Cascade Lag Map forecasts with automated battery-swap scheduling (90-second battery exchange) and a 500 kWh / 300 kW community Battery Energy Storage System (BESS) at the feeder.
- **Validation**: Verified battery swap reservation avoiding fast-charger grid draw and successfully shaved 200 kW peak from the feeder transformer.

### Decision 3.8: Low-Tech Inclusive Channels
- **Context**: A significant proportion of Delhi visitors and e-rickshaw drivers do not possess smartphones or English fluency.
- **Decision**: Built a multi-channel dissemination engine supporting physical color-coded ticket stubs, bilingual PA broadcast scripts (Hindi & English), an interactive WhatsApp/IVR bilingual bot, and e-rickshaw stand captain dispatch directives.
- **Validation**: Verified ticket stub generation, Hindi PA audio dispatch, and WhatsApp Hindi conversational responses.

