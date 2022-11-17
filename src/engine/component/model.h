#ifndef PRT3_MODEL_COMPONENT_H
#define PRT3_MODEL_COMPONENT_H

#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/scene/node.h"

namespace prt3 {

class Scene;

template<typename T>
class ComponentStorage;

class EditorContext;
template<typename T>
void inner_show_component(EditorContext &, NodeID);

class ModelComponent {
public:
    ModelComponent(Scene & scene, NodeID node_id);
    ModelComponent(Scene & scene, NodeID node_id, ModelHandle model_handle);
    ModelComponent(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ModelHandle model_handle() const { return m_model_handle; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Model"; }

private:
    NodeID m_node_id;
    ModelHandle m_model_handle;

    void remove(Scene & /*scene*/) {}

    friend class ComponentStorage<ModelComponent>;

    friend void inner_show_component<ModelComponent>(EditorContext &, NodeID);
};

} // namespace prt3

#endif