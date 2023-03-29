#include "navigation_system.h"

#include "src/engine/physics/aabb.h"
#include "src/engine/physics/aabb_tree.h"
#include "src/util/log.h"
#include "src/util/geometry_util.h"

#include <vector>
#include <queue>
#include <set>

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
        uint16_t neighbours[4];
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
    uint32_t y = span.low;
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

void reinsert_null_region_vertices(
    std::vector<VertexData> const & raw_vertices,
    std::vector<VertexData> & simplified_vertices
) {
    unsigned int vert_a = 0;
    while (vert_a < simplified_vertices.size()) {
        uint32_t vert_b = (vert_a + 1) % simplified_vertices.size();
        uint32_t raw_index_1 = simplified_vertices[vert_a].raw_index;
        uint32_t raw_index_2 = simplified_vertices[vert_b].raw_index;
        uint32_t vertex_to_test = (raw_index_1 + 1) % raw_vertices.size();

        float max_deviation = 0.0f;
        int32_t vertex_to_insert = -1;

        if (raw_vertices[vertex_to_test].external_region_index == -1) {
            while (vertex_to_test != raw_index_2) {
                float deviation = point_dist_to_segment(
                    raw_vertices[vertex_to_test].position,
                    simplified_vertices[vert_a].position,
                    simplified_vertices[vert_b].position
                )

                if (deviation > max_deviation) {
                    max_deviation = deviation;
                    vertex_to_insert = vertex_to_test
                }

                vertex_to_test = (vertex_to_test + 1) % raw_vertices.size();
            }
        }

        if (vertex_to_insert != -1 && max_deviation > edge_max_deviation) {
            // insert vertex_to_insert at vert_a + 1
        } else {
            ++vert_a;
        }
    }
}

void simplify_contour(
    bool only_connected_to_null,
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

    reinsert_null_region_vertices(raw_vertices, simplified_vertices);
    check_null_region_max_edge(raw_vertices, simplified_vertices);
    remove_duuplicate_vertices(simplified_vertices);
}

// Many thanks to https://javid.nl/atlas.html for an excellent
// breakdown of navigation mesh generation
NavMeshID generate_nav_mesh(
    float granularity,
    float min_height,
    float min_width,
    glm::vec3 const * geometry_data,
    size_t n_geometry
) {
    assert(n_geometry % 3 == 0);
    if (n_geometry == 0) return NO_NAV_MESH;

    DynamicAABBTree aabb_tree;

    glm::vec3 lower_bound{geometry_data[0]};
    glm::vec3 upper_bound{geometry_data[0]};

    AABB aabb;

    // generate aabb of all geometry
    for (size_t i = 1; i < n_geometry; ++i) {
        aabb.lower_bound = glm::min(aabb.lower_bound, geometry_data[i]);
        aabb.upper_bound = glm::max(aabb.upper_bound, geometry_data[i]);
    }

    // insert each triangle into the aabb_tree
    float neg_inf = -std::numeric_limits<float>::infinity();
    float pos_inf = std::numeric_limits<float>::infinity();
    std::vector<AABB> aabbs;
    size_t n_tris = n_geometry / 3;
    aabbs.resize(n_tris, {glm::vec3{pos_inf}, glm::vec3{neg_inf}});

    size_t tri_index = 0;
    for (size_t i = 0; i < n_geometry; i += 3) {
        glm::vec3 & lb = aabbs[tri_index].lower_bound;
        glm::vec3 & ub = aabbs[tri_index].upper_bound;

        lb = glm::min(lb, geometry_data[i]);
        lb = glm::min(lb, geometry_data[i+1]);
        lb = glm::min(lb, geometry_data[i+2]);

        ub = glm::max(ub, geometry_data[i]);
        ub = glm::max(ub, geometry_data[i+1]);
        ub = glm::max(ub, geometry_data[i+2]);

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
    glm::ivec3 dim =
        glm::ivec3{glm::ceil(aabb.upper_bound / granularity)} -
        glm::ivec3{glm::floor(aabb.lower_bound / granularity)};

    std::vector<bool> voxels;
    voxels.resize(dim.x * dim.y * dim.z);

    glm::vec3 origin = aabb.lower_bound;

    constexpr ColliderTag tag = { -1, ColliderShape::none, ColliderType::collider };
    std::vector<ColliderTag> tags;

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
                        geometry_data[3 * tag.id],
                        geometry_data[3 * tag.id + 1],
                        geometry_data[3 * tag.id + 2]
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
                            start,
                            iy,
                            ix,
                            iz,
                            -1,
                            -1,
                            -1,
                            -1,
                            -1,
                            0
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

        --current_dist;
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

    std::vector<VertexData> raw_vertices;
    std::vector<VertexData> simplified_vertices;

    for (unsigned i = 0; i < spans.size(); ++ i) {
        Span const & span = spans[i];

        bool diff = neighbour_diffs[i].north |
                    neighbour_diffs[i].east  |
                    neighbour_diffs[i].south |
                    neighbour_diffs[i].west;

        if (span.region_index == -1 || diff) {
            continue;
        }

        unsigned dir = 0;
        bool only_connected_to_null = true;

        while (!neighbour_diffs[i].dir[dir]) {
            ++dir;
        }

        raw_vertices.resize(0);
        simplified_vertices.resize(0);

        build_raw_contour(
            origin,
            granularity,
            i,
            spans,
            dir,
            neighbour_diffs,
            raw_vertices,
            only_connected_to_null
        );

        simplify_contour(
            only_connected_to_null,
            raw_vertices,
            simplified_vertices
        );

        simplified_vertices.insert(
            simplified_vertices.end(),
            raw_vertices.begin(),
            raw_vertices.end()
        );
    }
}
