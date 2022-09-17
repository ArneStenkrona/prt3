#include "script.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

Script::Script(Scene & scene, NodeID node_id)
 : m_scene{scene},
   m_node_id{node_id} {

}

Node & Script::get_node() { return m_scene.get_node(m_node_id); }
