#include "prefab_db.h"

using namespace dds;

static char const * const id_to_str[PrefabDB::n_prefab_ids] = {
    "assets/prefabs/player.prefab", // player
    "assets/prefabs/camera.prefab", // camera
    "assets/prefabs/cool_turkey.prefab", // dark_flames
    "assets/prefabs/flame_pillar.prefab", // flame_illar
};

PrefabDB::PrefabDB() {
    for (unsigned i = 0; i < n_prefab_ids; ++i) {
        m_prefabs.emplace_back(id_to_str[i]);
    }
}
