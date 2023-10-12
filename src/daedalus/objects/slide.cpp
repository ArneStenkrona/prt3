#include "slide.h"

#include "src/daedalus/objects/interactable.h"

using namespace dds;

void Slide::on_signal(
    prt3::Scene & /*scene*/,
    prt3::SignalString const & signal,
    void * /*data*/
) {
    prt3::SignalString const & interact_pref = Interactable::interact_signal();
    if (!strncmp(signal.data(), interact_pref.data(), interact_pref.len())) {
        m_active = true;
        m_displace = !m_displace;
    }
}

void Slide::on_init(prt3::Scene & scene) {
    m_original_position = get_node(scene).local_transform().position;
    m_active = false;
    m_displace = false;
}

void Slide::on_update(prt3::Scene & scene, float delta_time) {
    if (!m_active) {
        return;
    }
    glm::vec3 curr_pos = get_node(scene).local_transform().position;
    glm::vec3 target = m_displace ? m_original_position + m_local_displacement :
                                    m_original_position;

    float speed = 10.0f;
    float t = glm::pow(0.5f, delta_time * speed);
    glm::vec3 new_pos = glm::mix(target, curr_pos, t);

    get_node(scene).local_transform().position = new_pos;
}
