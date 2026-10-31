#pragma once

#include "workbench/common.hpp"
#include <string>
#include <vector>
#include <array>

namespace workbench {

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct BoundingBox {
    Vec3 min{1e30, 1e30, 1e30};
    Vec3 max{-1e30, -1e30, -1e30};

    void expand(const Vec3& p) {
        if (p.x < min.x) min.x = p.x;
        if (p.y < min.y) min.y = p.y;
        if (p.z < min.z) min.z = p.z;
        if (p.x > max.x) max.x = p.x;
        if (p.y > max.y) max.y = p.y;
        if (p.z > max.z) max.z = p.z;
    }
};

struct Triangle {
    Vec3 normal;
    std::array<Vec3, 3> vertices;
};

struct STLMesh {
    std::string solid_name;
    std::vector<Triangle> triangles;
    BoundingBox bbox;
    size_t file_size_bytes = 0;

    std::string to_json() const;
};

class STLParser {
public:
    // Parse an ASCII STL from a string buffer or file path
    static STLMesh parse_ascii(const std::string& content, const std::string& filename = "geometry.stl");
    static STLMesh parse_file(const std::string& filepath);
};

} // namespace workbench
