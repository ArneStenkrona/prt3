#ifndef PRT3_NAVIGATION_SYSTEM_H
#define PRT3_NAVIGATION_SYSTEM_H

#include "src/engine/physics/aabb.h"
#include "src/engine/physics/aabb_tree.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/renderer.h"
#include "src/util/sub_vec.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <array>

namespace prt3 {

class Scene;

typedef int32_t NavMeshID;
constexpr NavMeshID NO_NAV_MESH = -1;

struct Adjacency {
    float portal_size;
    uint32_t tri_index;
    uint32_t edge0;
    uint32_t edge1;
};

class NavigationSystem {
public:
    NavMeshID generate_nav_mesh(
        NodeID node_id,
        Scene const & scene,
        CollisionLayer layer,
        float granularity,
        float max_edge_deviation,
        float max_edge_length,
        float min_height,
        float min_width
    );

    void remove_nav_mesh(NavMeshID id);

    void serialize_nav_mesh(
        NavMeshID id,
        std::ostream & out
    ) const;

    NavMeshID deserialize_nav_mesh(
        NodeID node_id,
        std::istream & in
    );

    void collect_render_data(
        Renderer & renderer,
        NodeID selected_node,
        EditorRenderData & data
    );

    bool generate_path(
        glm::vec3 origin,
        glm::vec3 destination,
        std::vector<glm::vec3> & path
    ) const;

    bool generate_path(
        NavMeshID nav_mesh_id,
        glm::vec3 origin,
        glm::vec3 destination,
        std::vector<glm::vec3> & path
    ) const;

private:
    struct NavigationMesh {
        std::vector<glm::vec3> vertices;
        std::vector<SubVec> neighbours;
        std::vector<uint32_t> neighbour_indices;
        /* per triangle */
        std::vector<SubVec> adjacencies;
        std::vector<Adjacency> adjacency_data;
        std::vector<uint8_t> island_indices;
        DynamicAABBTree aabb_tree;
    };

    std::unordered_map<NodeID, NavMeshID> m_nav_mesh_ids;
    std::unordered_map<NavMeshID, NodeID> m_node_ids;
    std::vector<NavMeshID> m_id_queue;

    std::unordered_map<NavMeshID, NavigationMesh> m_navigation_meshes;
    std::unordered_map<NavMeshID, ResourceID> m_render_meshes;

    bool get_tri_origin_dest(
        NavigationMesh const & nav_mesh,
        glm::vec3 origin,
        glm::vec3 destination,
        uint32_t & tri_origin,
        uint32_t & tri_dest
    ) const;

    bool generate_path(
        NavMeshID nav_mesh_id,
        glm::vec3 origin,
        glm::vec3 destination,
        uint32_t tri_origin,
        uint32_t tri_dest,
        std::vector<glm::vec3> & path
    ) const;

    void update_render_data(Renderer & renderer);

    NavMeshID insert_nav_mesh(NodeID node_id) {
        NavMeshID nav_mesh_id = m_navigation_meshes.size();
        if (!m_id_queue.empty()) {
            nav_mesh_id = m_id_queue.back();
            m_id_queue.pop_back();
        }
        m_node_ids[nav_mesh_id] = node_id;
        m_nav_mesh_ids[node_id] = nav_mesh_id;

        m_navigation_meshes[nav_mesh_id] = {};
        return nav_mesh_id;
    }

    void clear();

    friend class Scene;
};

} // namespace prt3

#endif // PRT3_NAVIGATION_SYSTEM_H
