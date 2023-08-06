#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace prt3;

bool prt3::load_texture_data(char const * path, TextureData & data) {
    data.data = stbi_load(
        path,
        &data.width,
        &data.height,
        &data.channels,
        0
    );
    return data.data != 0;
}

void prt3::free_texture_data(TextureData & data) {
    data.width = 0;
    data.height = 0;
    data.channels = 0;
    data.data = nullptr;
}

void prt3::free_texture_data(unsigned char * data) {
    stbi_image_free(data);
}
