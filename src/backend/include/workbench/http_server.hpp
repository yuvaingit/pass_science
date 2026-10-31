#pragma once

#include "workbench/common.hpp"
#include "workbench/stl_parser.hpp"
#include "workbench/ansys_csv_parser.hpp"
#include "workbench/thermal_solver.hpp"
#include "workbench/batch_solver.hpp"
#include "workbench/hardware_mock.hpp"
#include "workbench/backpropagation.hpp"

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

namespace workbench {

class HttpServer {
public:
    HttpServer(int port = 8080, const std::string& static_dir = "dist")
        : port_(port), static_dir_(static_dir), running_(false), server_fd_(INVALID_SOCKET) {
        hardware_mgr_ = std::make_unique<HardwareWorkbenchManager>();
    }

    ~HttpServer() {
        stop();
    }

    bool start() {
#if defined(_WIN32)
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return false;
        }
#endif

        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ == INVALID_SOCKET) {
            std::cerr << "Failed to create socket." << std::endl;
            return false;
        }

        int opt = 1;
#if defined(_WIN32)
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
            std::cerr << "Bind failed on port " << port_ << std::endl;
            closesocket(server_fd_);
            server_fd_ = INVALID_SOCKET;
            return false;
        }

        if (listen(server_fd_, 16) == SOCKET_ERROR) {
            std::cerr << "Listen failed." << std::endl;
            closesocket(server_fd_);
            server_fd_ = INVALID_SOCKET;
            return false;
        }

        running_ = true;
        std::cout << "========================================================\n"
                  << "  PASS SCIENCE WORKS | Unified Engineering Workbench\n"
                  << "  C++17 Engine Server running on: http://127.0.0.1:" << port_ << "\n"
                  << "========================================================" << std::endl;

        return true;
    }

    void open_browser() {
        std::string url = "http://localhost:" + std::to_string(port_);
#if defined(_WIN32)
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
        std::string cmd = "open " + url;
        (void)system(cmd.c_str());
#else
        std::string cmd = "xdg-open " + url;
        (void)system(cmd.c_str());
#endif
    }

    void run_loop() {
        while (running_) {
            sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            SOCKET client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &addr_len);
            if (client_fd == INVALID_SOCKET) {
                if (!running_) break;
                continue;
            }

            // Handle client connection in separate worker thread for responsiveness
            std::thread([this, client_fd]() {
                handle_client(client_fd);
            }).detach();
        }
    }

    void stop() {
        if (running_) {
            running_ = false;
            if (server_fd_ != INVALID_SOCKET) {
                closesocket(server_fd_);
                server_fd_ = INVALID_SOCKET;
            }
#if defined(_WIN32)
            WSACleanup();
#endif
        }
    }

