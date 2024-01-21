#include "texture_manager.h"

#include "texture.h"

#include "src/engine/core/context.h"

using namespace prt3;

TextureManager::TextureManager(Context & context)
: m_context{context} {}

ResourceID TextureManager::upload_texture(std::string const & path) {
    ResourceID res_id;

    if (m_path_to_resource_id.find(path) == m_path_to_resource_id.end()) {

        TextureData data;
        if (load_texture_data(path.c_str(), data)) {
            res_id = m_context.renderer().upload_texture(data);
            free_texture_data(data);
        } else {
            PRT3ERROR("failed to load texture at path \"%s\".\n", path.c_str());
            return NO_RESOURCE;
        }

        TextureRef & texture_ref = m_texture_refs[res_id];
        texture_ref.path = path;
        m_path_to_resource_id[path] = res_id;
    } else {
        res_id = m_path_to_resource_id.at(path);
    }

    ++m_texture_refs.at(res_id).ref_count;
    return res_id;
}

void TextureManager::clear() {
    for (auto const & pair : m_texture_refs) {
        m_context.renderer().free_texture(pair.first);
    }

    m_texture_refs.clear();
    m_path_to_resource_id.clear();
}

void TextureManager::free_texture_ref(ResourceID resource_id) {
    TextureRef & ref = m_texture_refs.at(resource_id);
    --ref.ref_count;

    if (ref.ref_count == 0) {
        m_context.renderer().free_texture(resource_id);
        m_path_to_resource_id.erase(ref.path);
        m_texture_refs.erase(resource_id);
    }
}
