#ifndef PRT3_MATERIAL_MANAGER_H
#define PRT3_MATERIAL_MANAGER_H

#include "src/engine/rendering/material.h"
#include "src/engine/rendering/resources.h"

#include <unordered_set>

namespace prt3 {

class Context;
class ModelManager;

class MaterialManager{
public:
    MaterialManager(Context & context);

    ResourceID upload_material(Material const & material);

    Material const & get_material(ResourceID id) const;

    Material & get_material(ResourceID id);

    std::unordered_set<ResourceID> const & material_ids() const
    { return m_material_ids; }

private:
    Context & m_context;

    std::unordered_set<ResourceID> m_material_ids;
    friend class ModelManager;
};

} // namespace prt3

#endif
