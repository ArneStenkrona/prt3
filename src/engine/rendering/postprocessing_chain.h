#ifndef PRT3_POSTPROCESSING_CHAIN
#define PRT3_POSTPROCESSING_CHAIN

#include "src/engine/rendering/postprocessing_pass.h"

#include <vector>

namespace prt3 {

struct PostProcessingChain {
    std::vector<PostProcessingPass> passes;
};

} // namespace prt3

#endif
