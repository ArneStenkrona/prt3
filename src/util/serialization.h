#ifndef PRT3_SERIALIZATION_H
#define PRT3_SERIALIZATION_H

#include "src/engine/scene/scene.h"

#include <iostream>

namespace prt3 {

void serialize_scene(Scene const & scene, std::ostream & stream);
void deserialize_scene(std::istream & stream, Scene & scene);


} // namespace prt3

#endif
