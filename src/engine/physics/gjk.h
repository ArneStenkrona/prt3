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
#include <algorithm>

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

void get_face_normals(glm::vec3 const * polytope,
                      uint8_t const * faces,
                      unsigned int n_faces,
                      glm::vec4 * normals,
                      unsigned int & min_triangle,
                      glm::vec3 const & a_start,
                      glm::vec3 const & a_dest);

void add_if_unique_edge(
    std::pair<uint8_t, uint8_t> * edges,
    unsigned int & n_edges,
    uint8_t const * faces,
    unsigned int a,
    unsigned int b);

struct CollisionResult {
    glm::vec3 normal;
    float penetration_depth;
    bool collided = false;
};

template<typename ShapeA, typename ShapeB>
CollisionResult epa(std::array<glm::vec3, 4> const & simplex,
                    unsigned int n_simplex,
                    ShapeA const & a,
                    ShapeB const & b,
                    glm::vec3 const & a_start,
                    glm::vec3 const & a_dest) {
    //       The memory allocated assumes a worst case
    //       of a complete graph of V={4+max_iter} vertices
    //       This gives us E=V(V-1)/2 edges and
    //       at most F=2E triangluar faces, if each edge
    //       is always shared by two triangles
    static constexpr unsigned int max_iter = 5;
    static constexpr size_t V = 4 + max_iter;
    static constexpr size_t E = V * (V-1) / 2;
    static constexpr size_t F = 2 * E;

    std::array<glm::vec3, V> polytope;
    unsigned int n_polytope = n_simplex;
    polytope[0] = simplex[0];
    polytope[1] = simplex[1];
    polytope[2] = simplex[2];
    polytope[3] = simplex[3];

    std::array<uint8_t, 3 * F> faces;
    faces[0] = 0; faces[ 1] = 1; faces[ 2] = 2;
    faces[3] = 0; faces[ 4] = 3; faces[ 5] = 1;
    faces[6] = 0; faces[ 7] = 2; faces[ 8] = 3;
    faces[9] = 1; faces[10] = 3; faces[11] = 2;
    unsigned int n_faces = 12;

    std::array<glm::vec4, F> normals;
    unsigned int n_normals = n_faces / 3;
    unsigned int min_face = 0;
    get_face_normals(polytope.data(),
                     faces.data(),
                     n_faces,
                     normals.data(),
                     min_face,
                     a_start,
                     a_dest);

    glm::vec3 min_normal;
    float min_distance = std::numeric_limits<float>::max();

    unsigned int iteration = 0;
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

            std::array<std::pair<uint8_t, uint8_t>, E> unique_edges;
            unsigned int n_ue = 0;
            for (unsigned int i = 0; i < n_normals; ++i) {
                if (glm::dot(glm::vec3(normals[i]), support) > 0.0f) {
                    unsigned int f = i * 3;

                    add_if_unique_edge(unique_edges.data(), n_ue, faces.data(), f    , f + 1);

                    add_if_unique_edge(unique_edges.data(), n_ue, faces.data(), f + 1, f + 2);

                    add_if_unique_edge(unique_edges.data(), n_ue, faces.data(), f + 2, f);
                    faces[f + 2] = faces[--n_faces];
                    faces[f + 1] = faces[--n_faces];
                    faces[f] = faces[--n_faces];

                    normals[i] = normals[--n_normals];
                    --i;
                }
            }

            if (n_ue == 0) {
                break;
            }

            std::array<uint8_t, 3 * F> new_faces;
            unsigned int n_new_faces = n_ue * 3;
            for (unsigned int i = 0; i < n_ue; ++i) {
                auto const & edge = unique_edges[i];
                new_faces[3*i] = edge.first;
                new_faces[3*i+1] = edge.second;
                new_faces[3*i+2] = n_polytope;
            }

            polytope[n_polytope] = support;
            ++n_polytope;

            std::array<glm::vec4, F> new_normals;
            unsigned int n_new_normals = n_new_faces / 3;
            unsigned int new_min_face = 0;
            get_face_normals(polytope.data(),
                             new_faces.data(),
                             n_new_faces,
                             new_normals.data(),
                             new_min_face,
                             a_start,
                             a_dest);

            float old_min_distance = std::numeric_limits<float>::max();
            for (unsigned int i = 0; i < n_normals; ++i) {
                if (normals[i].w < old_min_distance) {
                    old_min_distance = normals[i].w;
                    min_face = i;
                }
            }

            if (new_normals[new_min_face].w < old_min_distance) {
                min_face = new_min_face + n_normals;
            }

            for (unsigned int i = 0; i < n_new_faces; ++i) {
                faces[n_faces + i] = new_faces[i];
            }
            n_faces += n_new_faces;
            for (unsigned int i = 0; i < n_new_normals; ++i) {
                normals[n_normals + i] = new_normals[i];
            }
            n_normals += n_new_normals;
        }

        ++iteration;
    }

    if (min_distance == std::numeric_limits<float>::max()) {
        return {};
    }

    glm::vec3 point = polytope[faces[3 * min_face]];
    float depth = glm::abs(glm::dot(min_normal, point));

    CollisionResult collision_res;
    collision_res.normal = -min_normal;
    float eps = 0.001f;
    // collision_res.penetration_depth = min_distance + eps;
    collision_res.penetration_depth = depth + eps;
    collision_res.collided = min_distance != std::numeric_limits<float>::max();

    return collision_res;
}

template<typename ShapeA, typename ShapeB>
CollisionResult gjk(ShapeA const & a,
                    ShapeB const & b,
                    glm::vec3 const & a_start,
                    glm::vec3 const & a_dest) {
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
            return epa(points, n_points, a, b, a_start, a_dest);
        }
    }
}

} // namespace prt3::collision_util

#endif
