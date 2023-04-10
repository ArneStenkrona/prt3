#include "navigation_system.h"

#include "src/util/log.h"
#include "src/util/geometry_util.h"
#include "src/util/mesh_util.h"
#include "src/engine/scene/scene.h"

#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <queue>
#include <set>
#include <unordered_map>
#include <tuple>

using namespace prt3;

std::vector<std::vector<glm::vec3> > temp_contours;

struct Span {
    uint32_t low;
    uint32_t high;
    uint32_t x;
    uint32_t z;
    int32_t next_index;
    int32_t region_index;
    union {
        struct {
            int32_t north;
            int32_t east;
            int32_t south;
            int32_t west;
        };
        int32_t neighbours[4];
    };
    uint32_t distance_to_region_center;
};

union NeighbourDiff {
    struct {
        bool north;
        bool east;
        bool south;
        bool west;
    };
    bool dir[4];
};

struct VertexData {
    glm::vec3 position;
    int32_t external_region_index;
    int32_t internal_region_index;
    uint32_t raw_index;
    uint32_t dir;
};

struct TriangleData {
    uint32_t index;
    bool is_triangle_center_index;
};

inline int32_t connection(
    Span const & span,
    int32_t index,
    std::vector<SubVec> const & columns,
    std::vector<Span> const & spans,
    uint32_t required_height
) {
    if (index < 0) return -1;

    SubVec const & column = columns[index];

    for (uint32_t i = column.start_index;
        i < column.start_index + column.num_indices;
        ++i) {
        Span const & other = spans[i];

        bool slope_ok = abs(int32_t(span.low) - int32_t(other.low)) <= 1;

        int32_t max_low = span.low > other.low ? span.low : other.low;
        int32_t min_high = span.high < other.high ? span.high : other.high;
        bool height_ok  = min_high - max_low > int32_t(required_height);

        if (slope_ok && height_ok) {
            return i;
        }
    }
    return -1;
}

uint32_t calc_corner_height_index(
    uint32_t span_index,
    std::vector<Span> const & spans,
    unsigned int neighbour_dir
) {
    Span const & span = spans[span_index];
    auto max_floor = span.low;

    int32_t diag_span_index = -1;

    unsigned int dir_offset = (neighbour_dir + 1 ) % 4;

    int32_t axis_span_index = span.neighbours[neighbour_dir];
    if (axis_span_index != -1) {
        Span const & axis_span = spans[axis_span_index];
        max_floor = glm::max(max_floor, axis_span.low);
        diag_span_index = axis_span.neighbours[dir_offset];
    }

    axis_span_index = span.neighbours[dir_offset];
    if (axis_span_index != -1) {
        Span const & axis_span = spans[axis_span_index];
        max_floor = glm::max(max_floor, axis_span.low);
        if (diag_span_index == -1) {
            diag_span_index = axis_span.neighbours[neighbour_dir];
        }
    }

    if (diag_span_index != -1) {
        Span const & diag_span = spans[diag_span_index];
        max_floor = glm::max(max_floor, diag_span.low);
    }

    return max_floor;
}

void expand_regions(
    std::set<uint32_t> const & span_indices,
    std::vector<Span> & spans,
    int max_iter
) {
    if (span_indices.empty()) return;

    int iter = 0;

    while (max_iter == -1 || iter < max_iter) {
        unsigned int skipped = 0;

        for (uint32_t index : span_indices) {
            Span & span = spans[index];

            if (span.region_index != -1) {
                ++skipped;
                continue;
            }

            int32_t region_index = -1;
            uint32_t distance_to_region_center =
                std::numeric_limits<uint32_t>::max();

            for (unsigned dir = 0; dir < 4; ++dir) {
                auto ni = span.neighbours[dir];
                if (ni == -1) continue;
                Span & neighbour = spans[ni];
                if (neighbour.region_index == -1) continue;
                if (neighbour.distance_to_region_center + 1 <
                    distance_to_region_center) {

                    region_index = neighbour.region_index;
                    distance_to_region_center =
                        neighbour.distance_to_region_center + 1;
                }
            }

            if (region_index != -1) {
                span.region_index = region_index;
                span.distance_to_region_center = distance_to_region_center;
            } else {
                ++skipped;
            }
        }

        if (skipped == span_indices.size()) {
            break;
        }

        ++iter;
    }
}

void build_raw_contour(
    glm::vec3 origin,
    float granularity,
    uint32_t span_index,
    std::vector<Span> const & spans,
    unsigned int start_dir,
    std::vector<NeighbourDiff> & neighbour_diffs,
    std::vector<VertexData> & vertices,
    bool & only_connected_to_null
) {
    uint32_t raw_index = 0;

    unsigned dir = start_dir;

    Span const & span = spans[span_index];
    uint32_t x = span.x;
    uint32_t z = span.z;

    uint32_t curr_i = span_index;

    while (true) {
        if (neighbour_diffs[curr_i].dir[dir]) {
            float pos_x = origin.x + granularity * x;
            float pos_y = origin.y + granularity *
                          calc_corner_height_index(curr_i, spans, dir);
            float pos_z = origin.z + granularity * z + granularity;

            // TODO: double-check this
            switch (dir) {
                case 1: pos_x += granularity; break;
                case 2: pos_x += granularity; pos_z -= granularity; break;
                case 3: pos_z -= granularity; break;
            }

            int32_t region_index_dir = -1;
            int32_t neighbour_index = spans[curr_i].neighbours[dir];
            if (neighbour_index) {
                region_index_dir = spans[neighbour_index].region_index;
            }

            if (region_index_dir != -1) {
                only_connected_to_null = false;
            }

            VertexData vertex;
            vertex.position = glm::vec3{pos_x, pos_y, pos_z};
            vertex.external_region_index = region_index_dir;
            vertex.internal_region_index = spans[curr_i].region_index;
            vertex.dir = dir;

            vertex.raw_index = raw_index;
            if (vertices.empty() ||
                vertices.back().dir != dir ||
                vertices.back().external_region_index != region_index_dir ||
                vertices.back().internal_region_index != spans[curr_i].region_index) {
                vertices.push_back(vertex);
                ++raw_index;
            }

            neighbour_diffs[curr_i].dir[dir] = false;
            dir = (dir + 1) % 4;
        } else {
            curr_i = spans[curr_i].neighbours[dir];

            switch (dir) {
                case 0: ++z; break;
                case 1: ++x; break;
                case 2: --z; break;
                case 3: --x; break;
            }

            dir = (dir + 3) % 4;
        }

        if (curr_i == span_index && dir == start_dir) {
            break;
        }
    }
}

