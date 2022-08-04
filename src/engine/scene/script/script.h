#ifndef PRT3_SCRIPT_H
#define PRT3_SCRIPT_H

#include "src/engine/scene/node.h"

namespace prt3 {

class Scene;
class Script {
public:
    Script(Scene & scene, NodeID m_node_id);
    virtual ~Script() {}

    virtual void on_init() = 0;
    virtual void on_update() = 0;

protected:
    Scene & scene() { return m_scene; }
    Node & node();

private:
    Scene & m_scene;
    NodeID m_node_id;
};

} // namespace prt3

#endif
