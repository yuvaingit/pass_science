import React from 'react';
import { Box, Flame, Gauge, Cpu, Network, FileText } from 'lucide-react';
import { TabType } from '../../types/workbench';

interface TabBarProps {
  activeTab: TabType;
  onTabChange: (tab: TabType) => void;
}

export const TabBar: React.FC<TabBarProps> = ({ activeTab, onTabChange }) => {
  const tabs: { id: TabType; label: string; icon: React.FC<{ className?: string }>; task: string }[] = [
    { id: 'geometry', label: 'CAD & ANSYS Data', icon: Box, task: 'Task B' },
    { id: 'thermal', label: 'Thermal FEA Solver', icon: Flame, task: 'Task C' },
    { id: 'performance', label: 'Multicore Performance', icon: Gauge, task: 'Task D' },
    { id: 'hardware', label: 'Embedded Systems', icon: Cpu, task: 'Task E' },
    { id: 'backprop', label: 'Backpropagation', icon: Network, task: 'Task F' },
    { id: 'report', label: 'Report & Evidence', icon: FileText, task: 'Rubric' },
  ];

  return (
    <nav className="bg-slate-900/60 border-b border-slate-800 px-6 backdrop-blur-md">
      <div className="flex space-x-1 overflow-x-auto py-2 no-scrollbar">
        {tabs.map((tab) => {
          const Icon = tab.icon;
          const isActive = activeTab === tab.id;
          return (
            <button
              key={tab.id}
              id={`tab-btn-${tab.id}`}
              onClick={() => onTabChange(tab.id)}
              className={`flex items-center space-x-2 px-4 py-2.5 rounded-lg text-sm font-medium transition-all duration-200 whitespace-nowrap ${
                isActive
                  ? 'bg-blue-600 text-white shadow-md shadow-blue-600/30 font-semibold'
                  : 'text-slate-400 hover:text-slate-200 hover:bg-slate-800/60'
              }`}
            >
              <Icon className={`w-4 h-4 ${isActive ? 'text-white' : 'text-slate-400'}`} />
              <span>{tab.label}</span>
              <span
                className={`text-[10px] font-mono px-1.5 py-0.5 rounded ${
                  isActive
                    ? 'bg-blue-700/60 text-blue-100 border border-blue-400/30'
                    : 'bg-slate-800 text-slate-400 border border-slate-700/50'
                }`}
              >
                {tab.task}
              </span>
            </button>
          );
        })}
      </div>
    </nav>
  );
};