inline float point_dist_to_segment(glm::vec3 p, glm::vec3 l1, glm::vec3 l2) {
    float line_dist = glm::distance2(l1, l2);
    if (line_dist == 0.0f) return glm::distance(p, l1);
    float t = glm::clamp(glm::dot(p - l1, l2 - l1) / line_dist, 0.0f, 1.0f);
    return glm::distance(p, t * (l2 - l1));
}

void reinsert_null_region_vertices(
    float max_edge_deviation,
    std::vector<VertexData> const & raw_vertices,
    std::vector<VertexData> & simplified_vertices
) {
    uint32_t vertex_a = 0;
    while (vertex_a < simplified_vertices.size()) {
        uint32_t vertex_b = (vertex_a + 1) % simplified_vertices.size();
        uint32_t raw_index_a = simplified_vertices[vertex_a].raw_index;
        uint32_t raw_index_b = simplified_vertices[vertex_b].raw_index;
        uint32_t vertex_to_test = (raw_index_a + 1) % raw_vertices.size();

        float max_deviation = 0.0f;
        int32_t vertex_to_insert = -1;

        if (raw_vertices[vertex_to_test].external_region_index == -1) {
            while (vertex_to_test != raw_index_b) {
                float deviation = point_dist_to_segment(
                    raw_vertices[vertex_to_test].position,
                    simplified_vertices[vertex_a].position,
                    simplified_vertices[vertex_b].position
                );

                if (deviation > max_deviation) {
                    max_deviation = deviation;
                    vertex_to_insert = vertex_to_test;
                }

                vertex_to_test = (vertex_to_test + 1) % raw_vertices.size();
            }
        }

        if (vertex_to_insert != -1 && max_deviation > max_edge_deviation) {
            simplified_vertices.insert(
                simplified_vertices.begin() + vertex_a + 1,
                raw_vertices[vertex_to_insert]
            );
        } else {
            ++vertex_a;
        }
    }
}

void check_null_region_max_edge(
    float granularity,
    float max_edge_length,
    std::vector<VertexData> const & raw_vertices,
    std::vector<VertexData> & simplified_vertices
) {
    if (max_edge_length <= granularity) {
        return;
    }

    uint32_t vertex_a = 0;
    while (vertex_a < simplified_vertices.size()) {
        uint32_t vertex_b = (vertex_a + 1) % simplified_vertices.size();
        uint32_t raw_index_a = simplified_vertices[vertex_a].raw_index;
        uint32_t raw_index_b = simplified_vertices[vertex_b].raw_index;

        int32_t new_vertex = -1;
        uint32_t vertex_to_test = (raw_index_a + 1) % raw_vertices.size();

        if (raw_vertices[vertex_to_test].external_region_index == -1) {
            float dist_x = raw_vertices[raw_index_b].position.x -
                           raw_vertices[raw_index_a].position.x;
            float dist_z = raw_vertices[raw_index_b].position.z -
                           raw_vertices[raw_index_a].position.z;

            if (dist_x * dist_x + dist_z * dist_z >
                max_edge_length * max_edge_length) {
                uint32_t index_distance = raw_index_b < raw_index_a ?
                    raw_index_b + (raw_vertices.size() - raw_index_a) :
                    raw_index_b - raw_index_a;

                uint32_t cand = (raw_index_a + index_distance / 2) %
                                raw_vertices.size();
                if (cand != raw_index_a) {
                    new_vertex = cand;
                }
            }
        }

        if (new_vertex != -1) {
            simplified_vertices.insert(
                simplified_vertices.begin() + vertex_a + 1,
                raw_vertices[new_vertex]
            );
        } else {
            ++vertex_a;
        }
    }
}

void remove_duplicate_vertices(std::vector<VertexData> & vertices) {
    for (size_t i = 0; i < vertices.size();) {
        size_t next_i = (i + 1) % vertices.size();

        static constexpr float eps = 0.001f;
        float dist2 = glm::distance2(
            vertices[i].position,
            vertices[next_i].position
        );
        if (dist2 < eps) {
            vertices.erase(vertices.begin() + i);
        } else {
            ++i;
        }
    }
}

void simplify_contour(
    bool only_connected_to_null,
    float granularity,
    float max_edge_deviation,
    float max_edge_length,
    std::vector<VertexData> const & raw_vertices,
    std::vector<VertexData> & simplified_vertices
) {
    if (only_connected_to_null) {
        VertexData bottom_left = raw_vertices[0];

        VertexData top_right = raw_vertices[0];

        for (VertexData const & vertex : raw_vertices) {
            glm::vec3 pos = vertex.position;

            static constexpr float eps = 0.001f;
            if (pos.x < bottom_left.position.x ||
                (glm::distance(pos.x, bottom_left.position.x) <= eps &&
                 pos.z < bottom_left.position.z)) {
                bottom_left = vertex;
            }

            if (pos.x > top_right.position.x ||
                (glm::distance(pos.x, top_right.position.x) <= eps &&
                 pos.z > top_right.position.z)) {
                top_right = vertex;
            }
        }

        simplified_vertices.push_back(bottom_left);
        simplified_vertices.push_back(top_right);
    } else {
        for (unsigned int i = 0; i < raw_vertices.size(); ++i) {
            int32_t ext_region_1 = raw_vertices[i].external_region_index;
            unsigned int prev_i = (raw_vertices.size() - 1 + i) % raw_vertices.size();
            int32_t ext_region_2 = raw_vertices[prev_i].external_region_index;

            if (ext_region_1 != ext_region_2) {
                simplified_vertices.push_back(raw_vertices[i]);
            }
        }
    }

    if (raw_vertices.size() == 0 || simplified_vertices.size() == 0) {
        return;
    }

    reinsert_null_region_vertices(max_edge_deviation, raw_vertices, simplified_vertices);
    check_null_region_max_edge(granularity, max_edge_length, raw_vertices, simplified_vertices);
    remove_duplicate_vertices(simplified_vertices);
}

