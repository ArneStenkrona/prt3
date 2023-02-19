#include "script.h"

#include "src/engine/scene/scene.h"
#include "src/engine/component/script/script_include.h"

using namespace prt3;

Script::Script(Scene &, NodeID node_id)
 : m_node_id{node_id} {
}

Node & Script::get_node(Scene & scene) { return scene.get_node(m_node_id); }

bool Script::add_tag(Scene & scene, NodeTag const & tag) {
    return scene.add_tag_to_node(tag, m_node_id);
}

std::unordered_map<UUID, Script::TScriptDeserializer> *
    Script::s_deserializers = nullptr;

std::unordered_map<UUID, Script::TScriptInstantiator> *
    Script::s_instantiators = nullptr;

std::unordered_map<UUID, char const *> *
    Script::s_script_names = nullptr;

Script * Script::deserialize(
    UUID uuid,
    std::istream & in,
    Scene & scene,
    NodeID node_id
) {
    if (auto it = s_deserializers->find(uuid); it != s_deserializers->end()) {
        return it->second(in, scene, node_id);
    }

    return nullptr;
}

Script * Script::instantiate(
    UUID uuid,
    Scene & scene,
    NodeID node_id
) {
    if (auto it = s_instantiators->find(uuid); it != s_instantiators->end()) {
        return it->second(scene, node_id);
    }

    return nullptr;
}

bool Script::Register(
    UUID uuid,
    char const * name,
    Script::TScriptDeserializer deserializer,
    Script::TScriptInstantiator instantiator
) {
    if (s_deserializers == nullptr) {
        static std::unique_ptr<
            std::unordered_map<UUID, Script::TScriptDeserializer>
        > deserializers{
            new std::unordered_map<UUID, Script::TScriptDeserializer>()
        };
        static std::unique_ptr<
            std::unordered_map<UUID, Script::TScriptInstantiator>
        > instantiators{
            new std::unordered_map<UUID, Script::TScriptInstantiator>()
        };
        static std::unique_ptr<std::unordered_map<UUID, char const *>
        > names{
            new std::unordered_map<UUID, char const *>()
        };

        s_deserializers = deserializers.get();
        s_instantiators = instantiators.get();
        s_script_names = names.get();
    }

    if (s_deserializers->find(uuid) != s_deserializers->end()) {
        return false;
    }
    (*s_script_names)[uuid] = name;
    (*s_deserializers)[uuid] = deserializer;
    (*s_instantiators)[uuid] = instantiator;

    return true;
}
