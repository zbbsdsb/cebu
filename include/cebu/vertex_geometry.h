#pragma once

#include "cebu/common.h"
#include <vector>
#include <unordered_map>
#include <array>
#include <optional>
#include <cmath>
#include <limits>

namespace cebu {

/**
 * @brief 3D point structure
 */
struct Point3D {
    float x, y, z;

    Point3D() : x(0), y(0), z(0) {}
    Point3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    // Addition
    Point3D operator+(const Point3D& other) const {
        return Point3D(x + other.x, y + other.y, z + other.z);
    }

    // Subtraction
    Point3D operator-(const Point3D& other) const {
        return Point3D(x - other.x, y - other.y, z - other.z);
    }

    // Scalar multiplication
    Point3D operator*(float scalar) const {
        return Point3D(x * scalar, y * scalar, z * scalar);
    }

    // Dot product
    float dot(const Point3D& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // Squared distance
    float squared_distance_to(const Point3D& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return dx * dx + dy * dy + dz * dz;
    }

    // Distance
    float distance_to(const Point3D& other) const {
        return std::sqrt(squared_distance_to(other));
    }
};

/**
 * @brief Axis-aligned bounding box (AABB)
 */
struct BoundingBox {
    Point3D min;
    Point3D max;

    BoundingBox() : min(std::numeric_limits<float>::max()),
                    max(std::numeric_limits<float>::lowest()) {}

    BoundingBox(const Point3D& min_, const Point3D& max_)
        : min(min_), max(max_) {}

    /**
     * @brief Add a point to the bounding box, expanding it if necessary
     */
    void expand(const Point3D& point) {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    /**
     * @brief Add another bounding box to this one
     */
    void expand(const BoundingBox& other) {
        expand(other.min);
        expand(other.max);
    }

    /**
     * @brief Check if a point is inside the bounding box
     */
    bool contains(const Point3D& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    /**
     * @brief Check if this bounding box intersects with another
     */
    bool intersects(const BoundingBox& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }

    /**
     * @brief Get the center of the bounding box
     */
    Point3D center() const {
        return Point3D((min.x + max.x) * 0.5f,
                       (min.y + max.y) * 0.5f,
                       (min.z + max.z) * 0.5f);
    }

    /**
     * @brief Get the surface area of the bounding box
     */
    float surface_area() const {
        float dx = max.x - min.x;
        float dy = max.y - min.y;
        float dz = max.z - min.z;
        return 2.0f * (dx * dy + dy * dz + dz * dx);
    }

    /**
     * @brief Get the volume of the bounding box
     */
    float volume() const {
        float dx = max.x - min.x;
        float dy = max.y - min.y;
        float dz = max.z - min.z;
        return dx * dy * dz;
    }

    /**
     * @brief Get the extent along a specific axis (0=x, 1=y, 2=z)
     */
    float extent(int axis) const {
        switch (axis) {
            case 0: return max.x - min.x;
            case 1: return max.y - min.y;
            case 2: return max.z - min.z;
            default: return 0.0f;
        }
    }

    /**
     * @brief Create an invalid bounding box
     */
    static BoundingBox invalid() {
        return BoundingBox();
    }

    /**
     * @brief Check if the bounding box is valid
     */
    bool is_valid() const {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }
};

/**
 * @brief Vertex geometry manager
 *
 * Manages 3D coordinates for vertices in a simplicial complex.
 */
class VertexGeometry {
public:
    /**
     * @brief Constructor
     */
    VertexGeometry();

    /**
     * @brief Set the position of a vertex
     * @param vertex_id Vertex ID
     * @param position 3D position
     */
    void set_position(VertexID vertex_id, const Point3D& position);

    /**
     * @brief Set the position of a vertex
     * @param vertex_id Vertex ID
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
    void set_position(VertexID vertex_id, float x, float y, float z);

    /**
     * @brief Get the position of a vertex
     * @param vertex_id Vertex ID
     * @return Optional position, or std::nullopt if vertex not found
     */
    std::optional<Point3D> get_position(VertexID vertex_id) const;

    /**
     * @brief Check if a vertex has geometry data
     */
    bool has_position(VertexID vertex_id) const;

    /**
     * @brief Remove geometry data for a vertex
     */
    void remove_position(VertexID vertex_id);

    /**
     * @brief Get the bounding box of all vertices
     */
    BoundingBox compute_bounding_box() const;

    /**
     * @brief Get the number of vertices with geometry data
     */
    size_t vertex_count() const { return positions_.size(); }

    /**
     * @brief Clear all geometry data
     */
    void clear();

    /**
     * @brief Get all vertex IDs with geometry data
     */
    std::vector<VertexID> get_vertex_ids() const;

private:
    std::unordered_map<VertexID, Point3D> positions_;
};

} // namespace cebu
