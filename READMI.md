Simple Helicopter Flight Simulator — Demo Project

> A personal open-source project built to illustrate the architecture and technical concepts behind professional helicopter flight simulation — since production systems are proprietary and cannot be shared publicly.

---

## What This Is

This is a simplified but structurally realistic desktop simulation built with **C++14** and **Qt 6**, running on Windows.

It is structured as three independent components that communicate with each other through shared memory — mirroring the decoupled, process-isolated architecture common in real simulation systems.

---

## Architecture
┌─────────────────┐        ┌──────────────────┐        ┌──────────────────┐
│   FlightModel   │◄──────►│   ControlApp     │        │   DisplayApp     │
│   (DLL)         │        │   (Pilot Input)  │        │   (EFIS Display) │
│                 │        │                  │        │                  │
│  6-DoF dynamics │        │  Cyclic joystick │        │  Attitude        │
│  ~60 Hz loop    │        │  Collective      │        │  indicator       │
│  QThread        │        │  Pedals          │        │  Pitch ladder    │
└────────┬────────┘        └────────┬─────────┘        └────────┬─────────┘
│                          │                            │
└──────────────────────────┴────────────────────────────┘
QSharedMemory (IPC)

- **FlightModel** — A C++ DLL that runs the flight dynamics in a dedicated `QThread` at ~60 Hz. It reads pilot inputs and writes aircraft state (pitch, roll, yaw, altitude, vertical speed) to a shared memory segment.
- **ControlApp** — A Qt Widgets application that simulates the cockpit controls: a cyclic joystick (pitch/roll), a collective slider (altitude), and rudder pedals (yaw). Inputs are written to shared memory.
- **DisplayApp** — A Qt Widgets application that reads the aircraft state from shared memory and renders an EFIS-style primary flight display using `QPainter`: pitch ladder, bank angle indicator, heading, and vertical speed.

---

## Flight Model (Simplified 6-DoF)

The dynamics are intentionally simplified but physically plausible:

| Axis | Behaviour |
|------|-----------|
| Pitch / Roll | Cyclic input drives angular rate; spring return to neutral; rate damping |
| Vertical | Collective drives vertical speed |
| Yaw | Pedal input drives yaw rate directly |

---

## Key Technologies

| Technology | Purpose |
|------------|---------|
| C++14 | Core language |
| Qt 6 (Widgets) | UI, painting, threading |
| `QSharedMemory` | Inter-process communication between components |
| `QThread` | Dedicated flight model update loop at ~60 Hz |
| `QPainter` | Custom EFIS instrument rendering |
| MSVC / Visual Studio | Build toolchain (Windows x64) |

---

## How to Build

**Requirements:** Visual Studio 2022, Qt 6.x (MSVC x64), Qt VS Tools extension.

1. Clone the repo
2. Open `HelicopterSim.sln` in Visual Studio
3. Select **Debug | x64**
4. Build the solution (`Ctrl+Shift+B`)
5. Run `ControlApp` and `DisplayApp` — both need to be running simultaneously

---

## Project Structure
HelicopterSim/
├── FlightModel/        # DLL — flight dynamics, QThread loop
├── ControlApp/         # Qt app — pilot input controls
├── DisplayApp/         # Qt app — EFIS attitude display
└── HelicopterSim.sln   # Visual Studio solution

---

## Context

In professional simulation environments, flight models, display systems, and input handling are separate, independently deployable components — often running on different machines and communicating over a network or shared memory bus. This project replicates that separation at a small scale to demonstrate the same architectural principles.

---

## License

MIT — free to use, study, and modify.