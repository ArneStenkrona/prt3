#ifndef PRT3_RANDOM_H
#define PRT3_RANDOM_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <random>

template<typename T>
T random_int() {
    thread_local std::random_device rd;
    thread_local std::mt19937_64 gen{rd()};
    thread_local std::uniform_int_distribution<T> dis{
        std::numeric_limits<T>::min(),
        std::numeric_limits<T>::max()
    };
    return dis(gen);
}

/* emits random float between 0.0f and 1.0f, inclusive */
inline float random_float() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

inline glm::vec3 random_direction() {
    float theta = 2.0f * glm::pi<float>() * random_float();

    float z = 2.0f * random_float() - 1.0f;
    float r = glm::sqrt(1.0f - z * z);
    return glm::vec3{
        r * glm::cos(theta),
        r * glm::sin(theta),
        z
    };
}

#endif
