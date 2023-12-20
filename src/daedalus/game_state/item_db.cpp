#include "item_db.h"

using namespace dds;

ItemDB::ItemDB() {
    m_items[spell_flame_pillar] = { 1.00f, ItemType::spell };
}
