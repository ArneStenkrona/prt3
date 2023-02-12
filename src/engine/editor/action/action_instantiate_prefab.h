#ifndef PRT3_ACTION_INSTANTIATE_PREFAB_H
#define PRT3_ACTION_INSTANTIATE_PREFAB_H

#include "src/engine/editor/action/action.h"
#include "src/engine/scene/prefab.h"

namespace prt3 {

class ActionInstantiatePrefab : public Action {
public:
    ActionInstantiatePrefab(
        EditorContext & editor_context,
        NodeID parent_id,
        const char * path
    );

protected:
    virtual bool apply();
    virtual bool unapply();

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;
    NodeID m_parent_id;

    Prefab m_prefab;
};

} // namespace prt3

#endif // PRT3_ACTION_INSTANTIATE_PREFAB_H
