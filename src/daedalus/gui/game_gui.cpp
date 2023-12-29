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

    m_atlas.texture = scene.upload_persistent_texture(FONT_ATLAS_PATH);
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

    if (n_atlases > 0) {
        prt3::FontChar const & fc0 = m_atlas.metadata[0].char_data[0];
        glm::vec2 uv = fc0.uv_origin + 0.5f * fc0.uv_dimension;
        m_atlas.uv_0xffff = uv;
    }
}

static float get_string_width(
    char const * str,
    prt3::FontChar const * font_data,
    float font_size
) {
    float width = 0.0f;
    while (*str) {
        unsigned char uc =
            *reinterpret_cast<unsigned char const *>(str);

        prt3::FontChar const & fc = font_data[uc];
        width += font_size * fc.advance;

        ++str;
    }
    return width;
}

void GameGui::display_interact(
    prt3::Canvas & canvas,
    Interactable const & interactable
) {
    float font_size = 32;
    float t_alpha = glm::clamp(6.0f * m_interactable_timer - 1.0f, 0.0f, 1.0f);

    char const * str = id_to_string(interactable.get_string_id());
    float str_width = get_string_width(
        str,
        m_atlas.metadata[0].char_data.data(),
        font_size
    );

    prt3::CanvasNode cn;
    cn.dimension = glm::vec2{str_width + 16.0f, font_size + 16.0f};
    cn.dimension_mode = prt3::CanvasNode::UnitType::absolute;

    cn.position = glm::vec2{0.0f, 16.0f};
    cn.origin = glm::vec2{0.0f, 0.0f};
    cn.position_mode = prt3::CanvasNode::UnitType::absolute;
    cn.origin_mode = prt3::CanvasNode::UnitType::relative;
    cn.parent_anchor = prt3::CanvasNode::AnchorPoint::bottom;
    cn.center_point = prt3::CanvasNode::AnchorPoint::bottom;

    cn.color = glm::vec4{0.0f, 0.0f, 0.0f, 0.5f * t_alpha};
    cn.inherit_color = false;
    cn.texture = m_atlas.texture;

    cn.mode = prt3::CanvasNode::Mode::rect;

    cn.u.rect.uv0 = m_atlas.uv_0xffff;
    cn.u.rect.uv1 = m_atlas.uv_0xffff;
    cn.u.rect.uv2 = m_atlas.uv_0xffff;
    cn.u.rect.uv3 = m_atlas.uv_0xffff;

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

    text.color = glm::vec4{1.0f, 1.0f, 1.0f, t_alpha};
    text.inherit_color = false;
    text.texture = m_atlas.texture;

    text.mode = prt3::CanvasNode::Mode::text;

    text.u.text.char_info = m_atlas.metadata[0].char_data.data();
    text.u.text.font_size = font_size;
    text.u.text.text = str;
    text.u.text.length = strlen(str);

    text.layer = 0;
    canvas.begin_node(text);
    canvas.end_node();

    canvas.end_node();
}

void GameGui::on_start(
    prt3::Scene & /* scene */,
    prt3::NodeID /* canvas_id */,
    GameState const & /* game_state */
) {
    m_time_since_start = 0.0f;
}

void GameGui::on_update(
    prt3::Scene & scene,
    prt3::NodeID canvas_id,
    GameState const & game_state,
    float delta_time
) {
    prt3::Canvas & canvas = scene.get_component<prt3::Canvas>(canvas_id);

    /* interactable */
    if (m_prev_interactable != game_state.interactable()) {
        m_interactable_timer = 0.0f;
    }

    m_prev_interactable = game_state.interactable();
    if (game_state.interactable() != nullptr) {
        m_interactable_timer += delta_time;
        display_interact(canvas, *game_state.interactable());
    }

    if (m_time_since_start < m_room_name_time) {
        display_room_name(
            canvas,
            game_state.map().room_name(game_state.current_room())
        );
    }

    m_time_since_start += delta_time;
}

void GameGui::free_resources(prt3::Scene & scene) {
    if (m_atlas.texture != prt3::NO_RESOURCE) {
        scene.free_persistent_texture(m_atlas.texture);
    }
}

void GameGui::display_room_name(
    prt3::Canvas & canvas,
    char const * room_name
) {
    float font_size = 24;
    float t_alpha;
    if (m_time_since_start < m_room_name_time * 0.333f) {
        t_alpha = m_time_since_start / (m_room_name_time * 0.333f);
    } else if (m_time_since_start < m_room_name_time * 0.667f) {
        t_alpha = 1.0f;
    } else {
        t_alpha = 1.0f - glm::min(
            (m_time_since_start - 0.667f * m_room_name_time) /
                (m_room_name_time * 0.333f),
            1.0f
        );
    }

    char const * str = room_name;
    float str_width = get_string_width(
        str,
        m_atlas.metadata[0].char_data.data(),
        font_size
    );

    prt3::CanvasNode text;
    text.dimension = glm::vec2{str_width, font_size};
    text.dimension_mode = prt3::CanvasNode::UnitType::absolute;

    text.position = glm::vec2{-8.0f, 4.0f};
    text.origin = glm::vec2{0.0f, 0.0f};
    text.position_mode = prt3::CanvasNode::UnitType::absolute;
    text.origin_mode = prt3::CanvasNode::UnitType::relative;
    text.parent_anchor = prt3::CanvasNode::AnchorPoint::bottom_right;
    text.center_point = prt3::CanvasNode::AnchorPoint::bottom_right;

    text.color = glm::vec4{1.0f, 1.0f, 1.0f, t_alpha};
    text.inherit_color = false;
    text.texture = m_atlas.texture;

    text.mode = prt3::CanvasNode::Mode::text;

    text.u.text.char_info = m_atlas.metadata[0].char_data.data();
    text.u.text.font_size = font_size;
    text.u.text.text = str;
    text.u.text.length = strlen(str);

    text.layer = 0;

    canvas.begin_node(text);
    canvas.end_node();
}