inline float double_signed_area(
    glm::vec3 const & a,
    glm::vec3 const & b,
    glm::vec3 const & c
)  {
    return -((b.z - a.z) * (c.x - a.x) - (c.z - a.z) * (b.x - a.x));
}

inline bool is_left(
    glm::vec3 const & vertex,
    glm::vec3 const & a,
    glm::vec3 const & b
) {
    return double_signed_area(a, vertex, b) < 0.0f;
}

inline bool is_left_or_collinear(
    glm::vec3 const & vertex,
    glm::vec3 const & a,
    glm::vec3 const & b
) {
    return double_signed_area(a, vertex, b) <= 0.0f;
}

inline bool is_right(
    glm::vec3 const & vertex,
    glm::vec3 const & a,
    glm::vec3 const & b
) {
    return double_signed_area(a, vertex, b) > 0.0f;
}

inline bool is_right_or_collinear(
    glm::vec3 const & vertex,
    glm::vec3 const & a,
    glm::vec3 const & b
) {
    return double_signed_area(a, vertex, b) >= 0.0f;
}

bool is_in_internal_angle(
    uint32_t index_a,
    uint32_t index_b,
    glm::vec3 const * vertices,
    std::vector<TriangleData> & indices
) {
    glm::vec3 const & a = vertices[indices[index_a].index];
    glm::vec3 const & b = vertices[indices[index_b].index];

    uint32_t index_a_minus = (indices.size() - 1 + index_a) % indices.size();
    uint32_t index_a_plus = (index_a + 1) % indices.size();

    glm::vec3 const & a_minus = vertices[indices[index_a_minus].index];
    glm::vec3 const & a_plus = vertices[indices[index_a_plus].index];

    if (is_left_or_collinear(a, a_minus, a_plus)) {
        return is_left(b, a, a_minus) && is_right(b, a, a_plus);
    }

    return !(is_left_or_collinear(b, a, a_plus) &&
             is_right_or_collinear(b, a, a_minus));
}

inline bool segment_intersect_2d(
    glm::vec3 const & p1,
    glm::vec3 const & p2,
    glm::vec3 const & q1,
    glm::vec3 const & q2
) {
    return (((q1.x-p1.x)*(p2.z-p1.z) - (q1.z-p1.z)*(p2.x-p1.x))
            * ((q2.x-p1.x)*(p2.z-p1.z) - (q2.z-p1.z)*(p2.x-p1.x)) < 0)
            &&
           (((p1.x-q1.x)*(q2.z-q1.z) - (p1.z-q1.z)*(q2.x-q1.x))
            * ((p2.x-q1.x)*(q2.z-q1.z) - (p2.z-q1.z)*(q2.x-q1.x)) < 0);
}

bool valid_edge_intersection(
    uint32_t index_a,
    uint32_t index_b,
    glm::vec3 const * vertices,
    std::vector<TriangleData> & indices
) {
    for (uint32_t i = 0; i < indices.size(); ++i) {
        uint32_t poly_begin = i;
        uint32_t poly_end = (i + 1) % indices.size();

        glm::vec3 const & a = vertices[indices[index_a].index];
        glm::vec3 const & b = vertices[indices[index_b].index];
        glm::vec3 const & pb = vertices[indices[poly_begin].index];
        glm::vec3 const & pe = vertices[indices[poly_end].index];

        if (poly_begin != index_a && poly_begin != index_b &&
            poly_end != index_a && poly_end != index_b) {
            if ((pb.x == a.x && pb.z == a.z) ||
                (pb.x == b.x && pb.z == b.z) ||
                (pe.x == a.x && pe.z == a.z) ||
                (pe.x == b.x && pe.z == b.z)) {
                continue;
            }

            if (segment_intersect_2d(a, b, pb, pe)) {
                return false;
            }
        }
    }

    return true;
}

bool is_valid_partition(
    uint32_t index_a,
    uint32_t index_b,
    glm::vec3 const * vertices,
    std::vector<TriangleData> & indices
) {
    bool internal = is_in_internal_angle(index_a, index_b, vertices, indices);
    bool valid = valid_edge_intersection(index_a, index_b, vertices, indices);
    return internal && valid;
}

