#ifndef DDS_SLIDE_H
#define DDS_SLIDE_H

#include "src/daedalus/game_state/game_state.h"
#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"

#include <vector>

namespace dds {

class Slide : public prt3::Script {
public:
    explicit Slide(prt3::Scene & scene, prt3::NodeID node_id)
        : prt3::Script(scene, node_id) {}

    explicit Slide(
        std::istream &,
        prt3::Scene & scene,
        prt3::NodeID node_id
    )
        : prt3::Script(scene, node_id) {}

    virtual void on_init(prt3::Scene & scene);
    virtual void on_update(prt3::Scene & scene, float delta_time);

    glm::vec3 & local_displacement() { return m_local_displacement; }

    virtual void on_signal(
        prt3::Scene & scene,
        prt3::SignalString const & signal,
        void * data
    );

private:
    glm::vec3 m_original_position;
    bool m_active;
    bool m_displace;

    union {
        /* might be UB but what are you gonna do... */
        struct {
            float m_local_displacement_x;
            float m_local_displacement_y;
            float m_local_displacement_z;
        };
        glm::vec3 m_local_displacement;
    };

REGISTER_SCRIPT_BEGIN(Slide, slide, 6237164168861721380)
REGISTER_SERIALIZED_FIELD(m_local_displacement_x)
REGISTER_SERIALIZED_FIELD(m_local_displacement_y)
REGISTER_SERIALIZED_FIELD(m_local_displacement_z)
REGISTER_SCRIPT_END()
};

} // namespace dds

#endif // DDS_SLIDE_H
