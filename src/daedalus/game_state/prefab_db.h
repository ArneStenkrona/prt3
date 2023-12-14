#ifndef DDS_PREFAB_DB_H
#define DDS_PREFAB_DB_H

#include "src/engine/scene/prefab.h"

#include <array>

namespace dds {

class PrefabDB {
public:
    enum PrefabID {
        player,
        camera,
        dark_flames,
        n_prefab_ids,
        none = n_prefab_ids
    };

    PrefabDB();

    prt3::Prefab const & get(PrefabID id) const {
        return m_prefabs[id];
    }

private:
    std::vector<prt3::Prefab> m_prefabs;
};

} // namespace dds

#endif // DDS_PREFAB_DB_H
