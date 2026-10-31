# Unified Engineering Workbench — 3–5 Minute Demonstration Guide & Verification Script
**Assignment**: PASS Science Works R&D Common Technical Assignment (2026–2027)  
**Target Duration**: 3:30 to 4:45 minutes  
**Candidate ID**: `PASS_RD_2026`

---

## 1. Demonstration Directory Structure

Store all recorded demonstration artifacts in the following repository structure:

```text
docs/demo/
├── demo_guide.md                   <- This comprehensive walkthrough script
├── video/
│   └── workbench_demo_walkthrough.mp4  <- 3-5 minute screen recording with audio narration
├── screenshots/
│   ├── screenshot_01_launch.png        <- Initial launch & system environment specs
│   ├── screenshot_02_fea_mesh.png      <- 1D Thermal FEA temperature profile & convergence table
│   ├── screenshot_03_stl_ansys_3d.png  <- Three.js 3D STL wireframe & ANSYS heat cloud overlay
│   ├── screenshot_04_benchmark.png     <- Multicore benchmark (1, 2, 4 threads, speedup chart)
│   ├── screenshot_05_hardware.png      <- Hardware telemetry (MCU, FPGA, JTAG live stream)
│   ├── screenshot_06_fault_recovery.png<- Simulated timeout injection & auto-recovery
│   └── screenshot_07_backprop.png      <- Backpropagation finite diff gate & 100-step loss curve
└── logs/
    ├── server_launch.log               <- Backend HTTP daemon startup & API request logs
    ├── test_suite.log                  <- C++17 55/55 unit test run output
    └── benchmark_execution.log         <- Multicore benchmark timing capture
```

---

## 2. Step-by-Step 3–5 Minute Video Script & Feature Sequence

### ⏱️ 0:00 – 0:35 | Step 1: One-Click Application Launch & Environment Detection
- **Action**:
  - On Windows: Double-click `Launch.exe` (or `Launch.bat`).
  - On macOS / Linux: Run `./launch.sh`.
- **Expected Visible Output**:
  - The C++17 server starts in background on port `8080`.
  - The default browser opens immediately to `http://localhost:8080`.
  - Header displays detected environment: Host OS, CPU Model, Physical Cores, Logical Threads, RAM, Compiler, and Optimization Flags.
- **Narration Cue**:
  > *"Welcome to the Unified Engineering Workbench. The application starts with a single click, automatically bootstrapping our C++17 core engine and launching the React interface with zero terminal friction or external runtime dependencies."*

---

### ⏱️ 0:35 – 01:20 | Step 2: 1D Thermal FEA Solver & Convergence Study (Task C)
- **Action**:
  1. Click the **Thermal FEA** tab.
  2. Review the governing parameters ($L=1.0\text{ m}, A=10^{-4}\text{ m}^2, k=10\text{ W/m}\cdot\text{K}, q=10000\text{ W/m}^3, T_0=100^\circ\text{C}, T_L=20^\circ\text{C}$).
  3. Select element preset `N=20`, then click **Re-Solve FEA**.
  4. Test cancellation: Select `N=80`, click **Re-Solve FEA**, and immediately click **Cancel**. Observe the Amber `Cancelled` status badge and prompt UI recovery.
  5. Click `N=80` and let it complete.
  6. Click **Export Results CSV** to download `thermal_fea_N80_results.csv`.
- **Expected Visible Output**:
  - Parabolic temperature profile showing $T_{\text{FE}}$ curve matching $T_{\text{exact}}$ analytical solution.
  - Convergence table displaying mesh refinement for $N = 10, 20, 40, 80$ elements.
  - Midpoint error reduces quadratically ($\sim 1.25^\circ\text{C} \to 0.3125^\circ\text{C} \to 0.0781^\circ\text{C} \to 0.0195^\circ\text{C}$) confirming asymptotic 2nd-order convergence ($\text{Rate} \approx 2.00$).
- **Narration Cue**:
  > *"In Task C, our C++ thermal solver implements a direct tridiagonal Thomas algorithm. Notice that nodal temperatures match the analytical solution to machine precision, while element midpoint error exhibits exact second-order convergence with rate 2.00. We also have full thread-safe cancellation controls."*

---

### ⏱️ 01:20 – 02:05 | Step 3: STL Geometry & ANSYS CSV 3D Visualization (Task B)
- **Action**:
  1. Switch to the **Geometry View** tab.
  2. Click **Load Sample Bracket STL** (loads `data/sample_bracket.stl`).
  3. Rotate and zoom the 3D model with the mouse (Three.js OrbitControls).
  4. Toggle the **Wireframe** switch.
  5. Click **Load ANSYS Thermal CSV** (loads `data/ansys_thermal_results.csv`).
  6. Hover over 3D thermal nodes to inspect Raycasted temperature data.
  7. Toggle **Show Synthetic Label** and observe dataset provenance details.
- **Expected Visible Output**:
  - High-fidelity 3D structural bracket rendered with smooth PBR materials.
  - Bounding box metrics: triangle count, vertex count, and exact spatial bounds.
  - Heatmap point cloud overlaid on the CAD geometry colored from blue ($20^\circ\text{C}$) to fiery red ($100^\circ\text{C}$).
  - Raycaster tooltip displays: `Node ID`, `[X, Y, Z]` coordinates, and `Temperature (°C)`.
- **Narration Cue**:
  > *"In Task B, our C++ ASCII STL and ANSYS CSV parsers enforce strict bounding box calculation and NaN/Inf validation. Here in Three.js, the point cloud temperature field is dynamically overlaid on the CAD mesh with interactive raycasting inspection."*

