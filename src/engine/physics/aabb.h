#ifndef PRT3_AABB_H
#define PRT3_AABB_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>

namespace prt3 {

struct AABB {
    glm::vec3 lower_bound;
    glm::vec3 upper_bound;

    AABB sweep(glm::vec3 const & translation) const {
        return { lower_bound + translation, upper_bound + translation };
    }

    /**
     * @return surface area of the AABB
     */
    float area() const;
    /**
     * @return volume of the AABB
     */
    float volume() const;

    /**
     * @param a
     * @param b
     * @return  true if a intersects with b,
     *          false otherwise
     */
    static bool intersect(AABB const & a, AABB const & b);

    /**
     * @param a
     * @param b
     * @param result modifies result so that it contains
     *               true at index i if AABB in b at index
     *               i intersects a, false otherwise
     */
    static void intersect(AABB const & a,
                          std::vector<AABB> const & b,
                          std::vector<bool> & result);

    /**
     * @param a aabb
     * @param origin origin of line segment
     * @param end end of line segment
     * @return  true if a intersects with line segment,
     *          false otherwise
     */
    static bool intersect_ray(AABB const & a,
                              glm::vec3 const & origin,
                              glm::vec3 const & direction,
                              float max_distance);
    /**
     * @param other
     * @return true if aabb encloses other,
     *         false otherwise
     */
    bool contains(AABB const & other);

    /**
     * expands the AABB to the AABB enclosing
     * both operand AABB's
     * @param rhs right-hand side of the operation
     * @return reference to result
     */
    AABB& operator+=(AABB const & rhs);
    /**
     * returns the AABB enclosing both operands'
     * AABB's
     * @param lhs left-hand side of the operation
     * @param rhs right-hand side of the operation
     * @return AABB that encloses both lhs and rhs
     */
    friend AABB operator+(AABB lhs, AABB const & rhs) {
        lhs += rhs;
        return lhs;
    }
};

} // namespace prt3

#endif