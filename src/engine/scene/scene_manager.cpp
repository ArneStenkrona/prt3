#include "scene_manager.h"

#include "src/engine/core/context.h"

using namespace prt3;

SceneManager::SceneManager(Context & /*context*/)
 /*: m_context{context}*/ {}

void SceneManager::queue_scene(std::string const & path) {
    m_queued_scene_path = path;
}
