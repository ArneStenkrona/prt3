#ifndef PRT3_PREFAB_H
#define PRT3_PREFAB_H

#include "src/engine/scene/scene.h"
#include "src/engine/scene/node.h"

#include "src/util/mem.h"

namespace prt3 {

class Prefab {
public:
    Prefab(char const * path);

    NodeID instantiate(Scene & scene, NodeID parent);

    static void serialize_node(
        Scene const & scene,
        NodeID node_id,
        std::ostream & out
    );

    static NodeID deserialize_node(
        Scene & scene,
        NodeID parent,
        std::istream & in
    );
private:
    std::vector<char> m_data;

};

} // namespace prt3

#endif // PRT3_PREFAB_H
