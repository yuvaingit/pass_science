import React, { useState, useEffect } from 'react';
import {
  Cpu,
  Radio,
  AlertTriangle,
  CheckCircle,
  RefreshCw,
  Power,
  ShieldAlert,
  Zap,
  Server,
  Terminal,
  Activity,
  Layers,
} from 'lucide-react';
import { MockDevice, DeviceStatus } from '../../types/workbench';
import { WorkbenchAPI } from '../../services/api';

export const HardwareView: React.FC = () => {
  const [devices, setDevices] = useState<MockDevice[]>([]);
  const [selectedDevice, setSelectedDevice] = useState<MockDevice | null>(null);
  const [activeFailureLog, setActiveFailureLog] = useState<string | null>(null);
  const [isRecovering, setIsRecovering] = useState(false);

  useEffect(() => {
    loadDevices();
  }, []);

  const loadDevices = async () => {
    const devs = await WorkbenchAPI.getDevices();
    setDevices(devs);
    if (!selectedDevice && devs.length > 0) {
      setSelectedDevice(devs[0]);
    }
  };

  const handleToggleConnect = (devId: string) => {
    setDevices((prev) =>
      prev.map((d) => {
        if (d.id === devId) {
          const nextStatus: DeviceStatus = d.status === 'connected' ? 'disconnected' : 'connected';
          const updated = {
            ...d,
            status: nextStatus,
            errorLog:
              nextStatus === 'disconnected'
                ? [...d.errorLog, `[${new Date().toLocaleTimeString()}] User manual disconnect initiated.`]
                : [...d.errorLog, `[${new Date().toLocaleTimeString()}] Device reconnected and handshaked successfully.`],
          };
          if (selectedDevice?.id === devId) setSelectedDevice(updated);
          return updated;
        }
        return d;
      })
    );
  };

  const handleInjectTimeoutFailure = (devId: string) => {
    setActiveFailureLog(`Injecting recoverable communication timeout on device [${devId}]...`);
    setDevices((prev) =>
      prev.map((d) => {
        if (d.id === devId) {
          const updated: MockDevice = {
            ...d,
            status: 'timeout',
            errorLog: [
              ...d.errorLog,
              `[${new Date().toLocaleTimeString()}] ERR_COMM_TIMEOUT: Host packet ACK timeout (1500ms exceeded). No response on physical transport.`,
            ],
          };
          if (selectedDevice?.id === devId) setSelectedDevice(updated);
          return updated;
        }
        return d;
      })
    );
  };

  const handleRecoverDevice = (devId: string) => {
    setIsRecovering(true);
    setActiveFailureLog(`Executing automated watchdog reset and protocol recovery sequence on [${devId}]...`);
    setTimeout(() => {
      setDevices((prev) =>
        prev.map((d) => {
          if (d.id === devId) {
            const updated: MockDevice = {
              ...d,
              status: 'connected',
              errorLog: [
                ...d.errorLog,
                `[${new Date().toLocaleTimeString()}] RECOVERY_SUCCESS: Bus drained, target ACK verified, normal telemetry resumed.`,
              ],
            };
            if (selectedDevice?.id === devId) setSelectedDevice(updated);
            return updated;
          }
          return d;
        })
      );
      setIsRecovering(false);
      setActiveFailureLog(null);
    }, 800);
  };

  return (
    <div className="p-6 max-w-7xl mx-auto space-y-6">
      {/* Header */}
      <div className="flex flex-col md:flex-row md:items-center justify-between gap-4 bg-slate-900/80 p-5 rounded-xl border border-slate-800 shadow-sm">
        <div>
          <div className="flex items-center space-x-2">
            <h2 className="text-lg font-bold text-white">Embedded-System Integration Design & Mocks</h2>
            <span className="text-xs px-2 py-0.5 rounded bg-blue-500/10 text-blue-400 border border-blue-500/20 font-mono">
              Task E
            </span>
          </div>
          <p className="text-sm text-slate-400 mt-1">
            Host-connected workbench for Microcontroller, FPGA, and JTAG Debug Adapter with electrical domain mapping and fault injection.
          </p>
        </div>

        <div className="flex items-center space-x-2">
          <span className="text-xs px-3 py-1.5 rounded-lg bg-amber-500/10 border border-amber-500/20 text-amber-300 font-mono">
            Simulated Hardware Mode Active
          </span>
        </div>
      </div>

      {/* Failure Notification Banner */}
      {activeFailureLog && (
        <div className="flex items-start space-x-3 p-4 rounded-xl bg-amber-500/10 border border-amber-500/30 text-amber-200 text-sm">
          <AlertTriangle className="w-5 h-5 text-amber-400 flex-shrink-0 mt-0.5" />
          <div className="flex-1">
            <span className="font-semibold text-amber-300">Fault Injection & Recovery Status: </span>
            <span>{activeFailureLog}</span>
          </div>
        </div>
      )}

      {/* 3 Mock Device Cards (MCU, FPGA, JTAG) */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
        {devices.map((device) => {
          const isSelected = selectedDevice?.id === device.id;
          const isConnected = device.status === 'connected';
          const isTimeout = device.status === 'timeout';

          return (
            <div
              key={device.id}
              onClick={() => setSelectedDevice(device)}
              className={`bg-slate-900 rounded-xl border p-5 cursor-pointer transition-all shadow-md flex flex-col justify-between ${
                isSelected
                  ? 'border-blue-500 ring-1 ring-blue-500/50'
                  : 'border-slate-800 hover:border-slate-700'
              }`}
            >
              <div>
                {/* Header */}
                <div className="flex items-start justify-between">
                  <div className="flex items-center space-x-2.5">
                    <div
                      className={`w-9 h-9 rounded-lg flex items-center justify-center ${
                        device.type === 'mcu'
                          ? 'bg-blue-500/20 text-blue-400'
                          : device.type === 'fpga'
                          ? 'bg-purple-500/20 text-purple-400'
                          : 'bg-emerald-500/20 text-emerald-400'
                      }`}
                    >
                      <Cpu className="w-5 h-5" />
                    </div>
                    <div>
                      <h4 className="text-sm font-bold text-white leading-tight">{device.name}</h4>
                      <span className="text-[10px] font-mono uppercase tracking-wider text-slate-400">
                        {device.caps.type} • {device.caps.firmware}
                      </span>
                    </div>
                  </div>

                  {/* Status Indicator */}
                  <span
                    className={`text-[10px] font-mono px-2 py-0.5 rounded-full flex items-center space-x-1 border ${
                      isConnected
                        ? 'bg-emerald-500/10 text-emerald-400 border-emerald-500/30'
                        : isTimeout
                        ? 'bg-rose-500/10 text-rose-400 border-rose-500/30 animate-pulse'
                        : 'bg-slate-800 text-slate-400 border-slate-700'
                    }`}
                  >
                    <span
                      className={`w-1.5 h-1.5 rounded-full ${
                        isConnected ? 'bg-emerald-400' : isTimeout ? 'bg-rose-400' : 'bg-slate-500'
                      }`}
                    />
                    <span>{device.status.toUpperCase()}</span>
                  </span>
                </div>

                {/* Telemetry Snapshot */}
                {isConnected && device.telemetry && (
                  <div className="mt-4 bg-slate-850 p-3 rounded-lg border border-slate-800 text-xs font-mono space-y-1">
                    <div className="flex justify-between text-slate-400">
                      <span>VCC 3.3V Rail:</span>
                      <span className="text-slate-200 font-semibold">{device.telemetry.voltage3v3.toFixed(3)} V</span>
                    </div>
                    <div className="flex justify-between text-slate-400">
                      <span>Current Draw:</span>
                      <span className="text-blue-300 font-semibold">{device.telemetry.currentMa.toFixed(1)} mA</span>
                    </div>
                    <div className="flex justify-between text-slate-400">
                      <span>Core Temp:</span>
                      <span className="text-amber-300 font-semibold">{device.telemetry.coreTempC.toFixed(1)} °C</span>
                    </div>
                    <div className="flex justify-between text-slate-400">
                      <span>Packets Handled:</span>
                      <span className="text-emerald-300">{device.telemetry.packetCount.toLocaleString()}</span>
                    </div>
                  </div>
                )}

                {isTimeout && (
                  <div className="mt-4 bg-rose-500/10 border border-rose-500/30 p-3 rounded-lg text-xs text-rose-300 space-y-1">
                    <p className="font-bold flex items-center space-x-1">
                      <AlertTriangle className="w-3.5 h-3.5" />
                      <span>Recoverable Fault: Comm Timeout</span>
                    </p>
                    <p className="text-[11px] text-rose-400">
                      Physical layer handshake unresponsive. Ready for automated bus reset.
                    </p>
                  </div>
                )}
              </div>

              {/* Actions */}
              <div className="mt-4 pt-3 border-t border-slate-800 flex items-center gap-2">
                <button
                  onClick={(e) => {
                    e.stopPropagation();
                    handleToggleConnect(device.id);
                  }}
                  className={`flex-1 py-1.5 rounded text-xs font-medium flex items-center justify-center space-x-1 transition ${
                    isConnected
                      ? 'bg-slate-800 hover:bg-slate-700 text-slate-300 border border-slate-700'
                      : 'bg-blue-600 hover:bg-blue-500 text-white'
                  }`}
                >
                  <Power className="w-3 h-3" />
                  <span>{isConnected ? 'Disconnect' : 'Connect'}</span>
                </button>

                {isConnected && (
                  <button
                    onClick={(e) => {
                      e.stopPropagation();
                      handleInjectTimeoutFailure(device.id);
                    }}
                    className="px-2.5 py-1.5 rounded bg-slate-800 hover:bg-rose-900/40 text-rose-300 border border-rose-500/30 text-xs font-medium transition"
                    title="Simulate recoverable communication timeout"
                  >
                    Simulate Timeout
                  </button>
                )}

                {isTimeout && (
                  <button
                    onClick={(e) => {
                      e.stopPropagation();
                      handleRecoverDevice(device.id);
                    }}
                    disabled={isRecovering}
                    className="flex-1 py-1.5 rounded bg-emerald-600 hover:bg-emerald-500 text-white text-xs font-medium flex items-center justify-center space-x-1 transition shadow"
                  >
                    <RefreshCw className={`w-3 h-3 ${isRecovering ? 'animate-spin' : ''}`} />
                    <span>Recover Device</span>
                  </button>
                )}
              </div>
            </div>
          );
        })}
      </div>

      {/* Selected Device Live Console / Telemetry Details */}
      {selectedDevice && (
        <div className="bg-slate-900 p-5 rounded-xl border border-slate-800 space-y-3">
          <div className="flex items-center justify-between">
            <h3 className="text-sm font-semibold text-slate-200 flex items-center space-x-2">
              <Terminal className="w-4 h-4 text-emerald-400" />
              <span>Diagnostic Logs & Interface Details: {selectedDevice.name}</span>
            </h3>
            <span className="text-xs font-mono text-slate-400">
              Protocol: <span className="text-slate-200">{selectedDevice.caps.protocol}</span>
            </span>
          </div>

          <div className="grid grid-cols-1 lg:grid-cols-3 gap-4">
            <div className="bg-slate-850 p-3 rounded-lg border border-slate-800 text-xs space-y-1.5 font-mono">
              <p className="text-slate-400 font-semibold text-slate-200">Hardware Profile:</p>
              <p><span className="text-slate-500">Connector:</span> {selectedDevice.caps.connector}</p>
              <p><span className="text-slate-500">Voltage:</span> {selectedDevice.caps.voltageDomain}</p>
              <p><span className="text-slate-500">Register:</span> {selectedDevice.telemetry?.statusRegister}</p>
            </div>

            <div className="lg:col-span-2 bg-slate-950 p-3 rounded-lg border border-slate-800 font-mono text-[11px] h-32 overflow-y-auto space-y-1">
              <p className="text-slate-500">// Device event trace:</p>
              {selectedDevice.errorLog.length === 0 ? (
                <p className="text-slate-400 italic">No communication faults recorded. Interface running cleanly.</p>
              ) : (
                selectedDevice.errorLog.map((log, i) => (
                  <p
                    key={i}
                    className={
                      log.includes('ERR_') || log.includes('timeout')
                        ? 'text-rose-400'
                        : log.includes('RECOVERY')
                        ? 'text-emerald-400'
                        : 'text-slate-300'
                    }
                  >
                    {log}
                  </p>
                ))
              )}
            </div>
          </div>
        </div>
      )}

      {/* Electrical & Interface Pin-Voltage Table (Task E Core Requirement) */}
      <div className="bg-slate-900 p-5 rounded-xl border border-slate-800 space-y-4">
        <div className="flex items-center justify-between">
          <h3 className="text-sm font-semibold text-slate-200 flex items-center space-x-2">
            <Zap className="w-4 h-4 text-amber-400" />
            <span>Host-Connected Workbench Interface & Pin-Voltage Domain Specification</span>
          </h3>
          <span className="text-xs text-slate-400 font-mono">3 Distinct Voltage Domains</span>
        </div>

        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs font-mono">
            <thead className="bg-slate-850 text-slate-400 border-b border-slate-800">
              <tr>
                <th className="p-3">Device / Subsystem</th>
                <th className="p-3">Power & Grounding Strategy</th>
                <th className="p-3">Operating Voltage Domains</th>
                <th className="p-3">Level Shifting & Isolation</th>
                <th className="p-3">Host Connector & Protocol</th>
                <th className="p-3">JTAG Signal Ownership</th>
                <th className="p-3">Pinout & Hazard Mitigation</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-800/60 text-slate-300">
              {/* Row 1: MCU */}
              <tr className="hover:bg-slate-850/50">
                <td className="p-3 font-semibold text-blue-400">STM32 Microcontroller</td>
                <td className="p-3 text-slate-400">
                  USB 5V $\rightarrow$ on-board LDO (3.3V). Star ground tied to workbench ground plane.
                </td>
                <td className="p-3 text-emerald-400 font-semibold">3.3V Single Domain (VDD)</td>
                <td className="p-3 text-slate-400">
                  Direct 3.3V UART to host bridge; select pins 5V-tolerant (FT).
                </td>
                <td className="p-3 text-slate-200">
                  Micro-USB (VCP) / UART 115200 8N1
                </td>
                <td className="p-3 text-amber-300">
                  JTAG/SWD Target (Slave): SWDIO, SWCLK driven by J-Link adapter.
                </td>
                <td className="p-3 text-slate-400">
                  Reverse voltage diode on USB rail; 10k pull-up on nRESET.
                </td>
              </tr>

              {/* Row 2: FPGA */}
              <tr className="hover:bg-slate-850/50">
                <td className="p-3 font-semibold text-purple-400">Xilinx Artix-7 FPGA</td>
                <td className="p-3 text-slate-400">
                  External 5V/2A barrel jack. Multi-point low-impedance ground plane with TVS diodes.
                </td>
                <td className="p-3 text-emerald-400 font-semibold">
                  Multi-rail: 1.0V (VCCINT), 1.8V (VCCAUX), 3.3V (VCCO Banks)
                </td>
                <td className="p-3 text-slate-400">
                  TXB0108 / TXS0108 bidirectional level shifters between 1.8V bank and 3.3V peripherals.
                </td>
                <td className="p-3 text-slate-200">
                  FTDI FT2232H High-Speed USB-SPI bridge @ 25 MHz
                </td>
                <td className="p-3 text-amber-300">
                  JTAG Chain Target (Slave): Dedicated TAP controller (TCK, TMS, TDI, TDO).
                </td>
                <td className="p-3 text-slate-400">
                  Power sequencing enforced: VCCINT $\rightarrow$ VCCAUX $\rightarrow$ VCCO to avoid latch-up.
                </td>
              </tr>

              {/* Row 3: JTAG */}
              <tr className="hover:bg-slate-850/50">
                <td className="p-3 font-semibold text-amber-400">Segger J-Link JTAG Probe</td>
                <td className="p-3 text-slate-400">
                  Host USB powered. Galvanic isolation prevents ground loops between host and DUT.
                </td>
                <td className="p-3 text-emerald-400 font-semibold">
                  VREF Tracking Domain: 1.2V to 5.0V dynamic sensing
                </td>
                <td className="p-3 text-slate-400">
                  Active internal voltage translators driven by target's Pin 1 (VTref).
                </td>
                <td className="p-3 text-slate-200">
                  USB 2.0 High-Speed / Native JTAG protocol
                </td>
                <td className="p-3 text-amber-300 font-bold">
                  Bus Master (Driver): Drives TCK, TMS, TDI; samples TDO. Controls nTRST & nSRST.
                </td>
                <td className="p-3 text-slate-400">
                  Standard 20-pin / Cortex 10-pin. Floating VREF disables all line drivers to prevent back-powering.
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>

      {/* Critical Architecture Distinction & Safe Integration Plan (Task E Requirement) */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        {/* Why JTAG is NOT an interchangeable compute board */}
        <div className="bg-slate-900 p-5 rounded-xl border border-slate-800 space-y-3">
          <h3 className="text-sm font-semibold text-slate-200 flex items-center space-x-2">
            <ShieldAlert className="w-4 h-4 text-rose-400" />
            <span>Why a JTAG Adapter is a Debug Interface (NOT a Compute Board)</span>
          </h3>
          <div className="text-xs text-slate-300 space-y-2 leading-relaxed">
            <p>
              • <strong>Master/Slave Topology Asymmetry:</strong> A JTAG adapter is a specialized hardware in-circuit emulator and Test Access Port (TAP) state-machine controller. It functions strictly as a <em>bus master</em>, clocking scan-chains (TCK) and shifting bitstreams into boundary-scan registers.
            </p>
            <p>
              • <strong>Incompatible Transport & Purpose:</strong> Unlike microcontrollers or FPGAs, a JTAG probe executes no autonomous user workload, hosts no application firmware, and cannot be treated as an interchangeable compute node. It requires target reference voltage (<code className="text-blue-300">VTref</code>) to bias its output drivers.
            </p>
            <p>
              • <strong>Physical Hazard:</strong> Assuming all three boards share the same connector or voltage domain would cause catastrophic driver contention or back-powering (e.g. driving 3.3V JTAG signals into an unpowered 1.8V FPGA bank).
            </p>
          </div>
        </div>

        {/* Safe Integration Plan and Limitations */}
        <div className="bg-slate-900 p-5 rounded-xl border border-slate-800 space-y-3">
          <h3 className="text-sm font-semibold text-slate-200 flex items-center space-x-2">
            <CheckCircle className="w-4 h-4 text-emerald-400" />
            <span>Safe Hardware Integration Plan & Limitations</span>
          </h3>
          <div className="text-xs text-slate-300 space-y-2 leading-relaxed">
            <p>
              • <strong>Connection Sequence Protocol:</strong>
              1. Bond system ground (GND) across all units first.<br />
              2. Power target FPGA and MCU rails and verify steady-state voltages.<br />
              3. Connect JTAG debug ribbon cable (allowing VTref detection).<br />
              4. Open host serial and SPI communication channels.
            </p>
            <p>
              • <strong>Recoverable Failure Mechanisms:</strong> Software timeouts (1500ms threshold) trigger non-destructive bus resets without toggling board main power supplies, protecting volatile RAM and hardware registers.
            </p>
            <p>
              • <strong>Physical Limitations Acknowledgment:</strong> Per project guidelines, hardware interaction is verified via C++ mock adapters; physical PCB fabrication, silicon routing, and RTL synthesis are outside the required assignment scope.
            </p>
          </div>
        </div>
      </div>
    </div>
  );
};
