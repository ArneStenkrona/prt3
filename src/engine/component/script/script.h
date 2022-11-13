#ifndef PRT3_SCRIPT_H
#define PRT3_SCRIPT_H

#include "src/engine/scene/node.h"
#include "src/engine/scene/signal.h"
#include "src/util/serialization_util.h"
#include "src/util/fixed_string.h"

#include <cstdint>
#include <unordered_map>
#include <iostream>

namespace prt3 {

typedef uint64_t UUID;

typedef unsigned int ScriptID;
static constexpr ScriptID NO_SCRIPT = -1;

class Scene;
class Script {
public:
    using TScriptDeserializer = Script *(*)(std::istream &, Scene &, NodeID);
    using TScriptInstantiator = Script *(*)(Scene &, NodeID);

    explicit Script(Scene & scene, NodeID node_id);
    virtual ~Script() {}

    virtual char const * name() = 0;

    virtual void on_init(Scene &) {}
    virtual void on_late_init(Scene &) {}
    virtual void on_update(Scene & /*scene*/, float /*delta_time*/) {}
    virtual void on_late_update(Scene & /*scene*/, float /*delta_time*/) {}
    virtual void on_signal(
        Scene & /*scene*/,
        SignalString const & /*signal*/,
        void * /*data*/
    ) {}

    virtual void serialize(std::ostream & out) const {
        write_stream(out, uuid());
    }

    virtual Script * copy() const = 0;

    static Script * deserialize(
        UUID uuid,
        std::istream & in,
        Scene & scene,
        NodeID node_id
    );

    static Script * instantiate(
        UUID uuid,
        Scene & scene,
        NodeID node_id
    );

    virtual UUID uuid() const = 0;

    static std::unordered_map<UUID, char const *> const & script_names()
    { return s_script_names; }

protected:
    NodeID node_id() const { return m_node_id; }
    Node & get_node(Scene & scene);

    bool add_tag(Scene & scene, NodeTag const & tag);


    static std::unordered_map<UUID, TScriptDeserializer> s_deserializers;
    static std::unordered_map<UUID, TScriptInstantiator> s_instantiators;
    static std::unordered_map<UUID, char const *> s_script_names;

    static bool Register(
        UUID uuid,
        char const * name,
        TScriptDeserializer deserializer,
        TScriptInstantiator instantiator
    );

private:
    NodeID m_node_id;
};

} // namespace prt3

#endif
