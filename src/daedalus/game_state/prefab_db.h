#ifndef DDS_PREFAB_DB_H
#define DDS_PREFAB_DB_H

#include "src/daedalus/game_state/id.h"
#include "src/engine/scene/prefab.h"

#include <array>

namespace dds {

class PrefabDB {
public:
    enum PrefabID : DDSID {
        player,
        camera,
        dark_flames,
        flame_pillar,
        fire_rock,
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
