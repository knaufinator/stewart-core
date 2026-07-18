# stewart-core

Shared math core for the rotary-Stewart motion platform — the **single source of truth**,
consumed as a git submodule by both:

- **6DOF-Rotary-Stewart-Motion-Simulator** — desktop app (SIL preview + `.m6p` baking), plain CMake.
- **Mini-6DOF** — ESP32 firmware (ESP-IDF).

## Contents
| File | Purpose |
|---|---|
| `InverseKinematics.{cpp,h}` | Servo-angle IK from platform pose (`atan2f`, quadrant-safe) + `computeHomeHeight()`. |
| `AxisScaling.{cpp,h}` | Geometry-derived raw->position scaling (`mapRawToPosition`, `computeAxisScalesFromGeometry`). |
| `MotionCueing.{cpp,h}` | Washout + tilt-coordination MCA, biquads, presets, NVS persistence. |

## Build
- **ESP-IDF:** drop in as a component (e.g. `Controller/components/stewart-core`); self-registers. `MotionCueing.cpp` uses `debug_uart.h` on ESP (firmware provides it).
- **Plain CMake:** `add_subdirectory(stewart-core)` then link `stewart_core`. Non-ESP stubs `DEBUG_PRINTF`.

## History
Extracted 2026-07-18 from `6DOF-.../Controller` to end a two-fork divergence (the firmware fork used
`atanf(n/m)` and lacked `computeHomeHeight`). Canonical = `atan2f` + `computeHomeHeight`.
