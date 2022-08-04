#include "context.h"

using namespace prt3;

Context::Context()
 : m_renderer{*this, 640, 480, 1},
   m_model_manager{*this},
   m_scene{*this} {

}
