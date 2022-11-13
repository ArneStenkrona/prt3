#include "context.h"

using namespace prt3;

Context::Context()
 : m_renderer{*this, 960, 540, 1.0f},
   m_material_manager{*this},
   m_model_manager{*this},
   m_edit_scene{*this},
   m_game_scene{*this} {

}

Context::~Context() {
  m_edit_scene.clear();
  m_game_scene.clear();
}
