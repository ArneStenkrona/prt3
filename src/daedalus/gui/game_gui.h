#ifndef DDS_GAME_GUI_H
#define DDS_GAME_GUI_H

#include "src/engine/scene/scene.h"

namespace dds {

class GameGui {
public:
    void on_update(prt3::Scene & scene, prt3::NodeID canvas_id);
private:
};

} // namespace dds

#endif // DDS_GAME_GUI_H