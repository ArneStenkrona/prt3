#include "script.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

Script::Script(Scene & scene)
 : m_scene{scene} {

}

Node & Script::node() { return *m_scene.get_node(m_node_id); }
