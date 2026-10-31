# AI-Use Log & Critical Technical Judgement
**Candidate ID**: PASS_RD_2026 | **Project**: Unified Engineering Workbench

---

## 1. AI Tools & Models Utilized
- **AI Coding Assistant**: Google DeepMind Antigravity AI Assistant (Gemini 3.8 Flash High Architecture)
- **Primary Domain Skills**: C++17 Numerical Systems, Finite Element Weak Form Assembly, Multithreading Concurrency, WebGL/Three.js 3D Visualization, Embedded Hardware Interfaces.

---

## 2. Representative Interaction Log & Iterative Refinement

### Interaction 1: Thermal FEA Formulation & Assembly
- **Representative Prompt**: 
  *"Derive the element stiffness matrix and load vector for 1D steady-state heat conduction with uniform volumetric heat generation q. Solve -d/dx(kA dT/dx) = qA with prescribed Dirichlet BCs T(0)=100 and T(L)=20 using 2-node linear elements."*
- **Assistant Output**: 
  Generated weak form integral and element stiffness $k_e = \frac{kA}{h}\begin{bmatrix}1 & -1 \\ -1 & 1\end{bmatrix}$ and load $f_e = \frac{qAh}{2}\begin{bmatrix}1 \\ 1\end{bmatrix}$. Suggested applying boundary conditions via penalty method ($10^{15} \times T$).
- **Candidate Critical Edit & Judgement**: 
  **Rejected penalty method suggestion.** In double-precision floating-point arithmetic, adding $10^{15}$ to matrix diagonals introduces conditioning errors and matrix ill-conditioning. Instead, implemented **exact direct substitution / row-column elimination** into the interior equations, preserving symmetric positive-definiteness and enabling the $O(N)$ tridiagonal Thomas Algorithm (TDMA) to run without conditioning loss.
- **Validation Evidence**: 
  Nodal solution $|T_i - T_{\text{exact}}(x_i)| < 10^{-12\ \circ}\text{C}$ to machine precision.

---

### Interaction 2: FEA Error Assessment & Convergence Rate
- **Representative Prompt**: 
  *"Check the convergence rate of the 1D thermal solver across N = 10, 20, 40, 80 elements by comparing FEA nodes against exact solution T_exact(x)."*
- **Assistant Output**: 
  Computed $|T_i - T_{\text{exact}}(x_i)|$ at nodes and reported that error was zero across all $N$, concluding the model had converged with zero error.
- **Candidate Critical Edit & Judgement (Deliberate Challenge Test)**:
  Identified this as the **"Nodal Exactness Trap"**. In 1D linear FEA with constant source $q$, the finite element weak form matches the exact Green's function at element vertices. Evaluating error only at the nodes masks the true polynomial discretization error. 
  **Candidate Resolution**: Formulated and implemented midpoint interpolation ($x_{\text{mid}} = (x_i + x_{i+1})/2, T_{\text{mid}} = 0.5(T_i + T_{i+1})$). 
- **Validation Evidence**:
  Midpoint errors at $N = 10, 20, 40, 80$ yielded $1.25^\circ\text{C}, 0.3125^\circ\text{C}, 0.078125^\circ\text{C}, 0.019531^\circ\text{C}$, confirming the theoretical error formula $\frac{q}{8k}h^2$ and proving asymptotic $O(h^2)$ convergence.

---

### Interaction 3: Embedded Hardware Workbench Architecture
- **Representative Prompt**: 
  *"Design a host-connected workbench for an STM32 MCU, an Artix-7 FPGA, and a JTAG adapter. Can we share a unified 3.3V header for all three?"*
- **Assistant Output**: 
  Suggested placing all three devices onto a shared 3.3V power and communication bus.
- **Candidate Critical Edit & Judgement**:
  **Rejected unsuitable suggestion.** The Artix-7 FPGA operates with multi-rail voltage domains (1.0V VCCINT, 1.8V VCCAUX, 3.3V VCCO), requiring level shifting (e.g. TXB0108). Furthermore, a JTAG adapter is a dedicated in-circuit TAP master/probe with variable target voltage sensing ($V_{\text{Tref}}$), not a general-purpose compute node. Assuming common pins would result in driver contention and chip damage.
- **Validation Evidence**:
  Designed separate voltage domains, explicit JTAG master/slave signal ownership, and simulated timeout fault injection with recovery routines in `hardware_mock.cpp`.

---

## 3. Deliberate Challenge Test & Outcome
- **Challenge**: Feed malformed non-finite ASCII STL data (`vertex NaN 0.0 0.0`) and non-compliant CSV columns (`node_id,x_m,y_m,temperature_C` missing `z_m`) to the C++ parser.
- **Outcome**: The C++ core successfully trapped both errors, rejecting malformed files with actionable diagnostics (`ValidationException: Non-finite coordinate detected` and `ParseException: Missing required column z_m`). Zero pipeline crashes or corrupted states occurred.
