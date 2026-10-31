import React from 'react';
import { Activity, Cpu, Server, CheckCircle2, Clock } from 'lucide-react';
import { SystemStatus } from '../../types/workbench';

interface HeaderProps {
  status: SystemStatus;
}

export const Header: React.FC<HeaderProps> = ({ status }) => {
  return (
    <header className="bg-slate-900 border-b border-slate-800 px-6 py-4 shadow-md sticky top-0 z-50">
      <div className="flex flex-col lg:flex-row lg:items-center lg:justify-between gap-4">
        {/* Brand & Title */}
        <div className="flex items-center space-x-3">
          <div className="w-10 h-10 rounded-lg bg-gradient-to-tr from-blue-600 to-indigo-500 flex items-center justify-center shadow-lg shadow-blue-500/20">
            <Activity className="w-6 h-6 text-white" />
          </div>
          <div>
            <div className="flex items-center space-x-2">
              <h1 className="text-xl font-bold tracking-tight text-white">
                PASS SCIENCE WORKS
              </h1>
              <span className="text-xs font-semibold px-2 py-0.5 rounded bg-blue-500/10 text-blue-400 border border-blue-500/20">
                R&D 2026-2027
              </span>
            </div>
            <p className="text-xs text-slate-400">
              Unified Engineering Workbench • C++17 Core Engine • Cross-Platform Prototype
            </p>
          </div>
        </div>

        {/* System & Telemetry Status Badges */}
        <div className="flex flex-wrap items-center gap-3">
          {/* Backend Status */}
          <div className="flex items-center space-x-2 px-3 py-1.5 rounded-md bg-slate-800/80 border border-slate-700/60 text-xs">
            <Server className={`w-3.5 h-3.5 ${status.backendConnected ? 'text-emerald-400' : 'text-blue-400'}`} />
            <span className="text-slate-400">Core:</span>
            <span className="font-mono font-medium text-slate-200">
              {status.backendConnected ? 'C++17 Daemon (Active)' : 'C++17 Standalone'}
            </span>
            <span className={`w-2 h-2 rounded-full ${status.backendConnected ? 'bg-emerald-400 animate-pulse' : 'bg-blue-400'}`} />
          </div>

          {/* CPU & Cores */}
          <div className="flex items-center space-x-2 px-3 py-1.5 rounded-md bg-slate-800/80 border border-slate-700/60 text-xs">
            <Cpu className="w-3.5 h-3.5 text-indigo-400" />
            <span className="text-slate-400">Hardware:</span>
            <span className="font-mono font-medium text-slate-200">
              {status.logicalCores} Logical / {status.physicalCores} Phys Cores
            </span>
          </div>

          {/* Launch to Ready */}
          <div className="flex items-center space-x-2 px-3 py-1.5 rounded-md bg-slate-800/80 border border-slate-700/60 text-xs">
            <Clock className="w-3.5 h-3.5 text-amber-400" />
            <span className="text-slate-400">Ready Time:</span>
            <span className="font-mono font-medium text-amber-300">
              {status.readyTimeMs} ms
            </span>
          </div>

          {/* Compliance & Validation Badge */}
          <div className="flex items-center space-x-1.5 px-2.5 py-1.5 rounded-md bg-emerald-500/10 border border-emerald-500/30 text-emerald-300 text-xs font-medium">
            <CheckCircle2 className="w-3.5 h-3.5" />
            <span>Strict Verification Pass</span>
          </div>
        </div>
      </div>
    </header>
  );
};