bool triangulate(
    glm::vec3 const * vertices,
    std::vector<TriangleData> & indices,
    std::vector<uint32_t> & triangles
) {
    for (uint32_t i = 0; i < indices.size(); ++i) {
        uint32_t index_plus_2 = (i + 2) % indices.size();

        if (is_valid_partition(i, index_plus_2, vertices, indices)) {
            uint32_t index_plus_1 = (i + 1) % indices.size();
            indices[index_plus_1].is_triangle_center_index = true;
        }
    }

    while (indices.size() > 3) {
        float min_length_sq = std::numeric_limits<float>::max();
        uint32_t min_length_sq_index = -1;

        for (uint32_t i = 0; i < indices.size(); ++i) {
            uint32_t index_plus_1 = (i + 1) % indices.size();


            if (indices[index_plus_1].is_triangle_center_index) {
                uint32_t index_plus_2 = (i + 2) % indices.size();

                float delta_x = vertices[indices[index_plus_2].index].x -
                    vertices[indices[i].index].x;
                float delta_z = vertices[indices[index_plus_2].index].z -
                    vertices[indices[i].index].z;

                float length_sq = delta_x * delta_x + delta_z * delta_z;

                if (length_sq < min_length_sq) {
                    min_length_sq = length_sq;
                    min_length_sq_index = i;
                }
            }
        }

        if (min_length_sq == std::numeric_limits<float>::max()) {
            return false;
        }

        uint32_t index = min_length_sq_index;
        uint32_t index_plus_1 = (index + 1) % indices.size();
        uint32_t index_plus_2 = (index + 2) % indices.size();

        triangles.push_back(indices[index].index);
        triangles.push_back(indices[index_plus_1].index);
        triangles.push_back(indices[index_plus_2].index);

        indices.erase(indices.begin() + index_plus_1);

        if (index_plus_1 == 0 || index_plus_1 >= indices.size()) {
            index = indices.size() - 1;
            index_plus_1 = 0;
        }

        uint32_t index_minus = (indices.size() - 1 + index) % indices.size();

        indices[index].is_triangle_center_index =
            is_valid_partition(index_minus, index_plus_1, vertices, indices);

        index_plus_2 = (index_plus_1 + 1) % indices.size();

        indices[index_plus_1].is_triangle_center_index =
            is_valid_partition(index, index_plus_2, vertices, indices);
    }

    triangles.push_back(indices[0].index);
    triangles.push_back(indices[1].index);
    triangles.push_back(indices[2].index);

    return true;
}

