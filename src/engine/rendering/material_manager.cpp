#include "material_manager.h"

#include "src/engine/core/context.h"

using namespace prt3;

MaterialManager::MaterialManager(Context & context)
 : m_context{context}
{}

Material const & MaterialManager::get_material(ResourceID id) const
{ return m_context.renderer().get_material(id); }

Material & MaterialManager::get_material(ResourceID id)
{ return m_context.renderer().get_material(id); }
