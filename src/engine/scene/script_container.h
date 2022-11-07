#ifndef PRT3_SCRIPT_CONTAINER_H
#define PRT3_SCRIPT_CONTAINER_H

#include "src/engine/component/script/script.h"

#include <vector>

namespace prt3 {

class ScriptContainer {
public:
    ~ScriptContainer();
    ScriptContainer & operator=(ScriptContainer const & other);

    void update(Scene & scene, float delta_time);
    void clear();

    std::vector<Script *> const & scripts() const { return m_scripts; }
    std::vector<Script *> const & init_queue() const { return m_init_queue; }
    std::vector<Script *> const & late_init_queue() const{ return m_late_init_queue; }

    std::vector<Script *> & scripts() { return m_scripts; }
    std::vector<Script *> & init_queue() { return m_init_queue; }
    std::vector<Script *> & late_init_queue() { return m_late_init_queue; }
private:
    std::vector<Script *> m_scripts;
    std::vector<Script *> m_init_queue;
    std::vector<Script *> m_late_init_queue;
};

} // namespace prt3

#endif