// Many thanks to https://javid.nl/atlas.html
// and https://www.stefanolazzaroni.com/navigation-mesh-generation
NavMeshID NavigationSystem::generate_nav_mesh(
    NodeID node_id,
    Scene const & scene,
    CollisionLayer layer,
    float granularity,
    float max_edge_deviation,
    float max_edge_length,
    float min_width,
    float min_height
) {
    if (m_nav_mesh_ids.find(node_id) != m_nav_mesh_ids.end()) {
        remove_nav_mesh(m_nav_mesh_ids.at(node_id));
    }

    if (layer == 0) return NO_NAV_MESH;

    PhysicsSystem const & sys = scene.physics_system();

    std::vector<glm::vec3> geometry;

    for (auto const & pair : sys.mesh_colliders()) {
        MeshCollider const & col = pair.second;
        if ((col.get_layer() & layer) == 0) continue;
        geometry.insert(
            geometry.end(),
            col.triangle_cache().begin(),
            col.triangle_cache().end()
        );
    }

    if (geometry.empty()) return NO_NAV_MESH;

    assert(geometry.size() % 3 == 0);

    DynamicAABBTree aabb_tree;

    AABB aabb;
    aabb.lower_bound = geometry[0];
    aabb.upper_bound = geometry[0];

    // generate aabb of all geometry
    for (size_t i = 1; i < geometry.size(); ++i) {
        aabb.lower_bound = glm::min(aabb.lower_bound, geometry[i]);
        aabb.upper_bound = glm::max(aabb.upper_bound, geometry[i]);
    }

    float margin = glm::max(1.0f, granularity);
    float small_offset = 0.001f;
    aabb.lower_bound -= 0.5f * margin + small_offset;
    aabb.upper_bound += 0.5f * margin + small_offset;

    // insert each triangle into the aabb_tree
    float neg_inf = -std::numeric_limits<float>::infinity();
    float pos_inf = std::numeric_limits<float>::infinity();
    std::vector<AABB> aabbs;
    size_t n_tris = geometry.size() / 3;
    aabbs.resize(n_tris, {glm::vec3{pos_inf}, glm::vec3{neg_inf}});

    size_t tri_index = 0;
    for (size_t i = 0; i < geometry.size(); i += 3) {
        glm::vec3 & lb = aabbs[tri_index].lower_bound;
        glm::vec3 & ub = aabbs[tri_index].upper_bound;

        lb = glm::min(lb, geometry[i]);
        lb = glm::min(lb, geometry[i+1]);
        lb = glm::min(lb, geometry[i+2]);

        ub = glm::max(ub, geometry[i]);
        ub = glm::max(ub, geometry[i+1]);
        ub = glm::max(ub, geometry[i+2]);

        ++tri_index;
    }
    // TODO: Tag and layer are not needed, refactor DynamicAABBTree to allow
    //       just a simple id to be used
    std::vector<ColliderTag> tags;
    tags.resize(n_tris);
    std::vector<CollisionLayer> layers;
    layers.resize(n_tris, ~0);

    if (n_tris >= size_t(std::numeric_limits<ColliderID>::max())) {
        PRT3ERROR("%s\n", "ERROR: Too many triangles in navmesh.");
    }

    for (size_t i = 0; i < n_tris; ++i) {
        tags[i].id = i;
        // tags[i].shape = don't care;
        // tags[i].type = don't care;
    }

    aabb_tree.insert(
        tags.data(),
        layers.data(),
        aabbs.data(),
        n_tris
    );

    /* Voxelize */
    glm::ivec3 diff =
        glm::ivec3{glm::ceil(aabb.upper_bound / granularity)} -
        glm::ivec3{glm::floor(aabb.lower_bound / granularity)};

    glm::uvec3 dim = glm::uvec3{
        static_cast<unsigned int>(diff.x),
        static_cast<unsigned int>(diff.y),
        static_cast<unsigned int>(diff.z)
    };

    std::vector<bool> voxels;
    voxels.resize(dim.x * dim.y * dim.z);

    glm::vec3 origin = aabb.lower_bound;

    constexpr ColliderTag tag = {
        static_cast<ColliderID>(-1),
        ColliderShape::none,
        ColliderType::collider
    };

    uint32_t iv = 0;
    for (uint32_t ix = 0; ix < dim.x; ++ix) {
        for (uint32_t iz = 0; iz < dim.z; ++iz) {
            for (uint32_t iy = 0; iy < dim.y; ++iy) {
                AABB voxel_aabb;
                voxel_aabb.lower_bound =
                    origin + glm::vec3{ix, iy, iz} * granularity;
                voxel_aabb.upper_bound =
                    voxel_aabb.lower_bound + glm::vec3{granularity};

                tags.resize(0);
                aabb_tree.query(
                    tag,
                    ~0,
                    voxel_aabb,
                    tags
                );

                for (ColliderTag const & tag : tags) {
                    if (triangle_box_overlap(
                        voxel_aabb.lower_bound + glm::vec3{granularity / 2.0f},
                        glm::vec3{granularity / 2.0f},
                        geometry[3 * tag.id],
                        geometry[3 * tag.id + 1],
                        geometry[3 * tag.id + 2]
                    )) {
                        voxels[iv] = true;
                        break;
                    }
                }
                ++iv;
            }
        }
    }

    /* Generate height fields */
    std::vector<Span> spans;
    std::vector<SubVec> columns;
    columns.resize(dim.x * dim.z);

    iv = 0;
    for (uint32_t ix = 0; ix < dim.x; ++ix) {
        for (uint32_t iz = 0; iz < dim.z; ++iz) {
            uint32_t span_start = spans.size();

            bool prev = false;
            bool ground = false;
            uint32_t start = 0;
            for (uint32_t iy = 0; iy < dim.y; ++iy) {
                bool curr = voxels[iv];
                if (prev && !curr) {
                    start = iy;
                }

                if (ground && !prev && (curr || (iy + 1) == dim.y)) {
                    spans.emplace_back(Span{
                        start, // low
                        iy, // high
                        ix, // x
                        iz, // z
                        -1, // next_index
                        -1, // region_index
                        { {-1, -1, -1, -1} }, // neighbours
                        0 // distance_to_region_center
                    });
                }

                ground |= curr;
                prev = curr;
                ++iv;
            }

            SubVec & col = columns[ix * dim.z + iz];
            col.start_index = span_start;
            col.num_indices = spans.size() - span_start;

            for (uint32_t i = span_start; i + 1 < spans.size(); ++i) {
                spans[i].next_index = i + 1;
            }
        }
    }

    /* Generate distance field */
    uint32_t req_height =  static_cast<uint32_t>(ceil(min_height / granularity));
    uint32_t req_width =  static_cast<uint32_t>(ceil(min_width / granularity));

    for (unsigned int ix = 0; ix < dim.x; ++ix) {
        for (unsigned int iz = 0; iz < dim.z; ++iz) {
            SubVec & col = columns[ix * dim.z + iz];

            // north, east, south, west
            int32_t ni = iz + 1 < dim.z ? (ix * dim.z + (iz + 1)) : -1;
            int32_t ei = ix + 1 < dim.x ? ((ix + 1) * dim.z + iz) : -1;
            int32_t si = iz > 0 ? (ix * dim.z + (iz - 1)) : -1;
            int32_t wi = ix > 0 ? ((ix - 1) * dim.z + iz) : -1;

            for (uint32_t is = col.start_index;
                is < col.start_index + col.num_indices;
                ++is) {
                Span & span = spans[is];
                span.north = connection(span, ni, columns, spans, req_height);
                span.east = connection(span, ei, columns, spans, req_height);
                span.south = connection(span, si, columns, spans, req_height);
                span.west = connection(span, wi, columns, spans, req_height);
            }
        }
    }

    std::queue<uint32_t> queue;

    enum Status : uint8_t {
        OPEN = 0,
        IN_PROGRESS = 1,
        CLOSED = 2
    };

    std::vector<Status> status;
    status.resize(spans.size());

    std::vector<uint32_t> distances;
    distances.resize(spans.size());

    for (uint32_t i = 0; i < spans.size(); ++i) {
        Span const & span = spans[i];
        bool border = false;
        border |= span.north == -1;
        border |= span.east == -1;
        border |= span.south == -1;
        border |= span.west == -1;

        if (border) {
            queue.push(i);
            status[i] = IN_PROGRESS;
        }
    }

    uint32_t max_dist = 0;
    uint32_t min_dist = std::numeric_limits<uint32_t>::max();
    while (!queue.empty()) {
        uint32_t index = queue.front();
        Span const & span = spans[index];

        for (unsigned int dir = 0; dir < 4; ++dir) {
            if (span.neighbours[dir] != -1) {
                int32_t other_ind = span.neighbours[dir];
                switch (status[other_ind]) {
                    case OPEN: {
                        queue.push(other_ind);
                        status[other_ind] = IN_PROGRESS;
                        distances[other_ind] = distances[index] + 1;
                        break;
                    }
                    case IN_PROGRESS: {
                        if (distances[index] + 1 < distances[other_ind]) {
                            distances[other_ind] = distances[index] + 1;
                        }
                        break;
                    }
                    case CLOSED: {
                        if (distances[index] + 1 < distances[other_ind]) {
                            distances[other_ind] = distances[index] + 1;
                            queue.push(other_ind);
                            status[other_ind] = IN_PROGRESS;
                        }
                        break;
                    }
                }
            }
        }
        if (distances[index] > max_dist) max_dist = distances[index];
        if (distances[index] < min_dist) min_dist = distances[index];

        status[index] = CLOSED;
        queue.pop();
    }

    /* Create regions */
    std::set<uint32_t> flooded_spans;

    std::queue<uint32_t> span_queue;

    int expand_iter = 4 + req_width * 2;

    int32_t region_index = 0;
    for (auto current_dist = max_dist;
        current_dist > min_dist;) {
        for (size_t i = 0; i < spans.size(); ++i) {
            Span & span = spans[i];
            if (distances[i] >= current_dist && span.region_index == -1) {
                flooded_spans.insert(i);
            }
        }

        if (region_index > 0) {
            expand_regions(flooded_spans, spans, expand_iter);
        } else {
            expand_regions(flooded_spans, spans, -1);
        }

        for (auto it = flooded_spans.begin();
             it != flooded_spans.end();
        ) {
            if (spans[*it].region_index != -1) {
                ++it;
                continue;
            }

            auto fill_to = glm::max(
                uint32_t(current_dist - 1),
                req_width
            );

            unsigned region_size = 0;

            span_queue.push(*it);
            while (!span_queue.empty()) {
                Span & span = spans[span_queue.front()];

                bool is_on_border = false;

                for (unsigned dir = 0; dir < 4; ++dir) {
                    auto ni = span.neighbours[dir];
                    if (ni < 0) continue;
                    Span & neighbour = spans[ni];

                    if (neighbour.region_index != -1 &&
                        neighbour.region_index != region_index) {
                        is_on_border = true;
                        break;
                    }

                    unsigned diag_dir = (dir + 1) % 4;
                    auto dni = neighbour.neighbours[diag_dir];
                    if (dni < 0) continue;
                    Span & diag_neighbour = spans[dni];

                    if (diag_neighbour.region_index != -1 &&
                        diag_neighbour.region_index != region_index) {
                        is_on_border = true;
                        break;
                    }
                }

                if (is_on_border) {
                    span.region_index = -1;
                    span_queue.pop();
                    continue;
                }

                ++region_size;

                for (unsigned dir = 0; dir < 4; ++dir) {
                    auto ni = span.neighbours[dir];
                    if (ni < 0) continue;
                    Span & neighbour = spans[ni];

                    if (distances[ni] >= fill_to &&
                        neighbour.region_index == -1) {
                        neighbour.region_index = region_index;
                        neighbour.distance_to_region_center = 0;
                        span_queue.push(ni);
                    }
                }

                span_queue.pop();
            }

            if (region_size > 0) {
                ++region_index;
                flooded_spans.erase(it++);
            } else {
                ++it;
            }
        }

        current_dist = glm::max(
            current_dist >= 1u ?
                static_cast<uint32_t>(current_dist - 1u) : uint32_t{0},
            min_dist
        );
    }

    flooded_spans.clear();
    for (uint32_t i = 0; i < spans.size(); ++i) {
        Span const & span = spans[i];

        if (distances[i] > req_width && span.region_index == -1) {
            flooded_spans.insert(i);
        }
    }

    if (req_width > 0) {
        expand_regions(flooded_spans, spans, expand_iter * 8);
    } else {
        expand_regions(flooded_spans, spans, -1);
    }

    /* Contour */
    std::vector<NeighbourDiff> neighbour_diffs;
    neighbour_diffs.resize(spans.size());

    for (unsigned i = 0; i < spans.size(); ++ i) {
        Span const & span = spans[i];
        for (unsigned dir = 0; dir < 4; ++dir) {
            int32_t ni = span.neighbours[dir];
            if (ni == -1 || spans[ni].region_index != span.region_index) {
                neighbour_diffs[i].dir[dir] = true;
            }
        }
    }

    std::vector<VertexData> temp_raw_vertices;
    std::vector<VertexData> temp_simplified_vertices;

    std::vector<VertexData> simplified_vertices;

    for (unsigned i = 0; i < spans.size(); ++ i) {
        Span const & span = spans[i];

        bool diff = neighbour_diffs[i].north ||
                    neighbour_diffs[i].east  ||
                    neighbour_diffs[i].south ||
                    neighbour_diffs[i].west;

        if (span.region_index == -1 || !diff) {
            continue;
        }

        unsigned dir = 0;
        bool only_connected_to_null = true;

        while (!neighbour_diffs[i].dir[dir]) {
            ++dir;
        }

        temp_raw_vertices.resize(0);
        temp_simplified_vertices.resize(0);

        build_raw_contour(
            origin,
            granularity,
            i,
            spans,
            dir,
            neighbour_diffs,
            temp_raw_vertices,
            only_connected_to_null
        );

        simplify_contour(
            only_connected_to_null,
            granularity,
            max_edge_deviation,
            max_edge_length,
            temp_raw_vertices,
            temp_simplified_vertices
        );

        simplified_vertices.insert(
            simplified_vertices.end(),
            temp_simplified_vertices.begin(),
            temp_simplified_vertices.end()
        );
    }

    if (simplified_vertices.empty()) {
        PRT3ERROR("Error: Failed to generate navigation mesh.\n");
        return NO_NAV_MESH;
    }

    std::stable_sort(simplified_vertices.begin(), simplified_vertices.end(),
    [](VertexData const & a, VertexData const & b) -> bool
    {
        return a.internal_region_index < b.internal_region_index;
    });

    /* create nav mesh */
    NavMeshID nav_mesh_id = insert_nav_mesh(node_id);
    NavigationMesh & nav_mesh = m_navigation_meshes.at(nav_mesh_id);

    /* contour */
    std::vector<SubVec> contours;
    std::vector<glm::vec3> contour_vertices;

    std::unordered_map<glm::ivec2, std::set<std::tuple<uint32_t, int> > > duplicates;

    for (auto const & vertex : simplified_vertices) {
        if (vertex.internal_region_index == -1) continue;
        if (static_cast<size_t>(vertex.internal_region_index + 1) >
            contours.size()) {
            contours.resize(vertex.internal_region_index + 1);
        }
        SubVec & contour = contours[vertex.internal_region_index];
        if (contour.num_indices == 0) {
            contour.start_index = contour_vertices.size();
        }
        ++contour.num_indices;

        glm::ivec2 trunc_coord;
        trunc_coord.x = vertex.position.x / granularity;
        trunc_coord.y = vertex.position.z / granularity;

        int y_coord = vertex.position.y / granularity;

        duplicates[trunc_coord].insert({contour_vertices.size(), y_coord});

        contour_vertices.push_back(vertex.position);
    }

    /* polygon mesh */
    std::vector<TriangleData> temp_indices;
    std::vector<uint32_t> temp_vertices;

    std::vector<int32_t> contour_to_mesh_indices;
    contour_to_mesh_indices.resize(contour_vertices.size(), -1);

    for (uint32_t contour_index = 0; contour_index < contours.size(); ++contour_index) {
        SubVec const & contour = contours[contour_index];
        if (contour.num_indices < 3) {
            PRT3WARNING("Warning: Too few vertices in contour to generate polygon. Skipping...\n");
            continue;
        }

        temp_indices.resize(0);
        temp_vertices.resize(0);

        for (uint32_t i = 0; i < contour.num_indices; ++i) {
            temp_indices.emplace_back(
                TriangleData{contour.start_index + i, false}
            );
        }

        if (!triangulate(
            contour_vertices.data(),
            temp_indices,
            temp_vertices
        )) {
            PRT3WARNING("Warning: Triangulation failed.\n");
            continue;
        }

        std::vector<glm::vec3> & vertices = nav_mesh.vertices;

        size_t vi = vertices.size();
        vertices.resize(vertices.size() + temp_vertices.size());
        for (uint32_t i : temp_vertices) {
            vertices[vi] = contour_vertices[i];
            contour_to_mesh_indices[i] = vi;
            ++vi;
        }
    }

    /* adjacencies */
    nav_mesh.adjacencies.resize(nav_mesh.vertices.size() / 3);

    uint32_t i = 0;
    for (glm::vec3 const & vertex : nav_mesh.vertices) {
        glm::ivec2 trunc_coord;
        trunc_coord.x = vertex.x / granularity;
        trunc_coord.y = vertex.z / granularity;

        int y_coord = vertex.y / granularity;

        uint32_t tri_index = i / 3;
        if (nav_mesh.adjacencies[tri_index].num_indices == 0) {
            nav_mesh.adjacencies[tri_index].start_index =
                nav_mesh.adjacency_data.size();
        }

        uint32_t vert_1a = i;
        uint32_t vert_1b = 3 * tri_index + ((i + 1) % 3);

        glm::ivec3 trunc_1a = nav_mesh.vertices[vert_1a] / granularity;
        glm::ivec3 trunc_1b = nav_mesh.vertices[vert_1b] / granularity;

        for (auto const & tuple : duplicates.at(trunc_coord)) {
            int y = std::get<0>(tuple);
            if (glm::abs(y_coord - y) > 1) continue;

            uint32_t index = std::get<1>(tuple);
            int32_t mesh_index = contour_to_mesh_indices[index];
            if (mesh_index == -1) continue;

            uint32_t other_tri_index = index / 3;
            if (other_tri_index == tri_index) continue;

            for (uint32_t j = 0; j < 3; ++j) {
                uint32_t vert_2a = 3 * other_tri_index + ((index + j) % 3);
                uint32_t vert_2b = 3 * other_tri_index + ((index + j + 1) % 3);

                glm::ivec3 trunc_2a = nav_mesh.vertices[vert_2a] /
                                      granularity;
                glm::ivec3 trunc_2b = nav_mesh.vertices[vert_2b] /
                                      granularity;

                glm::ivec3 diff_aa = glm::abs(trunc_1a - trunc_2a);
                glm::ivec3 diff_bb = glm::abs(trunc_1b - trunc_2b);
                glm::ivec3 diff_ab = glm::abs(trunc_1a - trunc_2b);
                glm::ivec3 diff_ba = glm::abs(trunc_1b - trunc_2a);

                bool aa = diff_aa.x == 0 && diff_aa.z == 0 && diff_aa.y < 2;
                bool bb = diff_bb.x == 0 && diff_bb.z == 0 && diff_bb.y < 2;
                bool ab = diff_ab.x == 0 && diff_ab.z == 0 && diff_ab.y < 2;
                bool ba = diff_ba.x == 0 && diff_ba.z == 0 && diff_ba.y < 2;

                if ((aa && bb) || (ab && ba)) {
                    float edge_length =
                        glm::distance(nav_mesh.vertices[vert_1a],
                                      nav_mesh.vertices[vert_1b]);

                    nav_mesh.adjacency_data.push_back({
                        edge_length, // portal_size
                        other_tri_index, // tri_index
                        vert_1a, // edge0
                        vert_1b // edge1
                    });
                    ++nav_mesh.adjacencies[tri_index].num_indices;
                }
            }
        }

        ++i;
    }

    /* spatial partitioning */
    n_tris = nav_mesh.vertices.size() / 3;
    aabbs.resize(n_tris, {glm::vec3{pos_inf}, glm::vec3{neg_inf}});

    tri_index = 0;
    for (size_t i = 0; i < n_tris; i += 3) {
        glm::vec3 & lb = aabbs[tri_index].lower_bound;
        glm::vec3 & ub = aabbs[tri_index].upper_bound;

        lb = glm::min(lb, nav_mesh.vertices[i]);
        lb = glm::min(lb, nav_mesh.vertices[i+1]);
        lb = glm::min(lb, nav_mesh.vertices[i+2]);

        ub = glm::max(ub, nav_mesh.vertices[i]);
        ub = glm::max(ub, nav_mesh.vertices[i+1]);
        ub = glm::max(ub, nav_mesh.vertices[i+2]);

        ++tri_index;
    }

    tags.resize(n_tris);
    layers.resize(n_tris, ~0);

    if (n_tris >= size_t(std::numeric_limits<ColliderID>::max())) {
        PRT3ERROR("%s\n", "ERROR: Too many triangles in navmesh.");
    }

    for (size_t i = 0; i < n_tris; ++i) {
        tags[i].id = i;
        // tags[i].shape = don't care;
        // tags[i].type = don't care;
    }

    nav_mesh.aabb_tree.insert(
        tags.data(),
        layers.data(),
        aabbs.data(),
        n_tris
    );

    return nav_mesh_id;
}

