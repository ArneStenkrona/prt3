#include "mesh.h"

using namespace prt3;

Mesh::Mesh(Scene &, NodeID node_id, ResourceID resource_id)
 : m_resource_id{resource_id},
   m_node_id{node_id} {}

