#ifndef PRT3_TRANSFORM_CACHE
#define PRT3_TRANSFORM_CACHE

#include "src/engine/component/transform.h"
#include "src/engine/scene/node.h"

#include <vector>

namespace prt3 {

class TransformCache {
public:
    void collect_global_transforms(Node const * nodes,
                                   size_t n_nodes,
                                   NodeID root_id);

    std::vector<Transform> const & global_transforms() const
        { return m_global_transforms; }

    std::vector<Transform> const & global_transforms_history() const
        { return m_global_transforms_history; }

    void clear();

private:
    std::vector<Transform> m_global_transforms;
    std::vector<Transform> m_global_transforms_history;
};

} // namespace prt3

#endif
