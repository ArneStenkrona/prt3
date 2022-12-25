#ifndef PRT3_ARMATURE_H
#define PRT3_ARMATURE_H

#include "src/engine/animation/animation.h"
#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/scene/node.h"
#include "src/util/uuid.h"

namespace prt3 {

template<typename T>
class ComponentStorage;

class EditorContext;

class Armature {
public:
    Armature(Scene & scene, NodeID node_id);
    Armature(Scene & scene, NodeID node_id, ModelHandle model_handle);
    Armature(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ModelHandle model_handle() const { return m_model_handle; }
    void set_model_handle(
        Scene & scene,
        ModelHandle handle
    );

    AnimationID animation_id() const { return m_animation_id; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Armature"; }
    static constexpr UUID uuid = 10453986613637604333ull;

private:
    NodeID m_node_id;
    ModelHandle m_model_handle = NO_MODEL;
    AnimationID m_animation_id = NO_ANIMATION;

    void remove(Scene & /*scene*/);

    friend class ComponentStorage<Armature>;
};

} // namespace prt3

#endif
