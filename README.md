# PASS SCIENCE WORKS | R&D Common Technical Assignment 2026-2027
# Unified Engineering Workbench

[![C++17 Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake Build](https://img.shields.io/badge/CMake-3.15%2B-brightgreen.svg)](https://cmake.org/)
[![React UI](https://img.shields.io/badge/React-18-61dafb.svg)](https://react.dev/)
[![Three.js](https://img.shields.io/badge/Three.js-3D%20WebGL-black.svg)](https://threejs.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%2011%20x64%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)]()
[![Tests](https://img.shields.io/badge/Tests-50%2F50%20Passing-success.svg)]()

A high-performance, modular engineering workbench integrating CAD/ANSYS 3D visualization, 1D steady-state thermal Finite Element Analysis (FEA), multicore CPU parallel execution, embedded hardware simulation, and neural backpropagation verification.

---

## 1. Quick Start & One-Click Execution

### On Windows 11 x64 (Standalone, Zero-Install)
1. Extract the `CandidateID_Workbench.zip` archive.
2. **Double-click `Launch.exe`** (or `Launch.bat`).
3. The application will immediately open in your default browser at `http://localhost:8080`.
   - *No terminal commands, PowerShell, administrator privileges, package installations, or first-run builds required.*

### On macOS / Linux
Open a terminal in the extracted directory and run:
```bash
./launch.sh
```

### In Antigravity / VS Code (One Action)
1. Open the repository root in Antigravity IDE or VS Code.
2. Select the **Run & Debug** tab (`Ctrl+Shift+D` or `Cmd+Shift+D`).
3. Select **"One-Click Launch Workbench Application"** and press `F5`.

---

## 2. System Architecture & Module Boundaries

The workbench is strictly partitioned into a pure computational C++17 core library (`workbench_core`) and a modern interactive web presentation layer:

```
+---------------------------------------------------------------------------------------+
|                                    PRESENTATION LAYER                                 |
|               React 18 + TypeScript + Tailwind CSS + Three.js + Recharts              |
|        [Geometry View]  [Thermal FEA]  [Multicore Perf]  [Hardware Mocks]  [Backprop] |
+---------------------------------------------------------------------------------------+
                                           |
                                [HTTP/JSON REST Transport]
                                           |
+---------------------------------------------------------------------------------------+
|                                  C++17 CORE BACKEND                                   |
|   +-------------------+  +--------------------+  +--------------------------------+   |
|   |   File Parsers    |  |  Thermal FEA Core  |  |     Concurrency Engine         |   |
|   |  ASCII STL Parser |  |  Weak Form Solver  |  |  Bounded ThreadPool (1, 2, 4)  |   |
|   |  ANSYS CSV Parser |  |  Midpoint Error O(h²)| |  Deterministic Ordering       |   |
|   +-------------------+  +--------------------+  +--------------------------------+   |
|   +-------------------------------------------+  +--------------------------------+   |
|   |            Embedded Simulation            |  |         Backpropagation        |   |
|   |    MockMCU, MockFPGA, MockJTAG Adapter    |  |   Exact Chain-Rule Derivatives |   |
|   |    Fault Injection (1500ms Timeout Reset) |  |   Finite Difference (ε = 1e-6) |   |
|   +-------------------------------------------+  +--------------------------------+   |
+---------------------------------------------------------------------------------------+
```

### Module Responsibilities:
- `src/backend/include/workbench/stl_parser.hpp`: Streaming ASCII STL parser with bounding-box calculation and non-finite float validation.
- `src/backend/include/workbench/ansys_csv_parser.hpp`: ANSYS nodal temperature CSV parser (`node_id,x_m,y_m,z_m,temperature_C`).
- `src/backend/include/workbench/thermal_solver.hpp`: 1D steady-state thermal FEA solver with exact analytical comparison and $O(h^2)$ midpoint error convergence.
- `src/backend/include/workbench/thread_pool.hpp` & `batch_solver.hpp`: Bounded worker pool with 5 repetitions, median time, speedup $T_1/T_p$, and serial/parallel agreement verification ($\le 10^{-8\ \circ}\text{C}$).
- `src/backend/include/workbench/hardware_mock.hpp`: Embedded device mocks for MCU, FPGA, and JTAG probe with recoverable timeout simulation.
- `src/backend/include/workbench/backpropagation.hpp`: Exact analytical derivatives by chain rule, central finite differences check ($\epsilon = 10^{-6}$), and 100-step training loop.
- `src/backend/include/workbench/http_server.hpp`: Embedded zero-dependency HTTP/JSON server that serves API routes and static frontend assets.

---

## 3. Supported File Formats & Specifications

### 1. ASCII STL Geometry (`.stl`)
- **Format**: Standard ASCII STL specification (`solid ... facet normal ... vertex ... endloop endfacet endsolid`).
- **Validation**: Checks for 3 vertices per facet, valid counter-clockwise normal vectors, and rejects `NaN`/`Inf` coordinates.
- **Tested Limits**: Validated up to 25 MB (~150,000 triangular facets).

### 2. ANSYS Nodal Temperature Results (`.csv`) [LABELLED SYNTHETIC]
- **Required Columns**: `node_id,x_m,y_m,z_m,temperature_C` (case-insensitive headers).
- **Units**: Coordinates in meters [m], temperatures in Celsius [°C].
- **Validation**: Checks for numeric row consistency, strictly rejects missing columns, and rejects non-finite values.
- **Tested Limits**: Validated up to 50,000 nodes.

---

## 4. Build & Test Instructions (CMake Source Build)

### Prerequisites:
- CMake 3.15 or newer
- C++17 compliant compiler:
  - Windows: Visual Studio 2019 / 2022 (MSVC)
  - macOS: Apple Clang 12.0+ / Xcode Command Line Tools
  - Linux: GCC 9.0+ or Clang 10.0+
- Optional for frontend development: Node.js v18+ and npm

### 1. Build C++ Backend & Tests:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### 2. Execute Test Suite:
```bash
# Run unit and numerical tests
./build/bin/workbench_tests
```
*Output: `TEST SUMMARY: 50 PASSED, 0 FAILED`.*

### 3. CLI Direct Verification Flags:
```bash
# Run thermal FEA solver directly from CLI:
./build/bin/workbench_server --fea 20

# Run multicore batch benchmark:
./build/bin/workbench_server --benchmark

# Run backpropagation verification:
./build/bin/workbench_server --backprop
```

### 4. Build Frontend (if modifying UI):
```bash
cd src/frontend
npm install
npm run build
```

---

## 5. Engineering Solutions & Verification Outcomes

### Task C: Thermal FEA & Nodal Exactness Insight
- **Weak Form Stiffness**: $k_e = \frac{kA}{h}\begin{bmatrix}1 & -1 \\ -1 & 1\end{bmatrix}$
- **Load Vector**: $f_e = \frac{qAh}{2}\begin{bmatrix}1 \\ 1\end{bmatrix}$
- **Boundary Conditions**: $T(0) = 100^\circ\text{C}, T(L) = 20^\circ\text{C}$ directly substituted into interior equations.
- **Nodal Exactness Trap**: In 1D linear FEA with constant source $q$, nodal values coincide with the exact analytical solution to machine precision ($|T_i - T_{exact}| < 10^{-12\ \circ}\text{C}$). Evaluating error only at nodes falsely suggests zero error!
- **Midpoint Convergence**: By evaluating error at element midpoints $T_{mid} = 0.5(T_i + T_{i+1})$, the true quadratic curvature error is revealed:
  - $N=10 \implies e_{mid} = 1.250000^\circ\text{C}$
  - $N=20 \implies e_{mid} = 0.312500^\circ\text{C}$
  - $N=40 \implies e_{mid} = 0.078125^\circ\text{C}$
  - $N=80 \implies e_{mid} = 0.019531^\circ\text{C}$
  - Asymptotic convergence rate: **$O(h^2)$ strictly verified**.

### Task D: Multicore Execution & Agreement
- Bounded thread pool eliminates unbounded thread creation overhead.
- Serial and parallel versions achieve numerical agreement with **$\max |\Delta T| = 0.0000^\circ\text{C} \le 10^{-8\ \circ}\text{C}$**.
- 5 measured repetitions per thread worker count with median time and efficiency reporting.

### Task E: Embedded System Integration
- Detailed pin-voltage domain mapping covering 3.3V (MCU), multi-rail 1.0V/1.8V/3.3V (FPGA), and dynamic VTref sensing (JTAG probe).
- **Critical Distinction**: Explains why a JTAG adapter is a debug bus master (TAP controller), not a generic compute board.
- Working mock adapters with recoverable 1500ms timeout simulation and protocol recovery.

### Task F: Backpropagation Verification
- Initial error: $e = -1.5379$, initial loss: $J = 1.1825$.
- Exact chain rule derivatives: $\partial J/\partial v = -0.7107, \partial J/\partial c = -1.5379, \partial J/\partial w = -1.2095, \partial J/\partial b = -1.2095$.
- Central finite difference check ($\epsilon = 10^{-6}$): All parameter errors $\sim 10^{-11} \le 10^{-5}$ (**100% Passed**).
- 100-step training loss drops monotonically to $0.0003$.

---

## 6. Directory Layout & Artifacts

```
.
├── CMakeLists.txt                 # Root CMake build configuration
├── Launch.bat                     # Windows one-click batch launcher
├── Launch.exe                     # Windows one-click PE64 executable
├── launch.sh                      # macOS / Linux one-click shell launcher
├── README.md                      # Primary documentation & instructions
├── .vscode/                       # VS Code & Antigravity run/debug configurations
│   ├── launch.json
│   ├── tasks.json
│   └── settings.json
├── data/                          # Provenance and test geometries
│   ├── sample_bracket.stl         # Clean ASCII STL sample
│   ├── ansys_thermal_results.csv  # Synthetic ANSYS nodal temperature CSV
│   ├── malformed_sample.stl       # Negative test STL (NaN coordinate)
│   ├── malformed_ansys.csv        # Negative test CSV (missing column)
│   └── provenance.md              # Provenance, units, and schema notes
├── results/                       # Raw output CSVs from verified C++ runs
│   ├── convergence_study.csv      # FEA midpoint error and nodal outputs
│   ├── benchmark_timings.csv      # 5 repetitions, speedup, and efficiency
│   └── backprop_training_log.csv  # 100-step parameter trajectory and loss
├── docs/                          # Detailed technical documentation
│   ├── report.md                  # Comprehensive 6-page technical report
│   ├── ai_use_log.md              # AI tool prompts, edits, and challenge tests
│   ├── ownership_statement.md     # Statement of authorship and reused work
│   └── handwritten_backprop/      # 2-page handwritten derivation appendix
│       └── handwritten_derivation.md
├── src/
│   ├── backend/                   # C++17 modular core and server
│   │   ├── include/workbench/
│   │   │   ├── common.hpp
│   │   │   ├── stl_parser.hpp
│   │   │   ├── ansys_csv_parser.hpp
│   │   │   ├── thermal_solver.hpp
│   │   │   ├── thread_pool.hpp
│   │   │   ├── batch_solver.hpp
│   │   │   ├── hardware_mock.hpp
│   │   │   ├── backpropagation.hpp
│   │   │   └── http_server.hpp
│   │   └── src/
│   │       ├── main.cpp
│   │       ├── stl_parser.cpp
│   │       ├── ansys_csv_parser.cpp
│   │       ├── thermal_solver.cpp
│   │       ├── thread_pool.cpp
│   │       ├── batch_solver.cpp
│   │       ├── hardware_mock.cpp
│   │       └── backpropagation.cpp
│   └── frontend/                  # React 18 + TS + Tailwind + Three.js
└── tests/
    └── main_tests.cpp             # 50 unit and numerical tests
```

---

## 7. Known Limitations & Disclosures
1. **Simulation Scope**: In accordance with the assignment brief, hardware board peripherals (MCU, FPGA, JTAG) are simulated using modular C++ mock adapters; physical PCB fabrication, silicon routing, and real FPGA bitstream generation are outside the assignment prototype scope.
2. **WebGL Requirement**: The 3D CAD and point cloud viewer requires a WebGL-compatible browser.
3. **Single-Core Machines**: On virtualized machines with only 1 logical CPU core, the multicore benchmark runs correctly and verifies serial/parallel numerical agreement, but speedup cannot be assessed ($T_1/T_p \approx 1.0$).
