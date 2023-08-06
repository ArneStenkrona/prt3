#include "mesh_util.h"

void prt3::insert_box(
    glm::vec3 const & lower_bound,
    glm::vec3 const & upper_bound,
    glm::vec3 * geo
) {
    // side 0
    geo[0]  = glm::vec3{ lower_bound.x, lower_bound.y, lower_bound.z };
    geo[1]  = glm::vec3{ upper_bound.x, upper_bound.y, lower_bound.z };
    geo[2]  = glm::vec3{ upper_bound.x, lower_bound.y, lower_bound.z };

    geo[3]  = glm::vec3{ lower_bound.x, lower_bound.y, lower_bound.z };
    geo[4]  = glm::vec3{ lower_bound.x, upper_bound.y, lower_bound.z };
    geo[5]  = glm::vec3{ upper_bound.x, upper_bound.y, lower_bound.z };

    // side 1
    geo[6]  = glm::vec3{ lower_bound.x, lower_bound.y, lower_bound.z };
    geo[7]  = glm::vec3{ upper_bound.x, lower_bound.y, lower_bound.z };
    geo[8]  = glm::vec3{ upper_bound.x, lower_bound.y, upper_bound.z };

    geo[9]  = glm::vec3{ lower_bound.x, lower_bound.y, lower_bound.z };
    geo[10] = glm::vec3{ upper_bound.x, lower_bound.y, upper_bound.z };
    geo[11] = glm::vec3{ lower_bound.x, lower_bound.y, upper_bound.z };

    // side 2
    geo[12] = glm::vec3{ lower_bound.x, lower_bound.y, lower_bound.z };
    geo[13] = glm::vec3{ lower_bound.x, upper_bound.y, upper_bound.z };
    geo[14] = glm::vec3{ lower_bound.x, upper_bound.y, lower_bound.z };

    geo[15] = glm::vec3{ lower_bound.x, lower_bound.y, lower_bound.z };
    geo[16] = glm::vec3{ lower_bound.x, lower_bound.y, upper_bound.z };
    geo[17] = glm::vec3{ lower_bound.x, upper_bound.y, upper_bound.z };

    // side 3
    geo[18] = glm::vec3{ upper_bound.x, upper_bound.y, upper_bound.z };
    geo[19] = glm::vec3{ lower_bound.x, upper_bound.y, upper_bound.z };
    geo[20] = glm::vec3{ lower_bound.x, lower_bound.y, upper_bound.z };

    geo[21] = glm::vec3{ upper_bound.x, upper_bound.y, upper_bound.z };
    geo[22] = glm::vec3{ lower_bound.x, lower_bound.y, upper_bound.z };
    geo[23] = glm::vec3{ upper_bound.x, lower_bound.y, upper_bound.z };

    // side 4
    geo[24] = glm::vec3{ upper_bound.x, upper_bound.y, upper_bound.z };
    geo[25] = glm::vec3{ lower_bound.x, upper_bound.y, lower_bound.z };
    geo[26] = glm::vec3{ lower_bound.x, upper_bound.y, upper_bound.z };

    geo[27] = glm::vec3{ upper_bound.x, upper_bound.y, upper_bound.z };
    geo[28] = glm::vec3{ upper_bound.x, upper_bound.y, lower_bound.z };
    geo[29] = glm::vec3{ lower_bound.x, upper_bound.y, lower_bound.z };

    // side 5
    geo[30] = glm::vec3{ upper_bound.x, upper_bound.y, upper_bound.z };
    geo[31] = glm::vec3{ upper_bound.x, lower_bound.y, upper_bound.z };
    geo[32] = glm::vec3{ upper_bound.x, lower_bound.y, lower_bound.z };

    geo[33] = glm::vec3{ upper_bound.x, upper_bound.y, upper_bound.z };
    geo[34] = glm::vec3{ upper_bound.x, lower_bound.y, lower_bound.z };
    geo[35] = glm::vec3{ upper_bound.x, upper_bound.y, lower_bound.z };
}

void prt3::insert_line_box(
    glm::vec3 const & lower_bound,
    glm::vec3 const & upper_bound,
    glm::vec3 * geo
) {
    // top
    geo[0] = glm::vec3{lower_bound.x, lower_bound.y, lower_bound.z};
    geo[1] = glm::vec3{lower_bound.x, lower_bound.y, upper_bound.z};

    geo[2] = glm::vec3{lower_bound.x, lower_bound.y, upper_bound.z};
    geo[3] = glm::vec3{upper_bound.x, lower_bound.y, upper_bound.z};

    geo[4] = glm::vec3{upper_bound.x, lower_bound.y, upper_bound.z};
    geo[5] = glm::vec3{upper_bound.x, lower_bound.y, lower_bound.z};

    geo[6] = glm::vec3{upper_bound.x, lower_bound.y, lower_bound.z};
    geo[7] = glm::vec3{lower_bound.x, lower_bound.y, lower_bound.z};

    // bottom
    geo[8] = glm::vec3{lower_bound.x, upper_bound.y, lower_bound.z};
    geo[9] = glm::vec3{lower_bound.x, upper_bound.y, upper_bound.z};

    geo[10] = glm::vec3{lower_bound.x, upper_bound.y, upper_bound.z};
    geo[11] = glm::vec3{upper_bound.x, upper_bound.y, upper_bound.z};

    geo[12] = glm::vec3{upper_bound.x, upper_bound.y, upper_bound.z};
    geo[13] = glm::vec3{upper_bound.x, upper_bound.y, lower_bound.z};

    geo[14] = glm::vec3{upper_bound.x, upper_bound.y, lower_bound.z};
    geo[15] = glm::vec3{lower_bound.x, upper_bound.y, lower_bound.z};

    // connections
    geo[16] = glm::vec3{lower_bound.x, lower_bound.y, lower_bound.z};
    geo[17] = glm::vec3{lower_bound.x, upper_bound.y, lower_bound.z};

    geo[18] = glm::vec3{lower_bound.x, lower_bound.y, upper_bound.z};
    geo[19] = glm::vec3{lower_bound.x, upper_bound.y, upper_bound.z};

    geo[20] = glm::vec3{upper_bound.x, lower_bound.y, upper_bound.z};
    geo[21] = glm::vec3{upper_bound.x, upper_bound.y, upper_bound.z};

    geo[22] = glm::vec3{upper_bound.x, lower_bound.y, lower_bound.z};
    geo[23] = glm::vec3{upper_bound.x, upper_bound.y, lower_bound.z};
}
