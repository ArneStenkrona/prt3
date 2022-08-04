#include "context.h"

using namespace prt3;

Context::Context()
 : m_renderer{*this, 960, 540, 2.0f},
   m_model_manager{*this},
   m_scene{*this} {

}
