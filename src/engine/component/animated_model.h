#ifndef PRT3_ANIMATED_MODEL_H
#define PRT3_ANIMATED_MODEL_H

#include "src/engine/animation/animation.h"
#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/rendering/render_data.h"
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
    void set_model_handle(
        Scene & scene,
        ModelHandle handle
    );

    AnimationID animation_id() const { return m_animation_id; }

    MaterialOverride & material_override() { return m_material_override; }
    MaterialOverride const & material_override() const
    { return m_material_override; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Animated Model"; }
    static constexpr UUID uuid = 17658250485616706847ull;

private:
    NodeID m_node_id;
    ModelHandle m_model_handle = NO_MODEL;
    AnimationID m_animation_id = NO_ANIMATION;
    // material override is not serialized
    MaterialOverride m_material_override = {};

    void remove(Scene & scene);

    friend class ComponentStorage<AnimatedModel>;
};

} // namespace prt3

#endif
