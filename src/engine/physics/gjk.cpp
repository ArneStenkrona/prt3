#include "gjk.h"

bool prt3::collision_util::next_simplex_line(
                       std::array<glm::vec3, 4> & points,
                       unsigned int & n_points,
                       glm::vec3 & direction) {
    glm::vec3 a = points[0];
    glm::vec3 b = points[1];

    glm::vec3 ab = b - a;
    glm::vec3 ao =   - a;

    if (glm::dot(ab, ao) > 0.0f) {
        direction = glm::cross(glm::cross(ab, ao), ab);
    } else {
        n_points = 1;
        direction = ao;
    }

    return false;
}

bool prt3::collision_util::next_simplex_triangle(
                           std::array<glm::vec3, 4> & points,
                           unsigned int & n_points,
                           glm::vec3 & direction) {
    glm::vec3 a = points[0];
    glm::vec3 b = points[1];
    glm::vec3 c = points[2];

    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 ao =   - a;

    glm::vec3 abc = glm::cross(ab, ac);

    if (glm::dot(glm::cross(abc, ac), ao) > 0.0f) {
        if (glm::dot(ac, ao) > 0.0f) {
            points[1] = c;
            n_points = 2;
            direction = glm::cross(glm::cross(ac, ao), ac);
        } else {
            n_points = 2;
            return next_simplex_line(points, n_points, direction);
        }
    } else {
        if (glm::dot(glm::cross(ab, abc), ao) > 0.0f) {
            n_points = 2;
            return next_simplex_line(points, n_points, direction);
        } else {
            if (glm::dot(abc, ao) > 0.0f) {
                direction = abc;
            } else {
                points[1] = c;
                points[2] = b;
                direction = -abc;
            }
        }
    }

    return false;
}

bool prt3::collision_util::next_simplex_tetrahedron(
                              std::array<glm::vec3, 4> & points,
                              unsigned int & n_points,
                              glm::vec3 & direction) {
    glm::vec3 a = points[0];
    glm::vec3 b = points[1];
    glm::vec3 c = points[2];
    glm::vec3 d = points[3];

    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 ad = d - a;
    glm::vec3 ao =   - a;

    glm::vec3 abc = glm::cross(ab, ac);
    glm::vec3 acd = glm::cross(ac, ad);
    glm::vec3 adb = glm::cross(ad, ab);

    if (glm::dot(abc, ao) > 0.0f) {
        n_points = 3;
        return next_simplex_triangle(points, n_points, direction);
    }
    if (glm::dot(acd, ao) > 0.0f) {
        points[1] = c;
        points[2] = d;
        n_points = 3;
        return next_simplex_triangle(points, n_points, direction);
    }
    if (glm::dot(adb, ao) > 0.0f) {
        points[1] = d;
        points[2] = b;
        n_points = 3;
        return next_simplex_triangle(points, n_points, direction);
    }

    return true;
}

std::pair<std::vector<glm::vec4>, size_t> prt3::collision_util::get_face_normals(
    std::vector<glm::vec3> const & polytope,
    std::vector<unsigned int> const & faces) {
    std::vector<glm::vec4> normals;
    size_t min_triangle = 0;
    float min_distance = std::numeric_limits<float>::max();

    for (size_t i = 0; i < faces.size(); i += 3) {
        glm::vec3 a = polytope[faces[i]];
        glm::vec3 b = polytope[faces[i + 1]];
        glm::vec3 c = polytope[faces[i + 2]];

        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        float distance = glm::dot(normal, a);

        if (distance < 0) {
            normal *= -1.0f;
            distance *= -1.0f;
        }

        normals.emplace_back(normal, distance);

        if (distance < min_distance) {
            min_triangle = i / 3;
            min_distance = distance;
        }
    }

    return { normals, min_triangle };
}

void prt3::collision_util::add_if_unique_edge(
    std::vector<std::pair<unsigned int, unsigned int>> & edges,
    std::vector<unsigned int> const & faces,
    size_t a,
    size_t b) {
    auto reverse = std::find(edges.begin(), edges.end(),
                             std::make_pair(faces[b], faces[a]));

    if (reverse != edges.end()) {
        edges.erase(reverse);
    } else {
        edges.emplace_back(faces[a], faces[b]);
    }
}
