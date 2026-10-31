# Statement of Authorship & Work Ownership
**Candidate ID**: PASS_RD_2026 | **Project**: Unified Engineering Workbench

## 1. Original Work
The following components were originally conceived, formulated, and implemented for this common technical assignment:
- **C++17 Numerical Core (`src/backend/src/`)**: 
  - 1D thermal FEA weak form assembly, direct Dirichlet boundary condition elimination, and $O(N)$ Thomas tridiagonal algorithm (`thermal_solver.cpp`).
  - Element midpoint linear interpolation error calculation and $O(h^2)$ mesh convergence verification logic.
  - Custom line-by-line ASCII STL streaming parser with bounding-box tracking and non-finite float validation (`stl_parser.cpp`).
  - ANSYS nodal temperature CSV schema parser and statistical aggregator (`ansys_csv_parser.cpp`).
  - Bounded multi-threaded worker pool and deterministic input-ordered batch execution engine with agreement verification (`thread_pool.cpp`, `batch_solver.cpp`).
  - Embedded mock device subsystem for MCU, FPGA, and JTAG probe with recoverable fault injection (`hardware_mock.cpp`).
  - Analytical backpropagation engine with chain-rule derivatives and finite-difference gradient check verification (`backpropagation.cpp`).
- **Frontend Architecture (`src/frontend/src/`)**:
  - Full modular React 18 + TypeScript + Tailwind CSS UI with 6 views (`GeometryView`, `ThermalFEAView`, `PerformanceView`, `HardwareView`, `BackpropView`, `ReportView`).
  - Three.js WebGL canvas with interactive raycasting node inspection and Turbo/Rainbow colormap generation.
  - Interactive Recharts temperature distribution curves, midpoint error plots, benchmark speedup bar charts, and 100-step training loss graphs.
- **Documentation & Verification**:
  - Comprehensive technical report (`docs/report.md`), two-page handwritten backpropagation appendix (`docs/handwritten_backprop/`), and test suite (`tests/main_tests.cpp`).

## 2. Reused Work & Open-Source Libraries
The following openly licensed third-party libraries and standards were utilized:
- **React** (MIT License): UI component hierarchy.
- **Three.js** (MIT License): WebGL 3D rendering and OrbitControls.
- **Recharts** (MIT License): SVG plotting and charting.
- **Tailwind CSS** (MIT License): Modern utility styling.
- **Lucide React** (ISC License): Vector UI icons.
- **Vite** (MIT License): Frontend bundling.

All source code and documentation comply with open-source licensing requirements, contain no proprietary binaries, and involve no confidential or machine-specific dependencies.
