#include "navigation_system.h"

#include "src/engine/physics/aabb.h"
#include "src/engine/physics/aabb_tree.h"
#include "src/util/log.h"
#include "src/util/geometry_util.h"

#include <vector>
#include <queue>

using namespace prt3;

struct Column {
    uint32_t index;
    uint32_t count;
};

struct Span {
    uint16_t low;
    uint16_t high;
    int32_t next_index;
    union {
        struct {
            int16_t north;
            int16_t east;
            int16_t south;
            int16_t west;
        };
        uint16_t neighbours[4];
    };
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

// Many thanks to https://javid.nl/atlas.html for an excellent
// breakdown of navigation mesh generation
NavMeshID generate_nav_mesh(
    float granularity,
    float min_height,
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

    glm::vec3 o = aabb.lower_bound;

    constexpr ColliderTag tag = { -1, ColliderShape::none, ColliderType::collider };
    std::vector<ColliderTag> tags;

    uint32_t iv = 0;
    for (unsigned int ix = 0; ix < dim.x; ++ix) {
        for (unsigned int iz = 0; iz < dim.z; ++iz) {
            for (unsigned int iy = 0; iy < dim.y; ++iy) {
                AABB voxel_aabb;
                voxel_aabb.lower_bound =
                    o + glm::vec3{ix, iy, iz} * granularity;
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
                        o + glm::vec3{granularity / 2.0f},
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

    for (unsigned int ix = 0; ix < dim.x; ++ix) {
        for (unsigned int iz = 0; iz < dim.z; ++iz) {
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
                            -1
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
}
