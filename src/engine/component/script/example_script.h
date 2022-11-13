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

    virtual char const * name() { return s_name; };

    virtual void on_init(Scene &) {
        std::cout << "on_init()" << std::endl;
    }
    virtual void on_update(Scene &, float) {
        std::cout << "on_update()" << std::endl;
    }

    virtual Script * copy() const { return new ExampleSCript(*this); }

    virtual UUID uuid() const {
        return s_uuid;
    }

protected:
    static constexpr UUID s_uuid = 10447271495191217112ull;
    static constexpr char const * s_name = "example";

    static Script * deserialize(
        std::istream &,
        Scene & scene,
        NodeID node_id
    ) {
        return new ExampleScript(scene, node_id);
    }

    static Script * new_instance(
        Scene & scene,
        NodeID node_id
    ) {
        return new ExampleScript(scene, node_id);
    }

    inline static bool s_registered =
        Script::Register(
            s_uuid,
            s_name,
            ExampleScript::deserialize,
            ExampleScript::new_instance
        );

private:
};

} // namespace prt3

#endif
