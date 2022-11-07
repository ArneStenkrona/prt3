#ifndef PRT3_EXAMPLE_SCRIPT_H
#define PRT3_EXAMPLE_SCRIPT_H

#include "src/engine/scene/script/script.h"
#include "src/engine/scene/scene.h"

#include <iostream>

namespace prt3 {

class ExampleScript : public Script {
public:
    explicit ExampleScript(Scene & scene, NodeID node_id)
        : Script(scene, node_id) {}

    virtual void on_init(Scene &) {
        std::cout << "on_init()" << std::endl;
    }
    virtual void on_update(Scene &, float) {
        std::cout << "on_update()" << std::endl;
    }

    virtual Script * copy() const { return new ExampleSCript(*this); }

protected:
    static constexpr UUID s_uuid = 10447271495191217112ull;
    virtual UUID uuid() const {
        return s_uuid;
    }

    static Script * deserialize(
        std::istream &,
        Scene & scene,
        NodeID node_id
    ) {
        return new ExampleScript(scene, node_id);
    }

    inline static bool s_registered =
        Script::Register(s_uuid, ExampleScript::deserialize);
private:
};

} // namespace prt3

#endif
