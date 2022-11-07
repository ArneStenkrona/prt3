#include "context.h"

using namespace prt3;

Context::Context()
 : m_renderer{*this, 960, 540, 1.0f},
   m_model_manager{*this},
   m_edit_scene{*this},
   m_game_scene{*this} {

}
