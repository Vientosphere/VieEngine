<div align="center">

# 🌪️ VieEngine (v0.01 Pre-Alpha)
### *High-Velocity, Minimalist C++ Game & Physics Engine*

[![C++](https://img.shields.io/badge/Language-C%2B%2B17%20%2F%20C%2B%2B20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Raylib](https://img.shields.io/badge/Renderer-Raylib%205.0-red?style=for-the-badge&logo=raylib&logoColor=white)](https://www.raylib.com/)
[![Physics](https://img.shields.io/badge/Physics-Bullet3%20(Upcoming)-orange?style=for-the-badge)](https://pybullet.org/)
[![License](https://img.shields.io/badge/License-MIT-blue.svg?style=for-the-badge)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20x64-lightgrey?style=for-the-badge&logo=windows)](https://www.microsoft.com/windows)

<p align="center">
  <b>VieEngine</b> is an independent, lightweight C++ game engine architecture engineered from the ground up by <b>Viento</b>.<br>
  Merging the essence of French <i>"Vie"</i> (Life) with the raw momentum of Spanish <i>"Viento"</i> (Wind).
</p>

</div>

---

## ⚡ Key Highlights & Architecture

VieEngine avoids the bloat of traditional heavyweight engines by maintaining a clean, cache-friendly data-oriented pipeline.

```
┌────────────────────────────────────────────────────────┐
│                      VIEENGINE CORE                    │
├───────────────────┬────────────────────────────────────┤
│ 🎮 Input Subsystem │ Event Polling & Key State Mapping  │
│ 🔄 Game Loop       │ Fixed Timestep & 60 FPS Target     │
│ 🎨 Render Pipeline │ 2D/3D Hardware-Accelerated (Raylib)│
│ 💥 Physics Engine  │ Bounding Box & Bullet3 Integration │
└───────────────────┴────────────────────────────────────┘
```

---

## 🚀 Engine Capabilities (Roadmap)

- [x] **⚡ Fixed Timestep Game Loop:** Deterministic frame progression locked at 60 FPS.
- [x] **🎮 Dynamic Input Mapper:** Real-time continuous key state polling (WASD, Action Keys).
- [x] **🎨 Screen-Space Renderer:** Immediate mode hardware accelerated 2D primitives.
- [x] **🛡️ Clamping & Boundary Physics:** Zero-latency screen edge constraint validation.
- [ ] **📦 Object-Oriented Engine Abstraction:** Modular decoupling (`Engine`, `Renderer`, `Actor`).
- [ ] **📷 3D Spatial System & Camera:** Perspective projection, orbital & free-fly cameras.
- [ ] **🌐 Bullet3 Rigid Body Integration:** Discrete collision detection, gravity, friction & restitution.

---

## 🛠️ Tech Stack & Dependencies

| Layer | Technology | Description |
| :--- | :--- | :--- |
| **Core Language** | `C++` (GCC / MinGW-w64) | Memory-efficient, direct-to-metal performance |
| **Rendering** | `Raylib v5.0` | OpenGL abstraction layer for 2D/3D graphics |
| **Physics** | `Bullet3 Physics` | Industrial-grade 3D collision & dynamics simulation |
| **Build System** | `Batch / G++ Automation` | Zero-friction one-click compile & execute workflow |

---

## 💻 Quick Build & Launch

### Prerequisites
- Windows 10/11 64-bit
- GCC / MinGW-w64 compiler with Raylib installed

### One-Click Execution
Double click the automated toolchain script:
```cmd
Derle_Ve_Baslat.bat
```

---

## 📂 Project Structure

```bash
VieEngine/
├── 📁 .git/                 # Version control
├── 📄 .gitignore            # Clean repository rules
├── 📄 README.md             # Engine documentation & blueprint
├── ⚙️ Derle_Ve_Baslat.bat    # Automated C++ build pipeline
├── 🎮 Oyunu_Baslat.bat      # Instant binary launcher
└── 💻 viento_game.cpp       # VieEngine core entrypoint
```

---

## 👤 Architect & Author

* **Lead Engineer & Vision:** **Viento**
* *"Building from memory addresses to worlds."*

---

<div align="center">
  <sub>VieEngine © 2026. Made with ❤️ by Viento.</sub>
</div>
