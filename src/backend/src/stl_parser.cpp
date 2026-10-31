#include "workbench/stl_parser.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <cmath>

namespace workbench {

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

STLMesh STLParser::parse_ascii(const std::string& content, const std::string& filename) {
    if (content.empty()) {
        throw ValidationException("File '" + filename + "' is empty (0 bytes).");
    }

    std::istringstream stream(content);
    std::string line;
    STLMesh mesh;
    mesh.file_size_bytes = content.size();

    // Read first non-empty line
    bool found_solid = false;
    while (std::getline(stream, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

        if (trimmed.rfind("solid", 0) == 0) {
            found_solid = true;
            if (trimmed.size() > 6) {
                mesh.solid_name = trim(trimmed.substr(6));
            } else {
                mesh.solid_name = "unnamed_solid";
            }
            break;
        } else {
            throw ParseException("Malformed STL in '" + filename + "': Expected 'solid <name>' header as first line.");
        }
    }

    if (!found_solid) {
        throw ParseException("Malformed STL in '" + filename + "': Missing 'solid' declaration.");
    }

    Triangle current_triangle;
    int vertex_count_in_facet = 0;
    bool in_facet = false;
    bool in_loop = false;
    size_t line_number = 1;

    while (std::getline(stream, line)) {
        line_number++;
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

        if (trimmed.rfind("facet normal", 0) == 0) {
            if (in_facet) {
                throw ParseException("Line " + std::to_string(line_number) + ": Unexpected nested 'facet normal'.");
            }
            in_facet = true;
            vertex_count_in_facet = 0;

            std::istringstream ls(trimmed.substr(12));
            if (!(ls >> current_triangle.normal.x >> current_triangle.normal.y >> current_triangle.normal.z)) {
                throw ParseException("Line " + std::to_string(line_number) + ": Malformed facet normal vector.");
            }
            if (!std::isfinite(current_triangle.normal.x) ||
                !std::isfinite(current_triangle.normal.y) ||
                !std::isfinite(current_triangle.normal.z)) {
                throw ValidationException("Line " + std::to_string(line_number) + ": Non-finite (NaN/Inf) normal coordinate detected.");
            }
        } else if (trimmed.rfind("outer loop", 0) == 0) {
            if (!in_facet) {
                throw ParseException("Line " + std::to_string(line_number) + ": 'outer loop' outside of facet.");
            }
            in_loop = true;
        } else if (trimmed.rfind("vertex", 0) == 0) {
            if (!in_loop) {
                throw ParseException("Line " + std::to_string(line_number) + ": 'vertex' outside of outer loop.");
            }
            if (vertex_count_in_facet >= 3) {
                throw ParseException("Line " + std::to_string(line_number) + ": Polygon with more than 3 vertices detected (STL requires strictly 3 vertices per facet).");
            }

            Vec3 v;
            std::istringstream ls(trimmed.substr(6));
            if (!(ls >> v.x >> v.y >> v.z)) {
                throw ParseException("Line " + std::to_string(line_number) + ": Malformed vertex coordinates.");
            }
            if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z)) {
                throw ValidationException("Line " + std::to_string(line_number) + ": Non-finite (NaN/Inf) vertex coordinate detected.");
            }

            current_triangle.vertices[vertex_count_in_facet++] = v;
            mesh.bbox.expand(v);
        } else if (trimmed.rfind("endloop", 0) == 0) {
            if (!in_loop || vertex_count_in_facet != 3) {
                throw ParseException("Line " + std::to_string(line_number) + ": 'endloop' with incomplete vertices (found " + std::to_string(vertex_count_in_facet) + ", expected 3).");
            }
            in_loop = false;
        } else if (trimmed.rfind("endfacet", 0) == 0) {
            if (!in_facet) {
                throw ParseException("Line " + std::to_string(line_number) + ": 'endfacet' without matching facet normal.");
            }
            mesh.triangles.push_back(current_triangle);
            in_facet = false;
        } else if (trimmed.rfind("endsolid", 0) == 0) {
            break;
        }
    }

    if (mesh.triangles.empty()) {
        throw ValidationException("File '" + filename + "' contains no valid triangular facets.");
    }

    return mesh;
}

STLMesh STLParser::parse_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw WorkbenchException("Could not open STL file: " + filepath);
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return parse_ascii(content, filepath);
}

std::string STLMesh::to_json() const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << "{\"filename\":\"" << JsonHelper::escape(solid_name) << "\","
       << "\"triangleCount\":" << triangles.size() << ","
       << "\"vertexCount\":" << (triangles.size() * 3) << ","
       << "\"fileSize\":" << file_size_bytes << ","
       << "\"boundingBox\":{\"min\":[" << bbox.min.x << "," << bbox.min.y << "," << bbox.min.z << "],"
       << "\"max\":[" << bbox.max.x << "," << bbox.max.y << "," << bbox.max.z << "]},"
       << "\"vertices\":[";

    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto& t = triangles[i];
        for (int v = 0; v < 3; ++v) {
            ss << t.vertices[v].x << "," << t.vertices[v].y << "," << t.vertices[v].z;
            if (i < triangles.size() - 1 || v < 2) ss << ",";
        }
    }
    ss << "],\"normals\":[";
    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto& t = triangles[i];
        for (int v = 0; v < 3; ++v) {
            ss << t.normal.x << "," << t.normal.y << "," << t.normal.z;
            if (i < triangles.size() - 1 || v < 2) ss << ",";
        }
    }
    ss << "],\"provenance\":\"ASCII STL parsed and validated in C++17 core.\"}";
    return ss.str();
}

} // namespace workbench
