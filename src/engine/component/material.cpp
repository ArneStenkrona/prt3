#include "material.h"

using namespace prt3;

Material::Material(Scene &, NodeID node_id, ResourceID resource_id)
 : m_node_id{node_id},
   m_resource_id{resource_id} {}
