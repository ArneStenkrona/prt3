#ifndef PRT3_MODEL_COMPONENT_H
#define PRT3_MODEL_COMPONENT_H

#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/scene/node.h"
#include "src/util/uuid.h"

namespace prt3 {

class Scene;

template<typename T>
class ComponentStorage;

class EditorContext;

class ModelComponent {
public:
    ModelComponent(Scene & scene, NodeID node_id);
    ModelComponent(Scene & scene, NodeID node_id, ModelHandle model_handle);
    ModelComponent(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ModelHandle model_handle() const { return m_model_handle; }
    ModelHandle set_model_handle(ModelHandle handle)
    { return m_model_handle = handle; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Model"; }
    static constexpr UUID uuid = 10005303357075355173ull;

private:
    NodeID m_node_id;
    ModelHandle m_model_handle;

    void remove(Scene & /*scene*/) {}

    friend class ComponentStorage<ModelComponent>;
};

} // namespace prt3

#endif
