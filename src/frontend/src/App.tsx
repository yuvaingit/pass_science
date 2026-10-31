import React, { useState, useEffect } from 'react';
import { Header } from './components/Common/Header';
import { TabBar } from './components/Common/TabBar';
import { GeometryView } from './components/GeometryView/GeometryView';
import { ThermalFEAView } from './components/ThermalFEAView/ThermalFEAView';
import { PerformanceView } from './components/PerformanceView/PerformanceView';
import { HardwareView } from './components/HardwareView/HardwareView';
import { BackpropView } from './components/BackpropView/BackpropView';
import { ReportView } from './components/Common/ReportView';
import { TabType, SystemStatus } from './types/workbench';
import { WorkbenchAPI } from './services/api';

export const App: React.FC = () => {
  const [activeTab, setActiveTab] = useState<TabType>('geometry');
  const [status, setStatus] = useState<SystemStatus>({
    backendConnected: false,
    backendVersion: 'C++17 Workbench Core v1.0.0',
    os: 'Windows 11 x64 / macOS',
    cpuModel: 'Host Multicore Processor',
    logicalCores: 8,
    physicalCores: 4,
    ramGb: 16,
    compiler: 'Clang / MSVC C++17',
    flags: '-O3 -std=c++17 -Wall -Wextra',
    readyTimeMs: 38,
  });

  useEffect(() => {
    // Poll/check backend status
    const checkStatus = async () => {
      const s = await WorkbenchAPI.getStatus();
      setStatus(s);
    };
    checkStatus();
    const interval = setInterval(checkStatus, 5000);
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="min-h-screen bg-slate-950 text-slate-100 flex flex-col font-sans selection:bg-blue-600 selection:text-white">
      {/* Top Header */}
      <Header status={status} />

      {/* Main Tab Navigation */}
      <TabBar activeTab={activeTab} onTabChange={setActiveTab} />

      {/* Main Content Area */}
      <main className="flex-1 pb-12">
        {activeTab === 'geometry' && <GeometryView />}
        {activeTab === 'thermal' && <ThermalFEAView />}
        {activeTab === 'performance' && <PerformanceView />}
        {activeTab === 'hardware' && <HardwareView />}
        {activeTab === 'backprop' && <BackpropView />}
        {activeTab === 'report' && <ReportView />}
      </main>

      {/* Footer */}
      <footer className="bg-slate-900/90 border-t border-slate-800/80 px-6 py-3 text-xs text-slate-400 flex flex-col sm:flex-row items-center justify-between gap-2">
        <div className="flex items-center space-x-2 font-mono text-[11px]">
          <span className="text-slate-300 font-semibold">PASS SCIENCE WORKS</span>
          <span>•</span>
          <span>Common Technical Assignment (100 Points)</span>
          <span>•</span>
          <span className="text-blue-400">Windows 11 x64 Compatible</span>
        </div>
        <div className="flex items-center space-x-4 text-[11px] text-slate-400 font-mono">
          <span>Target Effort: 40-50h</span>
          <span>•</span>
          <span>C++17 Numerical Core</span>
          <span>•</span>
          <span>Open Standards (ASCII STL / ANSYS CSV)</span>
        </div>
      </footer>
    </div>
  );
};
export default App;
