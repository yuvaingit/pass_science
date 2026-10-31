#pragma once

#include "workbench/common.hpp"
#include <string>
#include <vector>

namespace workbench {

struct ANSYSNodalResult {
    int node_id = 0;
    double x_m = 0.0;
    double y_m = 0.0;
    double z_m = 0.0;
    double temperature_C = 0.0;
};

struct ANSYSResults {
    std::string filename;
    std::vector<ANSYSNodalResult> nodes;
    double min_temp = 1e30;
    double max_temp = -1e30;
    double avg_temp = 0.0;
    size_t file_size_bytes = 0;

    std::string to_json() const;
};

class ANSYSCSVParser {
public:
    static ANSYSResults parse_csv(const std::string& content, const std::string& filename = "ansys_thermal_results.csv");
    static ANSYSResults parse_file(const std::string& filepath);
};

} // namespace workbench
