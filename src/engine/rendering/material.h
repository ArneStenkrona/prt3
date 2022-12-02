#ifndef PRT3_MATERIAL_H
#define PRT3_MATERIAL_H

#include "src/engine/rendering/resources.h"

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
    std::string albedo_map;
    std::string normal_map;
    std::string metallic_map;
    std::string roughness_map;
    std::string ambient_occlusion_map;
};

} // namespace prt3

#endif
