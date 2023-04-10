#include "navigation_system.h"

#include "src/util/log.h"
#include "src/util/geometry_util.h"
#include "src/engine/scene/scene.h"

#include <glm/gtx/hash.hpp>

#include <vector>
#include <queue>
#include <set>
#include <unordered_map>

using namespace prt3;

struct Column {
    uint32_t index;
    uint32_t count;
};

struct Span {
    uint16_t low;
    uint16_t high;
    uint32_t x;
    uint32_t z;
    int32_t next_index;
    int32_t region_index;
    union {
        struct {
            int16_t north;
            int16_t east;
            int16_t south;
            int16_t west;
        };
        int16_t neighbours[4];
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
};

struct TriangleData {
    uint32_t index;
    bool is_triangle_center_index;
};

inline bool connected(
    Span const & span,
    int16_t index,
    std::vector<Column> const & columns,
    std::vector<Span> const & spans,
    uint16_t required_height
) {
    if (index < 0) return false;

    Column const & column = columns[index];

    for (uint32_t i = column.index; i < column.index + column.count; ++i) {
        Span const & other = spans[i];

        bool slope_ok = abs(int32_t(span.low) - int32_t(other.low)) > 1;

        uint16_t max_low = span.low > other.low ? span.low : other.low;
        uint16_t min_high = span.high < other.high ? span.high : other.high;
        bool height_ok  = max_low - min_high < required_height;

        if (slope_ok && height_ok) {
            return true;
        }
    }
    return false;
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

    if (diag_span_index) {
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

    while (max_iter != -1 && iter > max_iter) {
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
                if (neighbour.distance_to_region_center + 2 <
                    distance_to_region_center) {

                    region_index = neighbour.region_index;
                    distance_to_region_center =
                        neighbour.distance_to_region_center + 2;
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
            float pos_z = origin.z + granularity * z;

            // TODO: double-check this
            switch (dir) {
                case 1: pos_x += granularity; break;
                case 2: pos_x += granularity; pos_y -= granularity; break;
                case 3: pos_y -= granularity; break;
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

            vertex.raw_index = raw_index;
            vertices.push_back(vertex);

            neighbour_diffs[curr_i].dir[dir] = false;
            dir = (dir + 1) % 4;
            ++raw_index;
        } else {
            curr_i = spans[curr_i].neighbours[dir];

            // TODO: double check
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
    if (line_dist == 0) return glm::distance(p, l1);
    float t = glm::clamp(glm::dot(p - l1, l2 - l1) / line_dist, 0.0f, 1.0f);
    return glm::distance(p, t * (l2 - l1));
}

void reinsert_null_region_vertices(
    float max_edge_deviation,
    std::vector<VertexData> const & raw_vertices,
    std::vector<VertexData> & simplified_vertices
) {
    unsigned int vertex_a = 0;
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
    if (max_edge_length < granularity) {
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
            float dist_x = raw_vertices[raw_index_a].position.x -
                           raw_vertices[raw_index_b].position.x;
            float dist_y = raw_vertices[raw_index_a].position.y -
                           raw_vertices[raw_index_b].position.y;

            if (dist_x * dist_x + dist_y * dist_y >
                max_edge_length * max_edge_length) {
                uint32_t index_distance = raw_index_b < raw_index_a ?
                    raw_index_b + (raw_vertices.size() - raw_index_a) :
                    raw_index_b - raw_index_a;

                new_vertex = (raw_index_a + index_distance / 2) %
                              raw_vertices.size();
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
        if (dist2 < eps * eps) {
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

            if (pos.x < bottom_left.position.x ||
                (pos.x == bottom_left.position.x &&
                 pos.y < bottom_left.position.y )) {
                bottom_left = vertex;
            }

            if (pos.x > top_right.position.x ||
                (pos.x == top_right.position.x &&
                 pos.y > top_right.position.y )) {
                top_right = vertex;
            }
        }

        simplified_vertices.push_back(bottom_left);
        simplified_vertices.push_back(top_right);
    } else {
        for (unsigned int i = 0; i < raw_vertices.size(); ++i) {
            int32_t region_1 = raw_vertices[i].external_region_index;
            unsigned int next_i = (i + 1) % raw_vertices.size();
            int32_t region_2 = raw_vertices[next_i].external_region_index;

            if (region_1 != region_2) {
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
    return (b.y - a.y) * (c.x - a.x) - (c.y - a.y) * (b.x - a.x);
}

inline bool is_left(
    glm::vec3 const & vertex,
    glm::vec3 const & a,
    glm::vec3 const & b
) {
    return double_signed_area(a, vertex, b) < 0;
}

inline bool is_left_or_collinear(
    glm::vec3 const & vertex,
    glm::vec3 const & a,
    glm::vec3 const & b
) {
    return double_signed_area(a, vertex, b) <= 0;
}

inline bool is_right(
    glm::vec3 const & vertex,
    glm::vec3 const & a,
    glm::vec3 const & b
) {
    return double_signed_area(a, vertex, b) > 0;
}

inline bool is_right_or_collinear(
    glm::vec3 const & vertex,
    glm::vec3 const & a,
    glm::vec3 const & b
) {
    return double_signed_area(a, vertex, b) >= 0;
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
    return (((q1.x-p1.x)*(p2.y-p1.y) - (q1.y-p1.y)*(p2.x-p1.x))
            * ((q2.x-p1.x)*(p2.y-p1.y) - (q2.y-p1.y)*(p2.x-p1.x)) < 0)
            &&
           (((p1.x-q1.x)*(q2.y-q1.y) - (p1.y-q1.y)*(q2.x-q1.x))
            * ((p2.x-q1.x)*(q2.y-q1.y) - (p2.y-q1.y)*(q2.x-q1.x)) < 0);
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
            if ((pb.x == a.x && pb.y == a.y) ||
                (pb.x == b.x && pb.y == b.y) ||
                (pe.x == a.x && pe.y == a.y) ||
                (pe.x == b.x && pe.y == b.y)) {
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
                float delta_y = vertices[indices[index_plus_2].index].y -
                    vertices[indices[i].index].y;

                float length_sq = delta_x * delta_x + delta_y * delta_y;

                if (length_sq < min_length_sq) {
                    min_length_sq = length_sq;
                    min_length_sq_index = i;
                }
            }
        }

        if (min_length_sq < std::numeric_limits<float>::max()) {
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

        if (is_valid_partition(index_minus, index_plus_1, vertices, indices)) {
            indices[index].is_triangle_center_index = true;
        } else {
            indices[index].is_triangle_center_index = false;
        }

        index_plus_2 = (index + 2) % indices.size();
        if (is_valid_partition(index_plus_1, index_plus_2, vertices, indices)) {
            indices[index_plus_1].is_triangle_center_index = true;
        } else {
            indices[index_plus_1].is_triangle_center_index = false;
        }
    }

    triangles.push_back(indices[0].index);
    triangles.push_back(indices[1].index);
    triangles.push_back(indices[2].index);

    return true;
}

// Many thanks to https://javid.nl/atlas.html
// and https://www.stefanolazzaroni.com/navigation-mesh-generation
NavMeshID NavigationSystem::generate_nav_mesh(
    Scene const & scene,
    CollisionLayer layer,
    float granularity,
    float max_edge_deviation,
    float max_edge_length,
    float min_width,
    float min_height
) {
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
    glm::uvec3 dim =
        glm::uvec3{glm::ceil(aabb.upper_bound / granularity)} -
        glm::uvec3{glm::floor(aabb.lower_bound / granularity)};

    std::vector<bool> voxels;
    voxels.resize(dim.x * dim.y * dim.z);

    glm::vec3 origin = aabb.lower_bound;

    constexpr ColliderTag tag = {
        static_cast<ColliderID>(-1),
        ColliderShape::none,
        ColliderType::collider
    };

    uint32_t iv = 0;
    for (unsigned int ix = 0; ix < dim.x; ++ix) {
        for (unsigned int iz = 0; iz < dim.z; ++iz) {
            for (unsigned int iy = 0; iy < dim.y; ++iy) {
                AABB voxel_aabb;
                voxel_aabb.lower_bound =
                    origin + glm::vec3{ix, iy, iz} * granularity;
                voxel_aabb.upper_bound =
                    voxel_aabb.lower_bound + glm::vec3{granularity};

                tags.resize(0);
                aabb_tree.query(
                    tag,
                    ~0,
                    aabb,
                    tags
                );

                for (ColliderTag const & tag : tags) {
                    if (triangle_box_overlap(
                        origin + glm::vec3{granularity / 2.0f},
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
    std::vector<Column> columns;
    columns.resize(dim.x * dim.z);

    for (uint32_t ix = 0; ix < dim.x; ++ix) {
        for (uint32_t iz = 0; iz < dim.z; ++iz) {
            uint32_t iv = (ix * dim.z * dim.y) + iz * dim.y;
            uint32_t span_start = spans.size();

            bool prev = true;
            uint16_t start = 0;
            for (uint16_t iy = 0; iy < dim.y; ++iy) {
                bool curr = voxels[iv];
                if (prev != curr || (iy + 1 == dim.y && !curr)) {
                    if (curr || (iy + 1 == dim.y)) {
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
                    } else {
                        start = iy;
                    }
                }

                prev = curr;
                ++iv;
            }

            Column & col = columns[ix * dim.z + iz];
            col.index = span_start;
            col.count = spans.size() - span_start;

            for (uint32_t i = span_start; i + 1 < spans.size(); ++i) {
                spans[i].next_index = i + 1;
            }
        }
    }

    /* Generate distance field */
    uint16_t req_height =  static_cast<uint16_t>(ceil(min_height / granularity));
    uint16_t req_width =  static_cast<uint16_t>(ceil(min_width / granularity));

    for (unsigned int ix = 0; ix < dim.x; ++ix) {
        for (unsigned int iz = 0; iz < dim.z; ++iz) {
            Column & col = columns[ix * dim.z + iz];

            // north, east, south, west
            int32_t ni = iz + 1 < dim.z ? (ix * dim.z + (iz + 1)) : -1;
            int32_t ei = ix + 1 < dim.x ? ((ix + 1) * dim.z + iz) : -1;
            int32_t si = iz > 0 ? (ix * dim.z + (iz - 1)) : -1;
            int32_t wi = ix > 0 ? ((ix - 1) * dim.z + iz) : -1;

            for (uint32_t is = col.index; is < col.index + col.count; ++is) {
                Span & span = spans[is];
                span.north = connected(span, ni, columns, spans, req_height) ? ni : -1;
                span.east = connected(span, ei, columns, spans, req_height) ? ei : -1;
                span.south = connected(span, si, columns, spans, req_height) ? si : -1;
                span.west = connected(span, wi, columns, spans, req_height) ? wi : -1;
            }
        }
    }

    /* Generate distance field */
    std::queue<uint32_t> queue;

    enum Status : uint8_t {
        OPEN = 0,
        IN_PROGRESS = 1,
        CLOSED = 2
    };

    std::vector<Status> status;
    status.resize(spans.size());

    std::vector<uint16_t> distances;
    distances.resize(spans.size());

    for (uint32_t i = 0; i < spans.size(); ++i) {
        Span const & span = spans[i];
        bool border = false;
        border |= !span.north;
        border |= !span.east;
        border |= !span.south;
        border |= !span.west;

        if (border) {
            queue.push(i);
            status[i] = IN_PROGRESS;
        }
    }

    uint16_t max_dist = 0;
    while (!queue.empty()) {
        uint32_t index = queue.front();
        Span const & span = spans[index];

        for (unsigned int dir = 0; dir < 4; ++dir) {
            if (span.neighbours[dir] != -1) {
                int16_t other_ind = span.neighbours[dir];
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

        status[index] = CLOSED;
        queue.pop();
    }

    /* Create regions */
    std::set<uint32_t> flooded_spans;

    std::queue<uint32_t> span_queue;

    int expand_iter = 4 + req_width;

    int32_t region_index = 0;

    for (auto current_dist = max_dist;
        current_dist >= req_width;
        --current_dist) {
        for (size_t i = 0; i < spans.size(); ++i) {
            Span & span = spans[i];
            if (distances[i] == current_dist && span.region_index == -1) {
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
            auto fill_to = glm::max(
                uint16_t(current_dist - 2),
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

        if (span.region_index == -1 || diff) {
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

    /* polygon mesh */
    NavMeshID nav_mesh_id = m_navigation_meshes.size();
    m_navigation_meshes.push_back({});
    NavigationMesh & nav_mesh = m_navigation_meshes.back();

    if (simplified_vertices.empty()) {
        PRT3ERROR("Error: Failed to generate navigation mesh.\n");
        return NO_NAV_MESH;
    }

    std::stable_sort(simplified_vertices.begin(), simplified_vertices.end(),
    [](VertexData const & a, VertexData const & b) -> bool
    {
        return a.internal_region_index < b.internal_region_index;
    });

    std::vector<SubVec> contours;
    std::vector<glm::vec3> contour_vertices;

    std::unordered_map<glm::ivec3, std::set<uint32_t> > duplicates;

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

        glm::ivec3 trunc_coord;
        trunc_coord.x = vertex.position.x / granularity;
        trunc_coord.y = vertex.position.y / granularity;
        trunc_coord.z = vertex.position.z / granularity;

        duplicates[trunc_coord].insert(contour_vertices.size());

        contour_vertices.push_back(vertex.position);
    }

    std::vector<TriangleData> temp_indices;
    std::vector<uint32_t> temp_triangles;

    std::vector<glm::vec3> triangles;
    std::vector<int32_t> contour_to_mesh_indices;
    contour_to_mesh_indices.resize(contour_vertices.size(), -1);

    // std::vector<SubVec> temp_poly_sub;

    for (uint32_t contour_index = 0; contour_index < contours.size(); ++contour_index) {
        SubVec const & contour = contours[contour_index];
        if (contour.num_indices < 3) {
            PRT3WARNING("Warning: Too few vertices in contour to generate polygon. Skipping...\n");
            continue;
        }

        temp_indices.resize(0);
        temp_triangles.resize(0);

        for (uint32_t i = 0; i < contour.num_indices; ++i) {
            temp_indices.emplace_back(
                TriangleData{contour.start_index + i, false}
            );
        }

        if (!triangulate(
            contour_vertices.data(),
            temp_indices,
            temp_triangles
        )) {
            PRT3WARNING("Warning: Triangulation failed.\n");
            continue;
        }

        // // TODO: merge polygons if possible

        std::vector<glm::vec3> & vertices = nav_mesh.vertices;

        size_t vi = vertices.size();
        vertices.resize(vertices.size() + temp_triangles.size());
        for (uint32_t i : temp_triangles) {
            vertices[vi] = contour_vertices[i];
            contour_to_mesh_indices[i] = vi;
            ++vi;
        }
    }

    /* adjacencies */
    nav_mesh.adjacencies.resize(nav_mesh.vertices.size() / 3);

    size_t i = 0;
    for (glm::vec3 const & vertex : nav_mesh.vertices) {
        glm::ivec3 trunc_coord;
        trunc_coord.x = vertex.x / granularity;
        trunc_coord.y = vertex.y / granularity;
        trunc_coord.z = vertex.z / granularity;

        size_t tri_index = i / 3;
        if (nav_mesh.adjacencies[tri_index].num_indices == 0) {
            nav_mesh.adjacencies[tri_index].start_index =
                nav_mesh.adjacency_indices.size();
        }

        for (uint32_t index : duplicates.at(trunc_coord)) {
            int32_t mesh_index = contour_to_mesh_indices[index];
            if (mesh_index == -1) continue;
            nav_mesh.adjacency_indices.push_back(
                static_cast<uint32_t>(mesh_index)
            );
            ++nav_mesh.adjacencies[tri_index].num_indices;
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
