#ifndef PRT3_ANIMATED_MODEL_H
#define PRT3_ANIMATED_MODEL_H

#include "src/engine/animation/animation.h"
#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/scene/node.h"
#include "src/util/uuid.h"

namespace prt3 {

class Scene;

template<typename T>
class ComponentStorage;

class EditorContext;

class AnimatedModel {
public:
    AnimatedModel(Scene & scene, NodeID node_id);
    AnimatedModel(Scene & scene, NodeID node_id, ModelHandle model_handle);
    AnimatedModel(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ModelHandle model_handle() const { return m_model_handle; }
    ModelHandle set_model_handle(ModelHandle handle)
    { m_animation_id = 0; return m_model_handle = handle; }

    void set_animation_id(AnimationID id)
    { m_animation_id = id; }
    AnimationID animation_id() const { return m_animation_id; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Animated Model"; }
    static constexpr UUID uuid = 17658250485616706847ull;

private:
    NodeID m_node_id;
    ModelHandle m_model_handle;
    AnimationID m_animation_id = 0;

    void remove(Scene & /*scene*/) {}

    friend class ComponentStorage<AnimatedModel>;
};

} // namespace prt3

#endif
