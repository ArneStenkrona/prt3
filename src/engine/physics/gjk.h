#ifndef PRT3_GJK_H
#define PRT3_GJK_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>
#include <vector>
#include <cassert>
#include <utility>

// Thanks https://blog.winter.dev/2020/gjk-algorithm/ for
// wonderful implementation reference of GJK/EPA

namespace prt3::collision_util {

bool next_simplex_line(std::array<glm::vec3, 4> & points,
                       unsigned int & n_points,
                       glm::vec3 & direction);

bool next_simplex_triangle(std::array<glm::vec3, 4> & points,
                           unsigned int & n_points,
                           glm::vec3 & direction);

bool next_simplex_tetrahedron(std::array<glm::vec3, 4> & points,
                              unsigned int & n_points,
                              glm::vec3 & direction);

std::pair<std::vector<glm::vec4>, size_t> get_face_normals(
    std::vector<glm::vec3> const & polytope,
    std::vector<unsigned int> const & faces);

void add_if_unique_edge(
    std::vector<std::pair<unsigned int, unsigned int>> & edges,
    std::vector<unsigned int> const & faces,
    size_t a,
    size_t b);

struct CollisionResult {
    glm::vec3 normal;
    float penetration_depth;
    bool collided = false;
};

template<typename ShapeA, typename ShapeB>
CollisionResult epa(std::array<glm::vec3, 4> const & simplex,
                    size_t n_simplex,
                    ShapeA const & a,
                    ShapeB const & b) {
    std::vector<glm::vec3> polytope(simplex.begin(), simplex.begin() + n_simplex);
    std::vector<unsigned int> faces = {
        0, 1, 2,
        0, 3, 1,
        0, 2, 3,
        1, 3, 2
    };

    auto [normals, min_face] = get_face_normals(polytope, faces);

    glm::vec3 min_normal;
    float min_distance = std::numeric_limits<float>::max();

    unsigned int iteration = 0;
    static constexpr unsigned int max_iter = 32;
    while (min_distance == std::numeric_limits<float>::max()) {
        min_normal = glm::vec3(normals[min_face]);
        min_distance = normals[min_face].w;

        if (iteration >= max_iter) {
            break;
        }

        glm::vec3 support = calculate_support(a, b, min_normal);
        float signed_distance = glm::dot(min_normal, support);

        if (glm::abs(signed_distance - min_distance) > 0.001f) {
            min_distance = std::numeric_limits<float>::max();

            std::vector<std::pair<unsigned int, unsigned int>> unique_edges;

            for (size_t i = 0; i < normals.size(); ++i) {
                if (glm::dot(glm::vec3(normals[i]), support) > 0.0f) {
                    size_t f = i * 3;

                    add_if_unique_edge(unique_edges, faces, f    , f + 1);
                    add_if_unique_edge(unique_edges, faces, f + 1, f + 2);
                    add_if_unique_edge(unique_edges, faces, f + 2, f);

                    faces[f + 2] = faces.back();
                    faces.pop_back();
                    faces[f + 1] = faces.back();
                    faces.pop_back();
                    faces[f] = faces.back();
                    faces.pop_back();

                    normals[i] = normals.back();
                    normals.pop_back();

                    --i;
                }
            }

            if (unique_edges.empty()) {
                break;
            }

            std::vector<unsigned int> new_faces;
            for (auto [edge_index1, edge_index2] : unique_edges) {
                new_faces.push_back(edge_index1);
                new_faces.push_back(edge_index2);
                new_faces.push_back(polytope.size());
            }

            polytope.push_back(support);

            auto [new_normals, new_min_face] = get_face_normals(polytope, new_faces);

            float old_min_distance = std::numeric_limits<float>::max();
            for (size_t i = 0; i < normals.size(); ++i) {
                if (normals[i].w < old_min_distance) {
                    old_min_distance = normals[i].w;
                    min_face = i;
                }
            }

            if (new_normals[new_min_face].w < old_min_distance) {
                min_face = new_min_face + normals.size();
            }

            faces.insert(faces.end(), new_faces.begin(), new_faces.end());
            normals.insert(normals.end(), new_normals.begin(), new_normals.end());
        }

        ++iteration;
    }

    if (min_distance == std::numeric_limits<float>::max()) {
        return {};
    }

    CollisionResult collision_res;
    collision_res.normal = min_normal;
    float eps = 0.001f;
    collision_res.penetration_depth = min_distance + eps;
    collision_res.collided = min_distance != std::numeric_limits<float>::max();

    return collision_res;
}

template<typename ShapeA, typename ShapeB>
CollisionResult gjk(ShapeA const & a,
                    ShapeB const & b) {
    glm::vec3 support = calculate_support(a, b, glm::vec3{1.0f, 0.0f, 0.0f});

    unsigned int n_points = 1;
    std::array<glm::vec3, 4> points;
    points[0] = support;

    glm::vec3 direction = -support;

    while (true) {
        support = calculate_support(a, b, direction);
        if (glm::dot(support, direction) <= 0.0f) {
            CollisionResult res;
            res.collided = false;
            return res;
        }

        points[3] = points[2];
        points[2] = points[1];
        points[1] = points[0];
        points[0] = support;
        n_points = n_points == 4 ? 4 : n_points + 1;

        bool res_simplex = false;

        switch (n_points) {
            case 2: {
                res_simplex = next_simplex_line(points,
                                                n_points,
                                                direction);
                break;
            }
            case 3: {
                res_simplex = next_simplex_triangle(points,
                                                    n_points,
                                                    direction);
                break;
            }
            case 4: {
                res_simplex = next_simplex_tetrahedron(points,
                                                       n_points,
                                                       direction);
                break;
            }
            default: {
                assert(false && "Invalid simplex");
            }
        }

        if (res_simplex) {
            return epa(points, n_points, a, b);
        }
    }
}

} // namespace prt3::collision_util

#endif
