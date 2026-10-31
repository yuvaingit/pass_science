// Unified Engineering Workbench Type Definitions

export type TabType = 'geometry' | 'thermal' | 'performance' | 'hardware' | 'backprop' | 'report';

export interface SystemStatus {
  backendConnected: boolean;
  backendVersion: string;
  os: string;
  cpuModel: string;
  logicalCores: number;
  physicalCores: number;
  ramGb: number;
  compiler: string;
  flags: string;
  readyTimeMs: number;
}

// Task B: Geometry Types
export interface STLData {
  filename: string;
  triangleCount: number;
  vertexCount: number;
  boundingBox: {
    min: [number, number, number];
    max: [number, number, number];
  };
  vertices: number[]; // Flattened [x1, y1, z1, x2, ...]
  normals: number[];  // Flattened per-vertex or per-face
  provenance: string;
  fileSize: number;
}

export interface ANSYSNode {
  id: number;
  x: number;
  y: number;
  z: number;
  temp: number;
}

export interface ANSYSData {
  filename: string;
  nodeCount: number;
  minTemp: number;
  maxTemp: number;
  avgTemp: number;
  nodes: ANSYSNode[];
  provenance: string;
  fileSize: number;
}

// Task C: Thermal FEA Types
export interface FEAParams {
  length: number;       // L = 1.0 m
  area: number;         // A = 1e-4 m^2
  conductivity: number; // k = 10.0 W/(m K)
  heatGen: number;      // q = 10,000 W/m^3
  t0: number;           // T(0) = 100.0 C
  tL: number;           // T(L) = 20.0 C
  elements: number;     // N = 10, 20, 40, 80
}

export interface FEANodeResult {
  index: number;
  x: number;
  tFE: number;
  tExact: number;
  absError: number;
}

export interface FEAMidpointResult {
  elementIndex: number;
  xMid: number;
  tFEMid: number;
  tExactMid: number;
  absErrorMid: number;
}

export interface ConvergenceEntry {
  N: number;
  h: number;
  maxMidpointError: number;
  orderOfConvergence?: number;
}

export interface FEASolveResult {
  params: FEAParams;
  nodes: FEANodeResult[];
  midpoints: FEAMidpointResult[];
  maxMidpointError: number;
  convergenceTable: ConvergenceEntry[];
  solveTimeMs: number;
}

// Task D: Performance Benchmark Types
export interface BenchmarkConfig {
  caseCount: number;
  meshSize: number;
  seed: number;
  kMin: number;
  kMax: number;
  qMin: number;
  qMax: number;
}

export interface ThreadRunRecord {
  threads: number;
  repetitions: number[]; // 5 measured repetitions in ms
  medianMs: number;
  speedup: number;       // T1 / Tp
  efficiency: number;    // (T1 / Tp) / p
  maxTempDiff: number;   // Agreement with serial: <= 1e-8 C
}

export interface BenchmarkResult {
  config: BenchmarkConfig;
  cpuInfo: string;
  logicalCores: number;
  physicalCores: number;
  ramGb: number;
  compiler: string;
  flags: string;
  warmupMs: number;
  runs: ThreadRunRecord[];
  serialParallelAgreementOk: boolean;
  launchToReadyMs: number;
}

// Task E: Hardware Integration Types
export type DeviceType = 'mcu' | 'fpga' | 'jtag';
export type DeviceStatus = 'disconnected' | 'connecting' | 'connected' | 'error' | 'timeout';

export interface DeviceCapabilities {
  name: string;
  type: DeviceType;
  firmware: string;
  voltageDomain: string;
  connector: string;
  protocol: string;
  baudRate?: number;
  jtagClockMhz?: number;
  fpgaPart?: string;
  description: string;
}

export interface TelemetryMessage {
  timestamp: string;
  voltage3v3: number;
  currentMa: number;
  coreTempC: number;
  statusRegister: string;
  packetCount: number;
  lastError?: string;
}

export interface MockDevice {
  id: string;
  name: string;
  type: DeviceType;
  status: DeviceStatus;
  caps: DeviceCapabilities;
  telemetry?: TelemetryMessage;
  errorLog: string[];
}

// Task F: Backpropagation Types
export interface BackpropConfig {
  x: number;       // 1.0
  yTarget: number; // 2.0
  w: number;       // 0.5
  b: number;       // 0.0
  v: number;       // 1.0
  c: number;       // 0.0
  lr: number;      // 0.1
  steps: number;   // 100
}

export interface GradientCheck {
  paramName: string;
  analyticalGrad: number;
  numericalGrad: number;
  absoluteError: number;
  passed: boolean; // error <= 1e-5 with eps = 1e-6
}

export interface TrainingStep {
  step: number;
  w: number;
  b: number;
  v: number;
  c: number;
  h: number;
  yHat: number;
  loss: number;
  gradW: number;
  gradB: number;
  gradV: number;
  gradC: number;
}

export interface BackpropResult {
  config: BackpropConfig;
  forwardInitial: {
    a: number; // wx + b
    h: number; // tanh(a)
    yHat: number;
    loss: number;
  };
  gradientChecks: GradientCheck[];
  trainingHistory: TrainingStep[];
  finalLoss: number;
}
