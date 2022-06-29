#ifndef PRT3_ENGINE_H
#define PRT3_ENGINE_H

#include "src/engine/core/context.h"

namespace prt3
{

class Engine {
public:
    Engine();

    void run();
private:
    Context m_context;

    bool m_exit;
};

}

#endif
