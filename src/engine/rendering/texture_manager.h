#ifndef PRT3_TEXTURE_MANAGER_H
#define PRT3_TEXTURE_MANAGER_H

#include "src/engine/rendering/resources.h"

#include <vector>
#include <unordered_map>
#include <string>

namespace prt3 {

class Context;

class TextureManager {
private:
    struct TextureRef {
        std::string path;
        uint32_t ref_count;
    };
public:
    TextureManager(Context & context);

    ResourceID upload_texture(std::string const & path);
    void free_texture_ref(ResourceID resource_id);
    void clear();

    ResourceID get_texture_handle_from_path(std::string const & path) const
    { return m_path_to_resource_id.at(path); }

    std::string const & get_texture_path(ResourceID resource_id) const
    { return m_texture_refs.at(resource_id).path; }

    std::unordered_map<std::string, ResourceID> const & path_to_resource_id() const
    { return m_path_to_resource_id; }

private:
    Context & m_context;

    std::unordered_map<ResourceID, TextureRef> m_texture_refs;
    std::unordered_map<std::string, ResourceID> m_path_to_resource_id;

    friend class Scene;
};

} // namespace prt3

#endif // PRT3_TEXTURE_MANAGER_H