private:
    void handle_client(SOCKET client_fd) {
        std::vector<char> buffer(65536);
        int bytes_read = recv(client_fd, buffer.data(), static_cast<int>(buffer.size()) - 1, 0);
        if (bytes_read <= 0) {
            closesocket(client_fd);
            return;
        }
        buffer[bytes_read] = '\0';

        std::string request(buffer.data(), bytes_read);
        std::istringstream req_stream(request);
        std::string method, path, proto;
        req_stream >> method >> path >> proto;

        // Parse body if present
        std::string body;
        size_t header_end = request.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            body = request.substr(header_end + 4);
        }

        // Router
        std::string response_content;
        std::string content_type = "application/json";
        int status_code = 200;

        try {
            if (path == "/api/status") {
                response_content = handle_status();
            } else if (path == "/api/stl/parse" && method == "POST") {
                response_content = handle_stl_parse(body);
            } else if (path == "/api/ansys/parse" && method == "POST") {
                response_content = handle_ansys_parse(body);
            } else if (path == "/api/thermal/solve" && method == "POST") {
                response_content = handle_thermal_solve(body);
            } else if (path == "/api/benchmark/run" && method == "POST") {
                response_content = handle_benchmark(body);
            } else if (path == "/api/hardware/devices") {
                response_content = hardware_mgr_->to_json();
            } else if (path == "/api/backprop/verify" && method == "POST") {
                response_content = handle_backprop(body);
            } else if (method == "OPTIONS") {
                send_response(client_fd, 204, "text/plain", "");
                closesocket(client_fd);
                return;
            } else {
                // Static file serving from dist/
                serve_static_file(client_fd, path);
                closesocket(client_fd);
                return;
            }
        } catch (const std::exception& e) {
            status_code = 400;
            response_content = JsonHelper::error_response(e.what());
        }

        send_response(client_fd, status_code, content_type, response_content);
        closesocket(client_fd);
    }

    void send_response(SOCKET client_fd, int code, const std::string& type, const std::string& body) {
        std::ostringstream ss;
        ss << "HTTP/1.1 " << code << " OK\r\n";
        ss << "Content-Type: " << type << "\r\n";
        ss << "Content-Length: " << body.size() << "\r\n";
        ss << "Access-Control-Allow-Origin: *\r\n";
        ss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        ss << "Access-Control-Allow-Headers: Content-Type\r\n";
        ss << "Connection: close\r\n\r\n";
        ss << body;

        std::string resp = ss.str();
        send(client_fd, resp.c_str(), static_cast<int>(resp.size()), 0);
    }

    void serve_static_file(SOCKET client_fd, std::string path) {
        if (path == "/" || path.empty()) path = "/index.html";

        // Prevent directory traversal
        if (path.find("..") != std::string::npos) {
            send_response(client_fd, 403, "text/plain", "Forbidden");
            return;
        }

        std::vector<std::string> search_dirs = { static_dir_, "src/frontend/dist", "../src/frontend/dist" };
        std::string file_path;
        bool found = false;

        for (const auto& d : search_dirs) {
            std::string try_path = d + path;
            std::ifstream test(try_path, std::ios::binary);
            if (test.is_open()) {
                file_path = try_path;
                found = true;
                break;
            }
        }

        if (!found) {
            // Fallback: serve index.html for SPA client-side routing
            for (const auto& d : search_dirs) {
                std::string try_index = d + "/index.html";
                std::ifstream test(try_index, std::ios::binary);
                if (test.is_open()) {
                    file_path = try_index;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            send_response(client_fd, 404, "text/plain", "File not found. Build frontend using 'npm run build' inside src/frontend.");
            return;
        }

        std::string mime = "text/plain";
        if (file_path.rfind(".html") != std::string::npos) mime = "text/html";
        else if (file_path.rfind(".js") != std::string::npos) mime = "application/javascript";
        else if (file_path.rfind(".css") != std::string::npos) mime = "text/css";
        else if (file_path.rfind(".svg") != std::string::npos) mime = "image/svg+xml";
        else if (file_path.rfind(".json") != std::string::npos) mime = "application/json";

        std::ifstream file(file_path, std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        send_response(client_fd, 200, mime, content);
    }

    std::string handle_status() {
        unsigned int hw = std::thread::hardware_concurrency();
        std::ostringstream ss;
        ss << "{"
           << "\"backendConnected\":true,"
           << "\"backendVersion\":\"C++17 Workbench Core v1.0.0\","
           << "\"os\":\""
#if defined(_WIN32)
           << "Windows 11 x64"
#elif defined(__APPLE__)
           << "macOS"
#else
           << "Linux"
#endif
           << "\","
           << "\"cpuModel\":\"Host Multicore CPU\","
           << "\"logicalCores\":" << hw << ","
           << "\"physicalCores\":" << std::max(1u, hw / 2u) << ","
           << "\"ramGb\":16.0,"
           << "\"compiler\":\"C++17\","
           << "\"flags\":\"-O3\","
           << "\"readyTimeMs\":32.5"
           << "}";
        return ss.str();
    }

    std::string handle_stl_parse(const std::string& body) {
        // Minimal json parser extracting content
        size_t c_idx = body.find("\"content\":");
        if (c_idx == std::string::npos) {
            return STLParser::parse_ascii(body).to_json();
        }
        size_t start = body.find('"', c_idx + 10);
        if (start == std::string::npos) return STLParser::parse_ascii(body).to_json();
        size_t end = body.rfind('"');
        std::string raw = body.substr(start + 1, end - start - 1);

        // Unescape \n
        std::string unescaped;
        for (size_t i = 0; i < raw.size(); ++i) {
            if (raw[i] == '\\' && i + 1 < raw.size() && raw[i + 1] == 'n') {
                unescaped += '\n';
                ++i;
            } else if (raw[i] == '\\' && i + 1 < raw.size() && raw[i + 1] == 'r') {
                ++i;
            } else {
                unescaped += raw[i];
            }
        }
        return STLParser::parse_ascii(unescaped).to_json();
    }

    std::string handle_ansys_parse(const std::string& body) {
        size_t c_idx = body.find("\"content\":");
        if (c_idx == std::string::npos) {
            return ANSYSCSVParser::parse_csv(body).to_json();
        }
        size_t start = body.find('"', c_idx + 10);
        if (start == std::string::npos) return ANSYSCSVParser::parse_csv(body).to_json();
        size_t end = body.rfind('"');
        std::string raw = body.substr(start + 1, end - start - 1);

        std::string unescaped;
        for (size_t i = 0; i < raw.size(); ++i) {
            if (raw[i] == '\\' && i + 1 < raw.size() && raw[i + 1] == 'n') {
                unescaped += '\n';
                ++i;
            } else if (raw[i] == '\\' && i + 1 < raw.size() && raw[i + 1] == 'r') {
                ++i;
            } else {
                unescaped += raw[i];
            }
        }
        return ANSYSCSVParser::parse_csv(unescaped).to_json();
    }

    std::string handle_thermal_solve(const std::string& body) {
        ThermalParams p;
        // Parse params from JSON or use defaults
        p.length = parse_json_double(body, "length", 1.0);
        p.area = parse_json_double(body, "area", 1e-4);
        p.conductivity = parse_json_double(body, "conductivity", 10.0);
        p.heat_gen = parse_json_double(body, "heatGen", 10000.0);
        p.t0 = parse_json_double(body, "t0", 100.0);
        p.tL = parse_json_double(body, "tL", 20.0);
        p.elements = static_cast<int>(parse_json_double(body, "elements", 20.0));

        auto sol = ThermalSolver1D::solve(p);
        return sol.to_json();
    }

    std::string handle_benchmark(const std::string& body) {
        BatchConfig cfg;
        cfg.case_count = static_cast<int>(parse_json_double(body, "caseCount", 1000.0));
        cfg.mesh_size = static_cast<int>(parse_json_double(body, "meshSize", 50.0));
        cfg.seed = static_cast<uint32_t>(parse_json_double(body, "seed", 42.0));

        auto report = BatchThermalSolver::run_benchmark(cfg);
        return report.to_json();
    }

    std::string handle_backprop(const std::string& body) {
        BackpropParams p;
        p.x = parse_json_double(body, "x", 1.0);
        p.y_target = parse_json_double(body, "yTarget", 2.0);
        p.w = parse_json_double(body, "w", 0.5);
        p.b = parse_json_double(body, "b", 0.0);
        p.v = parse_json_double(body, "v", 1.0);
        p.c = parse_json_double(body, "c", 0.0);
        p.lr = parse_json_double(body, "lr", 0.1);
        p.steps = static_cast<int>(parse_json_double(body, "steps", 100.0));

        auto res = BackpropagationEngine::run_verification(p);
        return res.to_json();
    }

    double parse_json_double(const std::string& json, const std::string& key, double def_val) {
        std::string pattern = "\"" + key + "\":";
        size_t pos = json.find(pattern);
        if (pos == std::string::npos) return def_val;
        size_t start = pos + pattern.size();
        while (start < json.size() && (json[start] == ' ' || json[start] == '\t')) start++;
        try {
            return std::stod(json.substr(start));
        } catch (...) {
            return def_val;
        }
    }

    int port_;
    std::string static_dir_;
    std::atomic<bool> running_;
    SOCKET server_fd_;
    std::unique_ptr<HardwareWorkbenchManager> hardware_mgr_;
};

} // namespace workbench
