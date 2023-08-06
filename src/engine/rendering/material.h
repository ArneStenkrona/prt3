#ifndef PRT3_MATERIAL_H
#define PRT3_MATERIAL_H

#include "src/engine/rendering/texture_manager.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <string>

namespace prt3 {

struct Material {
    std::string name;
    glm::vec4 albedo{1.0f, 1.0f, 1.0f, 1.0f};
    float metallic = 0.0f;
    float roughness = 1.0f;
    float ao = 1.0f;
    float emissive = 0.0f;
    bool twosided = false;
    bool transparent = false;
    ResourceID albedo_map;
    ResourceID normal_map;
    ResourceID metallic_map;
    ResourceID roughness_map;
    ResourceID ambient_occlusion_map;
};

} // namespace prt3

#endif
