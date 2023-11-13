#include "game_gui.h"

#include "src/daedalus/game_state/game_state.h"
#include "src/util/serialization_util.h"

using namespace dds;

#define FONT_ATLAS_PATH "assets/gui/atlas.png" // hard-coded for now
#define FONT_ATLAS_META_DATA_PATH "assets/gui/atlas.fpmd" // hard-coded for now

GameGui::GameGui(prt3::Scene & scene) {
    std::ifstream in{FONT_ATLAS_META_DATA_PATH, std::ios::binary};
    if(in.fail()){
        return;
    }

    m_atlas.texture = scene.upload_texture(FONT_ATLAS_PATH);
    unsigned width, height, channels;
    scene.get_texture_metadata(m_atlas.texture, width, height, channels);
    glm::vec2 tex_dim{width, height};

    uint32_t n_atlases;
    prt3::read_stream(in, n_atlases);

    m_atlas.metadata.resize(n_atlases);

    for (uint32_t i = 0; i < n_atlases; ++i) {
        uint32_t font_name_len;
        prt3::read_stream(in, font_name_len);

        char dummy;
        while (font_name_len > 0) {
            prt3::read_stream(in, dummy);
            --font_name_len;
        }

        double font_size;
        prt3::read_stream(in, font_size);

        uint32_t n_glyphs;
        prt3::read_stream(in, n_glyphs);
        for (uint32_t gi = 0; gi < n_glyphs; ++gi) {
            uint32_t code_point;
            prt3::read_stream(in, code_point);
            unsigned char c = static_cast<unsigned char>(code_point);

            uint32_t x, y, w, h;
            int32_t oy;
            float a, l;
            prt3::read_stream(in, x);
            prt3::read_stream(in, y);
            prt3::read_stream(in, w);
            prt3::read_stream(in, h);
            prt3::read_stream(in, oy);
            prt3::read_stream(in, a);
            prt3::read_stream(in, l);
            glm::vec2 uv_origin = glm::vec2{x / tex_dim.x, y / tex_dim.y};
            glm::vec2 uv_dim = glm::vec2{w / tex_dim.x, h / tex_dim.y};
            glm::vec2 offset = glm::vec2{0.0f, -oy / font_size};
            float advance = a / font_size;
            float left_bearing = l / font_size;
            float ratio = float(w) / float(h);
            float norm_scale = h / font_size;

            prt3::FontChar & fc = m_atlas.metadata[i].char_data[c];
            fc.uv_origin = uv_origin;
            fc.uv_dimension = uv_dim;
            fc.offset = offset;
            fc.advance = advance;
            fc.left_bearing = left_bearing;
            fc.ratio = ratio;
            fc.norm_scale = norm_scale;
        }

        for (unsigned int c = 0; c < 256; ++c) {
            prt3::FontChar & fc = m_atlas.metadata[i].char_data[c];
            if (fc.uv_dimension.x == 0.0f) {
                // uv dim is zero, assume we have no data for this and replace
                // with '?'
                fc = m_atlas.metadata[i].char_data['?'];
            }
        }
    }
}

void GameGui::on_update(
    prt3::Scene & scene,
    prt3::NodeID canvas_id,
    GameState const & /*game_state*/
) {
    // TODO: implement
    prt3::Canvas & canvas = scene.get_component<prt3::Canvas>(canvas_id);

    prt3::CanvasNode cn;
    cn.dimension = glm::vec2{0.5f, 0.5f};
    cn.dimension_mode = prt3::CanvasNode::UnitType::relative;

    cn.position = glm::vec2{0.0f, 0.0f};
    cn.origin = glm::vec2{0.0f, 0.0f};
    cn.position_mode = prt3::CanvasNode::UnitType::relative;
    cn.origin_mode = prt3::CanvasNode::UnitType::relative;
    cn.parent_anchor = prt3::CanvasNode::AnchorPoint::mid;
    cn.center_point = prt3::CanvasNode::AnchorPoint::mid;

    cn.color = glm::vec4{0.0f, 0.0f, 0.0f, 0.5f};
    cn.inherit_color = false;
    cn.texture = m_atlas.texture;

    cn.mode = prt3::CanvasNode::Mode::rect;

    cn.u.rect.uv0 = glm::vec2{0.0f, 0.0f};
    cn.u.rect.uv1 = glm::vec2{0.0f, 0.0f};
    cn.u.rect.uv2 = glm::vec2{0.0f, 0.0f};
    cn.u.rect.uv3 = glm::vec2{0.0f, 0.0f};

    cn.layer = 0;

    canvas.begin_node(cn);

    prt3::CanvasNode text;
    text.dimension = glm::vec2{1.0f, 1.0f};
    text.dimension_mode = prt3::CanvasNode::UnitType::relative;

    text.position = glm::vec2{0.0f, 0.0f};
    text.origin = glm::vec2{0.0f, 0.0f};
    text.position_mode = prt3::CanvasNode::UnitType::relative;
    text.origin_mode = prt3::CanvasNode::UnitType::relative;
    text.parent_anchor = prt3::CanvasNode::AnchorPoint::mid;
    text.center_point = prt3::CanvasNode::AnchorPoint::mid;

    text.color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    text.inherit_color = false;
    text.texture = m_atlas.texture;

    text.mode = prt3::CanvasNode::Mode::text;

    text.u.text.char_info = m_atlas.metadata[0].char_data.data();
    text.u.text.font_size = 32;
    char const * lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing "\
                         "elit, sed do eiusmod tempor incididunt ut labore "\
                         "et dolore magna aliqua. Ut enim ad minim veniam, "\
                         "quis nostrud exercitation ullamco laboris nisi "\
                         "ut aliquip ex ea commodo consequat. Duis aute "\
                         "irure dolor in reprehenderit in voluptate velit "\
                         "esse cillum dolore eu fugiat nulla pariatur. "\
                         "Excepteur sint occaecat cupidatat non proident, "\
                         "sunt in culpa qui officia deserunt mollit anim id"\
                         " est laborum.";
    text.u.text.text = lorem;
    text.u.text.length = strlen(lorem);

    text.layer = 0;
    canvas.begin_node(text);
    canvas.end_node();

    canvas.end_node();
}
