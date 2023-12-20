#ifndef DDS_ITEM_DB_H
#define DDS_ITEM_DB_H

#include "src/daedalus/game_state/id.h"

#include <array>

namespace dds {

enum class ItemType {
    spell
};

struct Item {
    float use_duration; // estimate during simulation
    ItemType type;
};

class ItemDB {
public:
    enum ItemID : DDSID {
        spell_flame_pillar,
        n_item_ids,
        none = n_item_ids
    };

    ItemDB();

    Item const & get(ItemID id) const {
        return m_items[id];
    }

private:
    std::array<Item, n_item_ids> m_items;
};

} // namespace dds

#endif // DDS_ITEM_DB_H
