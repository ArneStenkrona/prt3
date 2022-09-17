#ifndef PRT3_SCRIPT_H
#define PRT3_SCRIPT_H

#include "src/engine/scene/node.h"

namespace prt3 {

class Scene;
class Script {
public:
    explicit Script(Scene & scene, NodeID node_id);
    virtual ~Script() {}

    virtual void on_init() {}
    virtual void on_update(float /*delta_time*/) {}
    virtual void on_late_update(float /*delta_time*/) {}
protected:
    Scene & scene() const { return m_scene; }
    NodeID node_id() const { return m_node_id; }
    Node & get_node();

private:
    Scene & m_scene;
    NodeID m_node_id;
};

} // namespace prt3

#endif
