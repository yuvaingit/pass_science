#pragma once

#include "workbench/common.hpp"
#include <string>
#include <vector>
#include <memory>

namespace workbench {

enum class DeviceType {
    MCU,
    FPGA,
    JTAG
};

enum class DeviceStatus {
    Disconnected,
    Connecting,
    Connected,
    Timeout,
    Error
};

struct DeviceCapabilities {
    std::string id;
    std::string name;
    DeviceType type;
    std::string firmware;
    std::string voltage_domain;
    std::string connector;
    std::string protocol;
    int baud_rate = 115200;
    double jtag_clock_mhz = 0.0;
    std::string description;
};

struct TelemetryPacket {
    std::string timestamp;
    double voltage_3v3 = 3.3;
    double current_ma = 50.0;
    double core_temp_c = 40.0;
    std::string status_register;
    uint32_t packet_count = 0;
    std::string error_message;
};

class IMockDevice {
public:
    virtual ~IMockDevice() = default;
    virtual DeviceCapabilities get_capabilities() const = 0;
    virtual DeviceStatus get_status() const = 0;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual TelemetryPacket read_telemetry() = 0;
    virtual void inject_failure(const std::string& error_type) = 0;
    virtual void recover() = 0;
    virtual std::vector<std::string> get_event_log() const = 0;
    virtual std::string to_json() const = 0;
};

class MockMCU : public IMockDevice {
public:
    MockMCU();
    DeviceCapabilities get_capabilities() const override;
    DeviceStatus get_status() const override;
    void connect() override;
    void disconnect() override;
    TelemetryPacket read_telemetry() override;
    void inject_failure(const std::string& error_type) override;
    void recover() override;
    std::vector<std::string> get_event_log() const override;
    std::string to_json() const override;

private:
    DeviceStatus status_{DeviceStatus::Connected};
    uint32_t packet_counter_{1200};
    std::vector<std::string> log_;
};

class MockFPGA : public IMockDevice {
public:
    MockFPGA();
    DeviceCapabilities get_capabilities() const override;
    DeviceStatus get_status() const override;
    void connect() override;
    void disconnect() override;
    TelemetryPacket read_telemetry() override;
    void inject_failure(const std::string& error_type) override;
    void recover() override;
    std::vector<std::string> get_event_log() const override;
    std::string to_json() const override;

private:
    DeviceStatus status_{DeviceStatus::Connected};
    uint32_t packet_counter_{3400};
    std::vector<std::string> log_;
};

class MockJTAG : public IMockDevice {
public:
    MockJTAG();
    DeviceCapabilities get_capabilities() const override;
    DeviceStatus get_status() const override;
    void connect() override;
    void disconnect() override;
    TelemetryPacket read_telemetry() override;
    void inject_failure(const std::string& error_type) override;
    void recover() override;
    std::vector<std::string> get_event_log() const override;
    std::string to_json() const override;

private:
    DeviceStatus status_{DeviceStatus::Connected};
    uint32_t packet_counter_{512};
    std::vector<std::string> log_;
};

class HardwareWorkbenchManager {
public:
    HardwareWorkbenchManager();

    std::vector<std::shared_ptr<IMockDevice>> list_devices();
    std::shared_ptr<IMockDevice> get_device(const std::string& id);
    std::string to_json();

private:
    std::vector<std::shared_ptr<IMockDevice>> devices_;
};

} // namespace workbench