void NavigationSystem::remove_nav_mesh(NavMeshID id) {
    NodeID node_id = m_node_ids.at(id);
    m_node_ids.erase(id);
    m_nav_mesh_ids.erase(node_id);
    m_navigation_meshes.erase(id);
    m_render_meshes.erase(id);

    m_id_queue.push_back(id);
}


void NavigationSystem::serialize_nav_mesh(
    NavMeshID id,
    std::ostream & out
) const {
    NavigationMesh const & nav_mesh = m_navigation_meshes.at(id);
    write_stream(out, nav_mesh.vertices.size());
    write_stream_n(out, nav_mesh.vertices.data(), nav_mesh.vertices.size());

    write_stream(out, nav_mesh.adjacencies.size());
    write_stream_n(out, nav_mesh.adjacencies.data(), nav_mesh.adjacencies.size());

    write_stream(out, nav_mesh.adjacency_data.size());
    write_stream_n(out, nav_mesh.adjacency_data.data(), nav_mesh.adjacency_data.size());
}

NavMeshID NavigationSystem::deserialize_nav_mesh(
    NodeID node_id,
    std::istream & in
) {
    NavMeshID nav_mesh_id = insert_nav_mesh(node_id);
    NavigationMesh & nav_mesh = m_navigation_meshes.at(nav_mesh_id);

    size_t n_vert;
    read_stream(in, n_vert);
    nav_mesh.vertices.resize(n_vert);
    read_stream_n(in, nav_mesh.vertices.data(), n_vert);

    size_t n_adj;
    read_stream(in, n_adj);
    nav_mesh.adjacencies.resize(n_adj);
    read_stream_n(in, nav_mesh.adjacencies.data(), n_adj);

    size_t n_adj_data;
    read_stream(in, n_adj_data);
    nav_mesh.adjacency_data.resize(n_adj_data);
    read_stream_n(in, nav_mesh.adjacency_data.data(), n_adj_data);

    /* spatial partitioning */
    size_t n_tris = nav_mesh.vertices.size() / 3;
    std::vector<AABB> aabbs;
    float neg_inf = -std::numeric_limits<float>::infinity();
    float pos_inf = std::numeric_limits<float>::infinity();
    aabbs.resize(n_tris, {glm::vec3{pos_inf}, glm::vec3{neg_inf}});

    size_t tri_index = 0;
    for (size_t i = 0; i < n_tris; i += 3) {
        glm::vec3 & lb = aabbs[tri_index].lower_bound;
        glm::vec3 & ub = aabbs[tri_index].upper_bound;

        lb = glm::min(lb, nav_mesh.vertices[i]);
        lb = glm::min(lb, nav_mesh.vertices[i+1]);
        lb = glm::min(lb, nav_mesh.vertices[i+2]);

        ub = glm::max(ub, nav_mesh.vertices[i]);
        ub = glm::max(ub, nav_mesh.vertices[i+1]);
        ub = glm::max(ub, nav_mesh.vertices[i+2]);

        ++tri_index;
    }

    std::vector<ColliderTag> tags;
    std::vector<CollisionLayer> layers;
    tags.resize(n_tris);
    layers.resize(n_tris, ~0);

    if (n_tris >= size_t(std::numeric_limits<ColliderID>::max())) {
        PRT3ERROR("%s\n", "ERROR: Too many triangles in navmesh.");
    }

    for (size_t i = 0; i < n_tris; ++i) {
        tags[i].id = i;
        // tags[i].shape = don't care;
        // tags[i].type = don't care;
    }

    nav_mesh.aabb_tree.insert(
        tags.data(),
        layers.data(),
        aabbs.data(),
        n_tris
    );

    return nav_mesh_id;
}

