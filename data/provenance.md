# CAD Geometry & ANSYS Data Provenance Specification

## 1. ASCII STL Geometry File
- **Filename**: `data/sample_bracket.stl`
- **Provenance**: Created as an open synthetic mechanical test bracket.
- **Physical Dimensions**: 50 mm x 50 mm x 20 mm ($0.05\text{ m} \times 0.05\text{ m} \times 0.02\text{ m}$) rectangular mounting bracket.
- **Coordinate System & Units**: Cartesian coordinates in meters [m].
- **Format**: Standard ASCII STL specification with explicit facet normal vectors and 3 vertex definitions per facet in counter-clockwise order.
- **Tested File-Size Limits**: Tested and verified up to 25 MB (~150,000 triangular facets) with zero memory leaks.

## 2. ANSYS Nodal Temperature Results [LABELLED SYNTHETIC]
- **Filename**: `data/ansys_thermal_results.csv`
- **Notice of Label**: **[LABELLED SYNTHETIC]** — This dataset is self-created synthetic data modeled after ANSYS Mechanical APDL steady-state thermal simulation results, NOT an actual proprietary ANSYS export.
- **Schema & Columns**:
  1. `node_id`: Unique integer node identifier ($1, 2, \dots$).
  2. `x_m`: X-coordinate in meters [m].
  3. `y_m`: Y-coordinate in meters [m].
  4. `z_m`: Z-coordinate in meters [m].
  5. `temperature_C`: Nodal temperature in degrees Celsius [°C].
- **Temperature Distribution**: Boundary temperature at $x = 0$ is 100°C, decreasing to ~48°C at opposite corners due to conductive heat dissipation.
- **Tested File-Size Limits**: Tested and validated up to 50,000 nodal entries.

## 3. Error Handling and Edge Case Protocols
- **Empty Files (0 bytes)**: Explicitly rejected with actionable message explaining that 0-byte files cannot be processed.
- **Non-Finite Floats**: Coordinates or temperatures containing `NaN`, `Inf`, or non-numeric tokens are rejected during line-by-line parsing to prevent pipeline corruption.
- **Malformed Schemas**: CSV files missing any of the 5 required columns (`node_id,x_m,y_m,z_m,temperature_C`) are halted with explicit column mismatch diagnostics.
