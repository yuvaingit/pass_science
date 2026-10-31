#include "workbench/ansys_csv_parser.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace workbench {

static std::string to_lower(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

static std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, ',')) {
        size_t first = token.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) {
            tokens.push_back("");
        } else {
            size_t last = token.find_last_not_of(" \t\r\n");
            tokens.push_back(token.substr(first, (last - first + 1)));
        }
    }
    return tokens;
}

ANSYSResults ANSYSCSVParser::parse_csv(const std::string& content, const std::string& filename) {
    if (content.empty()) {
        throw ValidationException("CSV file '" + filename + "' is empty (0 bytes).");
    }

    std::istringstream stream(content);
    std::string line;
    ANSYSResults res;
    res.filename = filename;
    res.file_size_bytes = content.size();

    // Read header line
    if (!std::getline(stream, line)) {
        throw ParseException("Malformed CSV: File has no lines.");
    }

    auto headers = split_csv_line(line);
    std::vector<std::string> lower_headers;
    for (const auto& h : headers) {
        lower_headers.push_back(to_lower(h));
    }

    // Required schema: node_id,x_m,y_m,z_m,temperature_C
    int id_idx = -1, x_idx = -1, y_idx = -1, z_idx = -1, t_idx = -1;
    for (size_t i = 0; i < lower_headers.size(); ++i) {
        if (lower_headers[i] == "node_id") id_idx = static_cast<int>(i);
        else if (lower_headers[i] == "x_m") x_idx = static_cast<int>(i);
        else if (lower_headers[i] == "y_m") y_idx = static_cast<int>(i);
        else if (lower_headers[i] == "z_m") z_idx = static_cast<int>(i);
        else if (lower_headers[i] == "temperature_c") t_idx = static_cast<int>(i);
    }

    if (id_idx == -1 || x_idx == -1 || y_idx == -1 || z_idx == -1 || t_idx == -1) {
        throw ParseException("Invalid CSV Schema in '" + filename + "'. Required column headers: node_id,x_m,y_m,z_m,temperature_C");
    }

    double temp_sum = 0.0;
    size_t line_num = 1;

    while (std::getline(stream, line)) {
        line_num++;
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        auto parts = split_csv_line(line);
        if (parts.size() < 5) {
            throw ParseException("Line " + std::to_string(line_num) + ": Expected at least 5 columns, got " + std::to_string(parts.size()));
        }

        ANSYSNodalResult node;
        try {
            node.node_id = std::stoi(parts[id_idx]);
            node.x_m = std::stod(parts[x_idx]);
            node.y_m = std::stod(parts[y_idx]);
            node.z_m = std::stod(parts[z_idx]);
            node.temperature_C = std::stod(parts[t_idx]);
        } catch (const std::exception& e) {
            throw ValidationException("Line " + std::to_string(line_num) + ": Numeric conversion failure (" + e.what() + ")");
        }

        if (!std::isfinite(node.x_m) || !std::isfinite(node.y_m) || !std::isfinite(node.z_m) || !std::isfinite(node.temperature_C)) {
            throw ValidationException("Line " + std::to_string(line_num) + ": Non-finite (NaN/Inf) coordinate or temperature.");
        }

        if (node.temperature_C < res.min_temp) res.min_temp = node.temperature_C;
        if (node.temperature_C > res.max_temp) res.max_temp = node.temperature_C;
        temp_sum += node.temperature_C;

        res.nodes.push_back(node);
    }

    if (res.nodes.empty()) {
        throw ValidationException("CSV file '" + filename + "' contains no valid data rows.");
    }

    res.avg_temp = temp_sum / res.nodes.size();
    return res;
}

ANSYSResults ANSYSCSVParser::parse_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw WorkbenchException("Could not open ANSYS CSV file: " + filepath);
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return parse_csv(content, filepath);
}

std::string ANSYSResults::to_json() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << "{\"filename\":\"" << JsonHelper::escape(filename) << "\","
       << "\"nodeCount\":" << nodes.size() << ","
       << "\"minTemp\":" << min_temp << ","
       << "\"maxTemp\":" << max_temp << ","
       << "\"avgTemp\":" << avg_temp << ","
       << "\"fileSize\":" << file_size_bytes << ","
       << "\"provenance\":\"ANSYS Nodal CSV parsed and validated in C++17 core.\","
       << "\"nodes\":[";

    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& n = nodes[i];
        ss << "{\"id\":" << n.node_id << ",\"x\":" << n.x_m << ",\"y\":" << n.y_m << ",\"z\":" << n.z_m << ",\"temp\":" << n.temperature_C << "}";
        if (i < nodes.size() - 1) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

} // namespace workbench