void NavigationSystem::update_render_data(Renderer & renderer) {
    for (auto const & pair : m_nav_mesh_ids) {
        NavMeshID id = pair.second;
        if (m_render_meshes.find(id) != m_render_meshes.end()) continue;
        NavigationMesh const & nav_mesh = m_navigation_meshes.at(id);

        auto const & tris = nav_mesh.vertices;
        size_t n = tris.size();

        thread_local std::vector<glm::vec3> lines;
        lines.resize(n * 2);

        size_t n_tri = n / 3;
        size_t li = 0;
        for (size_t i = 0; i < n_tri; ++i) {
            lines[li] = tris[3*i];
            ++li;
            lines[li] = tris[3*i+1];
            ++li;
            lines[li] = tris[3*i+1];
            ++li;
            lines[li] = tris[3*i+2];
            ++li;
            lines[li] = tris[3*i+2];
            ++li;
            lines[li] = tris[3*i];
            ++li;
        }

        if (m_render_meshes.find(id) == m_render_meshes.end()) {
            m_render_meshes[id] =
                renderer.upload_line_mesh(lines.data(), lines.size());
        } else {
            renderer.update_line_mesh(
                m_render_meshes.at(id), lines.data(), lines.size()
            );
        }
    }
}

void NavigationSystem::collect_render_data(
    Renderer & renderer,
    NodeID selected_node,
    EditorRenderData & data
) {
    update_render_data(renderer);

    size_t i = data.line_data.size();
    data.line_data.resize(
        data.line_data.size() + m_render_meshes.size()
    );

    if (m_nav_mesh_ids.find(selected_node) != m_nav_mesh_ids.end()) {
        NavMeshID id = m_nav_mesh_ids.at(selected_node);
        data.line_data[i].mesh_id = m_render_meshes.at(id);
        data.line_data[i].color = glm::vec4{0.0f, 0.0f, 1.0f, 1.0f};
        data.line_data[i].transform = glm::mat4{1.0f};
        ++i;
    }

}
