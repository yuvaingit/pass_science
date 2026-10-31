#include "workbench/hardware_mock.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>

namespace workbench {

static std::string current_time_str() {
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    std::ostringstream ss;
    ss << std::put_time(tm, "%H:%M:%S");
    return ss.str();
}

static std::string status_to_str(DeviceStatus st) {
    switch (st) {
        case DeviceStatus::Connected: return "connected";
        case DeviceStatus::Connecting: return "connecting";
        case DeviceStatus::Disconnected: return "disconnected";
        case DeviceStatus::Timeout: return "timeout";
        case DeviceStatus::Error: return "error";
    }
    return "unknown";
}

// -------------------------------------------------------------
// Mock MCU (STM32F401RE Cortex-M4)
// -------------------------------------------------------------

MockMCU::MockMCU() {
    log_.push_back("[" + current_time_str() + "] STM32 Mock Device initialized. VDD 3.3V stable.");
}

DeviceCapabilities MockMCU::get_capabilities() const {
    DeviceCapabilities c;
    c.id = "mcu-01";
    c.name = "STM32F401 Microcontroller Board";
    c.type = DeviceType::MCU;
    c.firmware = "v2.1.4-rtos";
    c.voltage_domain = "3.3V Logic (VDD: 3.3V, select pins 5V-tolerant)";
    c.connector = "Micro-USB VCP / 64-pin Morpho Header";
    c.protocol = "UART @ 115200 baud, 8N1, packetized CRC16";
    c.baud_rate = 115200;
    c.description = "Host-connected sensor telemetry & actuator node";
    return c;
}

DeviceStatus MockMCU::get_status() const { return status_; }

void MockMCU::connect() {
    status_ = DeviceStatus::Connected;
    log_.push_back("[" + current_time_str() + "] Host serial connection established (115200 baud).");
}

void MockMCU::disconnect() {
    status_ = DeviceStatus::Disconnected;
    log_.push_back("[" + current_time_str() + "] Serial port closed by host.");
}

TelemetryPacket MockMCU::read_telemetry() {
    TelemetryPacket p;
    p.timestamp = current_time_str();
    p.voltage_3v3 = 3.298;
    p.current_ma = 42.6;
    p.core_temp_c = 38.4;
    p.status_register = "0x00000001 (SYS_OK | RTOS_RUNNING)";
    p.packet_count = ++packet_counter_;

    if (status_ == DeviceStatus::Timeout) {
        p.error_message = "ERR_COMM_TIMEOUT: UART ACK not received within 1500ms.";
    }
    return p;
}

void MockMCU::inject_failure(const std::string& error_type) {
    status_ = DeviceStatus::Timeout;
    log_.push_back("[" + current_time_str() + "] INJECT_FAILURE: " + error_type + " simulated. Host ACK timer expired (1500ms).");
}

void MockMCU::recover() {
    status_ = DeviceStatus::Connected;
    log_.push_back("[" + current_time_str() + "] RECOVERY_SUCCESS: UART FIFO flushed, target ACK verified, normal telemetry resumed.");
}

std::vector<std::string> MockMCU::get_event_log() const { return log_; }

std::string MockMCU::to_json() const {
    auto caps = get_capabilities();
    auto telem = const_cast<MockMCU*>(this)->read_telemetry();

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{\"id\":\"" << caps.id << "\","
       << "\"name\":\"" << JsonHelper::escape(caps.name) << "\","
       << "\"type\":\"mcu\","
       << "\"status\":\"" << status_to_str(status_) << "\","
       << "\"caps\":{"
       << "\"name\":\"" << JsonHelper::escape(caps.name) << "\","
       << "\"type\":\"mcu\","
       << "\"firmware\":\"" << caps.firmware << "\","
       << "\"voltageDomain\":\"" << JsonHelper::escape(caps.voltage_domain) << "\","
       << "\"connector\":\"" << JsonHelper::escape(caps.connector) << "\","
       << "\"protocol\":\"" << JsonHelper::escape(caps.protocol) << "\","
       << "\"baudRate\":" << caps.baud_rate << ","
       << "\"description\":\"" << JsonHelper::escape(caps.description) << "\"},"
       << "\"telemetry\":{"
       << "\"timestamp\":\"" << telem.timestamp << "\","
       << "\"voltage3v3\":" << telem.voltage_3v3 << ","
       << "\"currentMa\":" << telem.current_ma << ","
       << "\"coreTempC\":" << telem.core_temp_c << ","
       << "\"statusRegister\":\"" << JsonHelper::escape(telem.status_register) << "\","
       << "\"packetCount\":" << telem.packet_count << "},"
       << "\"errorLog\":[";

    for (size_t i = 0; i < log_.size(); ++i) {
        ss << "\"" << JsonHelper::escape(log_[i]) << "\"";
        if (i < log_.size() - 1) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

// -------------------------------------------------------------
// Mock FPGA (Xilinx Artix-7 XC7A35T)
// -------------------------------------------------------------

MockFPGA::MockFPGA() {
    log_.push_back("[" + current_time_str() + "] Artix-7 Mock Device initialized. Multi-rail power sequenced (1.0V/1.8V/3.3V).");
}

DeviceCapabilities MockFPGA::get_capabilities() const {
    DeviceCapabilities c;
    c.id = "fpga-01";
    c.name = "Xilinx Artix-7 XC7A35T Board";
    c.type = DeviceType::FPGA;
    c.firmware = "Bitstream Rev D (Thermal Accel IP)";
    c.voltage_domain = "Multi-rail: 1.0V VCCINT, 1.8V VCCAUX, 3.3V VCCO";
    c.connector = "High-density PMOD & USB-FTDI SPI bridge";
    c.protocol = "SPI Slave Mode 0 @ 25 MHz; framing 0xAA55";
    c.description = "Hardware pipeline coprocessor for matrix acceleration";
    return c;
}

DeviceStatus MockFPGA::get_status() const { return status_; }

void MockFPGA::connect() {
    status_ = DeviceStatus::Connected;
    log_.push_back("[" + current_time_str() + "] FTDI SPI high-speed link synced @ 25 MHz.");
}

void MockFPGA::disconnect() {
    status_ = DeviceStatus::Disconnected;
    log_.push_back("[" + current_time_str() + "] FTDI SPI interface released.");
}

TelemetryPacket MockFPGA::read_telemetry() {
    TelemetryPacket p;
    p.timestamp = current_time_str();
    p.voltage_3v3 = 3.312;
    p.current_ma = 185.3;
    p.core_temp_c = 46.2;
    p.status_register = "0x000000A5 (DONE=1 | MMCM_LOCKED=1 | BUSY=0)";
    p.packet_count = ++packet_counter_;

    if (status_ == DeviceStatus::Timeout) {
        p.error_message = "ERR_SPI_TIMEOUT: Slave select deasserted without packet frame termination.";
    }
    return p;
}

void MockFPGA::inject_failure(const std::string& error_type) {
    status_ = DeviceStatus::Timeout;
    log_.push_back("[" + current_time_str() + "] INJECT_FAILURE: " + error_type + " simulated. SPI MISO timeout.");
}

void MockFPGA::recover() {
    status_ = DeviceStatus::Connected;
    log_.push_back("[" + current_time_str() + "] RECOVERY_SUCCESS: SPI bus reset toggled, MMCM re-locked, telemetry nominal.");
}

std::vector<std::string> MockFPGA::get_event_log() const { return log_; }

std::string MockFPGA::to_json() const {
    auto caps = get_capabilities();
    auto telem = const_cast<MockFPGA*>(this)->read_telemetry();

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{\"id\":\"" << caps.id << "\","
       << "\"name\":\"" << JsonHelper::escape(caps.name) << "\","
       << "\"type\":\"fpga\","
       << "\"status\":\"" << status_to_str(status_) << "\","
       << "\"caps\":{"
       << "\"name\":\"" << JsonHelper::escape(caps.name) << "\","
       << "\"type\":\"fpga\","
       << "\"firmware\":\"" << caps.firmware << "\","
       << "\"voltageDomain\":\"" << JsonHelper::escape(caps.voltage_domain) << "\","
       << "\"connector\":\"" << JsonHelper::escape(caps.connector) << "\","
       << "\"protocol\":\"" << JsonHelper::escape(caps.protocol) << "\","
       << "\"description\":\"" << JsonHelper::escape(caps.description) << "\"},"
       << "\"telemetry\":{"
       << "\"timestamp\":\"" << telem.timestamp << "\","
       << "\"voltage3v3\":" << telem.voltage_3v3 << ","
       << "\"currentMa\":" << telem.current_ma << ","
       << "\"coreTempC\":" << telem.core_temp_c << ","
       << "\"statusRegister\":\"" << JsonHelper::escape(telem.status_register) << "\","
       << "\"packetCount\":" << telem.packet_count << "},"
       << "\"errorLog\":[";

    for (size_t i = 0; i < log_.size(); ++i) {
        ss << "\"" << JsonHelper::escape(log_[i]) << "\"";
        if (i < log_.size() - 1) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

// -------------------------------------------------------------
// Mock JTAG Debug Adapter (Segger J-Link / FT2232H)
// -------------------------------------------------------------

MockJTAG::MockJTAG() {
    log_.push_back("[" + current_time_str() + "] JTAG Debug Adapter initialized. Target VTref tracking active (1.2V-5.0V).");
}

DeviceCapabilities MockJTAG::get_capabilities() const {
    DeviceCapabilities c;
    c.id = "jtag-01";
    c.name = "Segger J-Link / FT2232H Debug Adapter";
    c.type = DeviceType::JTAG;
    c.firmware = "J-Link PRO v11.0 / OpenOCD compatible";
    c.voltage_domain = "Target VREF Tracking (1.2V to 5.0V dynamic sensing)";
    c.connector = "Standard 20-pin 0.1\" / Cortex 10-pin 0.05\" JTAG Header";
    c.protocol = "IEEE 1149.1 JTAG (TCK, TMS, TDI, TDO) / ARM SWD";
    c.jtag_clock_mhz = 12.0;
    c.description = "Dedicated in-circuit debug probe (NOT a generic compute board)";
    return c;
}

DeviceStatus MockJTAG::get_status() const { return status_; }

void MockJTAG::connect() {
    status_ = DeviceStatus::Connected;
    log_.push_back("[" + current_time_str() + "] Target TAP scan chain detected: IDCODE=0x4BA00477 (ARM Cortex-M4 Core).");
}

void MockJTAG::disconnect() {
    status_ = DeviceStatus::Disconnected;
    log_.push_back("[" + current_time_str() + "] Target debug session detached; line drivers high-Z.");
}

TelemetryPacket MockJTAG::read_telemetry() {
    TelemetryPacket p;
    p.timestamp = current_time_str();
    p.voltage_3v3 = 3.305;
    p.current_ma = 18.2;
    p.core_temp_c = 32.1;
    p.status_register = "0x00000010 (TARGET_HALTED=0 | TAP_STATE=RUN_TEST_IDLE)";
    p.packet_count = ++packet_counter_;

    if (status_ == DeviceStatus::Timeout) {
        p.error_message = "ERR_JTAG_TIMEOUT: DAP communication timeout during memory aperture access.";
    }
    return p;
}

void MockJTAG::inject_failure(const std::string& error_type) {
    status_ = DeviceStatus::Timeout;
    log_.push_back("[" + current_time_str() + "] INJECT_FAILURE: " + error_type + " simulated. DAP acknowledge timeout.");
}

void MockJTAG::recover() {
    status_ = DeviceStatus::Connected;
    log_.push_back("[" + current_time_str() + "] RECOVERY_SUCCESS: JTAG TAP state-machine reset (5x TMS=1), core halted & resumed safely.");
}

std::vector<std::string> MockJTAG::get_event_log() const { return log_; }

std::string MockJTAG::to_json() const {
    auto caps = get_capabilities();
    auto telem = const_cast<MockJTAG*>(this)->read_telemetry();

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{\"id\":\"" << caps.id << "\","
       << "\"name\":\"" << JsonHelper::escape(caps.name) << "\","
       << "\"type\":\"jtag\","
       << "\"status\":\"" << status_to_str(status_) << "\","
       << "\"caps\":{"
       << "\"name\":\"" << JsonHelper::escape(caps.name) << "\","
       << "\"type\":\"jtag\","
       << "\"firmware\":\"" << caps.firmware << "\","
       << "\"voltageDomain\":\"" << JsonHelper::escape(caps.voltage_domain) << "\","
       << "\"connector\":\"" << JsonHelper::escape(caps.connector) << "\","
       << "\"protocol\":\"" << JsonHelper::escape(caps.protocol) << "\","
       << "\"jtagClockMhz\":" << caps.jtag_clock_mhz << ","
       << "\"description\":\"" << JsonHelper::escape(caps.description) << "\"},"
       << "\"telemetry\":{"
       << "\"timestamp\":\"" << telem.timestamp << "\","
       << "\"voltage3v3\":" << telem.voltage_3v3 << ","
       << "\"currentMa\":" << telem.current_ma << ","
       << "\"coreTempC\":" << telem.core_temp_c << ","
       << "\"statusRegister\":\"" << JsonHelper::escape(telem.status_register) << "\","
       << "\"packetCount\":" << telem.packet_count << "},"
       << "\"errorLog\":[";

    for (size_t i = 0; i < log_.size(); ++i) {
        ss << "\"" << JsonHelper::escape(log_[i]) << "\"";
        if (i < log_.size() - 1) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

// -------------------------------------------------------------
// HardwareWorkbenchManager
// -------------------------------------------------------------

HardwareWorkbenchManager::HardwareWorkbenchManager() {
    devices_.push_back(std::make_shared<MockMCU>());
    devices_.push_back(std::make_shared<MockFPGA>());
    devices_.push_back(std::make_shared<MockJTAG>());
}

std::vector<std::shared_ptr<IMockDevice>> HardwareWorkbenchManager::list_devices() {
    return devices_;
}

std::shared_ptr<IMockDevice> HardwareWorkbenchManager::get_device(const std::string& id) {
    for (auto& d : devices_) {
        if (d->get_capabilities().id == id) return d;
    }
    return nullptr;
}

std::string HardwareWorkbenchManager::to_json() {
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < devices_.size(); ++i) {
        ss << devices_[i]->to_json();
        if (i < devices_.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

} // namespace workbench
