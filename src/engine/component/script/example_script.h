#ifndef PRT3_EXAMPLE_SCRIPT_H
#define PRT3_EXAMPLE_SCRIPT_H

#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"
#include "src/util/log.h"

namespace prt3 {

class ExampleScript : public Script {
public:
    explicit ExampleScript(Scene & scene, NodeID node_id)
        : Script(scene, node_id) {}

    explicit ExampleScript(std::istream &, Scene & scene, NodeID node_id)
        : Script(scene, node_id) {}

    virtual void on_init(Scene &) {
        PRT3LOG("on_init()\n");
    }
    virtual void on_update(Scene &, float) {
        PRT3LOG("on_update()\n");
    }

REGISTER_SCRIPT(ExampleScript, example, 10447271495191217112)
};

} // namespace prt3

#endif
