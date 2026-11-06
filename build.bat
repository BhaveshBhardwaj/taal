@echo off
echo ========================================================
echo  TAAL: Compiling High-Performance C++20 Platform
echo ========================================================

echo Compiling unified engine: taal_engine.exe ...
g++ -std=c++20 -O3 -I include ^
  src/measure/kuramoto_dpi.cpp ^
  src/measure/pacemaker_audit.cpp ^
  src/measure/cascade_lag_map.cpp ^
  src/measure/common_forcing_model.cpp ^
  src/forecast/reflexive_forecast.cpp ^
  src/forecast/critical_slowing_down.cpp ^
  src/forecast/synthetic_delhi_twin.cpp ^
  src/desync/polyrhythm_menu.cpp ^
  src/desync/phase_pricing.cpp ^
  src/desync/slot_swap_exchange.cpp ^
  src/desync/rolling_exits_checkout.cpp ^
  src/desync/itinerary_charging_lowtech.cpp ^
  src/guardrails/resident_shield.cpp ^
  src/guardrails/regret_and_herding.cpp ^
  src/guardrails/privacy_dpdp.cpp ^
  src/govern/mohalla_council.cpp ^
  src/energy/transformer_thermal_model.cpp ^
  src/energy/battery_swap_vpp.cpp ^
  src/transit_tourism/dmrc_dual_station.cpp ^
  src/transit_tourism/cool_corridor_router.cpp ^
  src/transit_tourism/mohalla_merchant_auction.cpp ^
  src/benchmark/prior_art_matrix.cpp ^
  src/main.cpp -o taal_engine.exe

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] taal_engine.exe built successfully!
    echo Running validation suite...
    taal_engine.exe validate
) else (
    echo [ERROR] Compilation failed.
)
