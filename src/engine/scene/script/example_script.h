#ifndef PRT3_EXAMPLE_SCRIPT_H
#define PRT3_EXAMPLE_SCRIPT_H

#include "src/engine/scene/script/script.h"
#include "src/engine/scene/scene.h"

#include <iostream>

namespace prt3 {

class ExampleScript : public Script {
public:
    explicit ExampleScript(Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    virtual void on_init() {
        std::cout << "on_init()" << std::endl;
    }
    virtual void on_update() {
        std::cout << "on_update()" << std::endl;
    }
private:
};

} // namespace prt3

#endif
