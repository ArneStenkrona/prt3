#ifndef DDS_GAME_GUI_H
#define DDS_GAME_GUI_H

#include "src/engine/scene/scene.h"

#include <array>
#include <vector>

namespace dds {

class GameState;

class GameGui {
public:
    GameGui(prt3::Scene & scene);

    void on_update(
        prt3::Scene & scene,
        prt3::NodeID canvas_id,
        GameState const & game_state
    );
private:

    struct FontMetadata {
        std::array<prt3::FontChar, 256> char_data;
    };

    struct GUIAtlas {
        prt3::ResourceID texture = prt3::NO_RESOURCE;
        std::vector<FontMetadata> metadata;
    };

    GUIAtlas m_atlas;
};

} // namespace dds

#endif // DDS_GAME_GUI_H