---

### ⏱️ 02:05 – 02:55 | Step 4: Multicore Concurrency Benchmark (Task D)
- **Action**:
  1. Switch to the **Performance** tab.
  2. Review the workload parameters ($1000\text{ batch cases}, 50\text{ mesh elements}, \text{Seed } 42$).
  3. Click **Run Benchmark Batch**.
  4. Observe the Blue `Running` pulse badge and the active **Cancel** button.
  5. Allow the benchmark to complete across 1, 2, and 4 worker threads.
- **Expected Visible Output**:
  - Measured 5-repetition timing table showing:
    - Thread pool initialization overhead ($<0.1\text{ ms}$).
    - Warm-up duration ($<0.2\text{ ms}$).
    - Repetitions 1 to 5 wall-clock durations.
    - Median execution time.
    - Speedup: $\sim 1.8\times$ on 2 threads, $\sim 3.4\times$ to $4.2\times$ on 4 threads.
    - Parallel efficiency: $>85\%$.
    - Numerical Agreement Gate: Green `PASSED (≤ 1e-8 °C)` badge.
  - Interactive bar charts for Speedup scaling and Efficiency percentage.
- **Narration Cue**:
  > *"Task D evaluates CPU concurrency using a bounded worker pool. Our upgraded methodology pre-initializes and warms up the pool before timing, isolating pure computation from thread dispatch overhead. Serial and parallel results achieve strict bitwise agreement within 1e-8 degrees Celsius."*

---

### ⏱️ 02:55 – 03:45 | Step 5: Embedded Hardware Simulation & Fault Recovery (Task E)
- **Action**:
  1. Switch to the **Hardware Lab** tab.
  2. Inspect the 3 distinct device adapters:
     - **STM32F401 Microcontroller Board** (UART 115200 baud, 3.3V)
     - **Xilinx Artix-7 XC7A35T FPGA Board** (SPI 25 MHz, 1.0V/1.8V/3.3V rails)
     - **Segger J-Link / FT2232H JTAG Probe** (Dedicated debug probe, 12 MHz)
  3. Observe live fluctuating telemetry: 3.3V rail voltage, current (mA), core temperature ($^\circ\text{C}$), and hardware status registers.
  4. Click **Inject Timeout (1500 ms)** on the FPGA card.
  5. Observe the status change to amber `TIMEOUT`, error log entry, and failure indicator.
  6. Click **Reset & Recover**. Observe the FPGA transition back to green `CONNECTED`.
- **Expected Visible Output**:
  - Live charts streaming telemetry data in real-time.
  - Clear pinout, voltage domain, and protocol capability badges.
  - Immediate state transition to `TIMEOUT` upon fault injection, followed by instant, verified RTOS recovery.
- **Narration Cue**:
  > *"Task E demonstrates mock hardware adapters for an MCU, FPGA, and dedicated JTAG probe. We provide full protocol specifications, realistic telemetry jitter, and simulated timeout fault injection with graceful recovery."*

---

### ⏱️ 03:45 – 04:30 | Step 6: Neural Backpropagation & Verification (Task F)
- **Action**:
  1. Switch to the **Backpropagation** tab.
  2. Review sample parameters: $x = 1.0$, Target $y = 2.0$, Initial $w=0.5, b=0, v=1.0, c=0$, Learning Rate $\eta=0.1$.
  3. Inspect the **Finite Differences Verification Gate**:
     - All 4 parameters ($w, b, v, c$) show analytical vs numerical gradients matching with errors $< 10^{-10}$ (Threshold: $\le 10^{-5}$).
  4. Inspect the **100-Step Training Loss Graph**:
     - Loss drops smoothly from $J_0 = 1.1825$ down to $< 0.02$.
  5. Show reference link to the **2-page Handwritten Derivation**.
- **Expected Visible Output**:
  - Forward pass box: $a=0.5$, $h=\tanh(0.5)=0.4621$, $\hat{y}=0.4621$, Initial Loss $= 1.1825$.
  - Gradient Table with green `PASSED` badges for all 4 parameters.
  - Interactive Recharts training curve demonstrating monotonic convergence.
- **Narration Cue**:
  > *"Finally, Task F derives backpropagation for a single regression sample. Our C++ engine verifies analytical gradients against central finite differences to within 1e-10 precision, and our 100-step training loop demonstrates smooth loss convergence."*

---

### ⏱️ 04:30 – 04:45 | Step 7: Conclusion & Summary
- **Action**: Switch to the **Report View** tab or return to overview.
- **Narration Cue**:
  > *"All 6 core modules, test suites, and documentation are fully implemented, verified, and packaged. Thank you."*

---

## 3. Terminal Log Capture Instructions

To generate the exact log files for `docs/demo/logs/`:

```bash
# 1. Capture complete test suite execution (55 tests)
./build/bin/workbench_tests > docs/demo/logs/test_suite.log 2>&1

# 2. Capture multicore benchmark CLI execution
./build/bin/workbench_server --benchmark > docs/demo/logs/benchmark_execution.log 2>&1

# 3. Capture server launch and sample requests
./build/bin/workbench_server --port 8080 > docs/demo/logs/server_launch.log 2>&1 &
SERVER_PID=$!
sleep 1
curl -s http://127.0.0.1:8080/api/status >> docs/demo/logs/server_launch.log
kill $SERVER_PID
```
