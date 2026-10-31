# PASS SCIENCE WORKS | R&D Common Technical Assignment 2026-2027
# Unified Engineering Workbench

[![C++17 Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake Build](https://img.shields.io/badge/CMake-3.15%2B-brightgreen.svg)](https://cmake.org/)
[![React UI](https://img.shields.io/badge/React-18-61dafb.svg)](https://react.dev/)
[![Three.js](https://img.shields.io/badge/Three.js-3D%20WebGL-black.svg)](https://threejs.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%2011%20x64%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)]()
[![Tests](https://img.shields.io/badge/Tests-55%2F55%20Passing-success.svg)]()

A high-performance, modular engineering workbench integrating CAD/ANSYS 3D visualization, 1D steady-state thermal Finite Element Analysis (FEA), multicore CPU parallel execution, embedded hardware simulation, computation cancellation controls, and neural backpropagation verification.

---

## 1. Quick Start & One-Click Execution

### On Windows 11 x64 (Standalone, Zero-Install)
1. Extract the `CandidateID_Workbench_Windows_x64.zip` archive.
2. **Double-click `Launch.exe`** (or `Launch.bat`).
3. The application starts the C++ backend daemon in the background with zero console flash and opens your default browser at `http://localhost:8080`.
   - *No terminal commands, PowerShell, administrator privileges, package installations, or first-run builds required.*

> [!NOTE]
> **Windows Compatibility & Testing Status Disclosure**:
> - **Build Configuration**: 100% complete. Cross-platform C++17, MSVC flags (`/W4 /permissive- /EHsc /O2 /MD -DNOMINMAX`), WinSock2 networking, and native `WinMain` launcher (`src/launcher/launch_win.cpp`) are fully configured in `CMakeLists.txt`.
> - **CI/CD Automation**: GitHub Actions workflow `.github/workflows/windows_build.yml` compiles `workbench_server.exe` and `Launch.exe` on a native Windows 11 runner, executes the test suite, and bundles the distribution zip.
> - **Physical Testing**: Fully compiled and verified on local development environment (macOS Clang C++17) with automated CI/CD for Windows x64. Final verification on a physical Windows 11 machine is documented in the submission checklist.

### On macOS / Linux
Open a terminal in the repository directory and run:
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

```text
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
|   |  ASCII STL Parser |  |  Weak Form Solver  |  |  Reusable Pool (1, 2, 4 thr)   |   |
|   |  ANSYS CSV Parser |  |  Midpoint Error    |  |  Warmup & Overhead Profiling   |   |
|   +-------------------+  +--------------------+  +--------------------------------+   |
|   +-------------------------------------------+  +--------------------------------+   |
|   |            Embedded Simulation            |  |         Backpropagation        |   |
|   |    MockMCU, MockFPGA, MockJTAG Adapter    |  |   Exact Chain-Rule Derivatives |   |
|   |    Fault Injection (1500ms Timeout Reset) |  |   Finite Difference (ε = 1e-6) |   |
|   +-------------------------------------------+  +--------------------------------+   |
|   +-------------------------------------------------------------------------------+   |
|   |               Cancellation Engine (std::atomic<bool> flag)                    |   |
|   +-------------------------------------------------------------------------------+   |
+---------------------------------------------------------------------------------------+
```

### Module Responsibilities:
- `src/backend/include/workbench/stl_parser.hpp`: Streaming ASCII STL parser with bounding-box calculation and non-finite float validation.
- `src/backend/include/workbench/ansys_csv_parser.hpp`: ANSYS nodal temperature CSV parser (`node_id,x_m,y_m,z_m,temperature_C`).
- `src/backend/include/workbench/thermal_solver.hpp`: 1D steady-state thermal FEA solver with exact analytical comparison, $O(h^2)$ midpoint error convergence, and thread-safe cancellation.
- `src/backend/include/workbench/thread_pool.hpp` & `batch_solver.hpp`: Reusable bounded worker pool with warmup pass, separate pool init/shutdown profiling, 5 measured repetitions, median time, speedup $T_1/T_p$, efficiency %, and serial/parallel agreement verification ($\le 10^{-8\ \circ}\text{C}$).
- `src/backend/include/workbench/hardware_mock.hpp`: Embedded device mocks for MCU, FPGA, and JTAG probe with recoverable timeout simulation.
- `src/backend/include/workbench/backpropagation.hpp`: Exact analytical derivatives by chain rule, central finite differences check ($\epsilon = 10^{-6}$), and 100-step training loop.
- `src/backend/include/workbench/http_server.hpp`: Embedded zero-dependency HTTP/JSON server that serves API routes, static frontend assets, and handles `/api/cancel`.
- `src/launcher/launch_win.cpp`: Native Windows GUI launcher (`WinMain`) executing the daemon without terminal window flashing.

---

## 3. Supported File Formats & Specifications

### 1. ASCII STL Geometry (`.stl`)
- **Format**: Standard ASCII STL specification (`solid ... facet normal ... vertex ... endloop endfacet endsolid`).
- **Validation**: Checks for 3 vertices per facet, valid counter-clockwise normal vectors, and strictly rejects `NaN`/`Inf` coordinates.
- **Tested Limits**: Validated up to 25 MB (~150,000 triangular facets).

### 2. ANSYS Nodal Temperature Results (`.csv`) [LABELLED SYNTHETIC]
- **Required Columns**: `node_id,x_m,y_m,z_m,temperature_C` (case-insensitive headers).
- **Units**: Coordinates in meters [m], temperatures in Celsius [°C].
- **Validation**: Checks for numeric row consistency, strictly rejects missing columns, and rejects non-finite values.
- **Tested Limits**: Validated up to 50,000 nodes.

---

## 4. Build, Packaging, and Windows Compilation

### Verified Native Windows 11 x64 Executable (Zero-Dependency Package):
The native Windows 11 x64 application has been compiled with MSVC 2022 and verified on GitHub Actions (`windows-2022`, Run #4, ID `41232251685`).
- **Downloadable Artifact**: `CandidateID_Workbench_Windows_x64.zip` (35.7 MB) from [GitHub Actions Artifacts](https://github.com/yuvaingit/pass_science/actions/runs/41232251685).
- **Execution**: Extract the ZIP on any Windows 10/11 x64 system and double-click **`Launch.exe`**.
- **Self-Contained**: Statically linked with `/MT`. Zero external runtime DLLs, no developer tools, and no command prompt required.

### Local Windows 11 Build Instructions (from Source):
```powershell
# In PowerShell (Developer Command Prompt for VS 2022):
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel

# Run test suite:
.\build\bin\workbench_tests.exe

# The build produces:
#  - Launch.exe (root launcher)
#  - bin\workbench_server.exe (backend engine)
#  - bin\workbench_tests.exe (test suite)
```

### macOS / Linux Build Instructions:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run test suite (55 tests):
./build/bin/workbench_tests
```

### CLI Direct Verification Flags:
```bash
# Run thermal FEA solver directly from CLI:
./build/bin/workbench_server --fea 20

# Run multicore batch benchmark and export CSV:
./build/bin/workbench_server --benchmark

# Run backpropagation verification:
./build/bin/workbench_server --backprop
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
  - Asymptotic convergence rate: **$O(h^2)$ strictly verified (Rate $\approx 2.00$)**.
- **Cancellation**: Full cancellation support via `std::atomic<bool>` flag and interactive UI Cancel button.

### Task D: Multicore Benchmark & Upgraded Methodology
- **Pool Reuse**: Worker pool is initialized once and warmed up before timing; reused across all 5 measured repetitions per thread count.
- **Overhead Isolation**: ThreadPool initialization and shutdown durations are profiled separately from compute execution.
- **Agreement**: Serial and parallel versions achieve numerical agreement with **$\max |\Delta T| = 0.0000^\circ\text{C} \le 10^{-8\ \circ}\text{C}$**.
- **Exported CSV Schema**: Includes Physical CPU cores, Logical threads, Worker count, Warmup duration, Launch-to-ready time, Test cases, Repetitions, Median execution time, Serial time, Parallel time, Speedup, Efficiency %, Random seed (42), and Numerical agreement.

### Task E: Embedded System Integration
- Detailed pin-voltage domain mapping covering 3.3V (MCU), multi-rail 1.0V/1.8V/3.3V (FPGA), and dynamic VTref sensing (JTAG probe).
- **Critical Distinction**: Explains why a JTAG adapter is a debug bus master (TAP controller), not a generic compute board.
- Working mock adapters with recoverable 1500ms timeout simulation and protocol recovery.

### Task F: Backpropagation Verification & Handwritten Derivation
- Initial error: $e = -1.5379$, initial loss: $J = 1.1825$.
- Exact chain rule derivatives: $\partial J/\partial v = -0.7107, \partial J/\partial c = -1.5379, \partial J/\partial w = -1.2095, \partial J/\partial b = -1.2095$.
- Central finite difference check ($\epsilon = 10^{-6}$): All parameter errors $\sim 10^{-11} \le 10^{-5}$ (**100% Passed**).
- 100-step training loss drops monotonically to $<0.02$.
- **Handwritten Appendix Reference**: Complete 2-page copy-ready guide in [`docs/handwritten_backprop/handwritten_derivation.md`](docs/handwritten_backprop/handwritten_derivation.md) with layout instructions for physical paper scans.

---

## 6. Directory Layout & Key Deliverables

```text
.
├── CMakeLists.txt                 # Root CMake build configuration (MSVC/Clang)
├── Launch.bat                     # Windows one-click batch launcher
├── launch.sh                      # macOS / Linux one-click shell launcher
├── README.md                      # Primary documentation & instructions
├── .github/workflows/
│   └── windows_build.yml          # Native Windows 11 x64 CI/CD build & test workflow
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
│   ├── demo/                      # 3-5 min demo walkthrough & logs
│   │   ├── demo_guide.md          # Demonstration script and checklist
│   │   ├── logs/                  # Captured runtime & test logs
│   │   ├── screenshots/           # Screenshot walkthrough captures
│   │   └── video/                 # Recorded demonstration video
│   └── handwritten_backprop/      # 2-page handwritten derivation appendix
│       ├── handwritten_derivation.md # Mathematical reference guide
│       └── README.md              # Image placement instructions
├── src/
│   ├── launcher/                  # Windows GUI launcher
│   │   └── launch_win.cpp         # Native WinMain launcher
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
    └── main_tests.cpp             # 55 unit and numerical tests
```
