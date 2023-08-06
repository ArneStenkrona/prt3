#ifndef PRT3_TEXTURE_H

namespace prt3 {

struct TextureData {
    int width;
    int height;
    int channels;
    unsigned char * data;
};

bool load_texture_data(char const * path, TextureData & data);
void free_texture_data(TextureData & data);
void free_texture_data(unsigned char * data);

} // namespace prt3

#define PRT3_TEXTURE_H
#endif // PRT3_TEXTURE_H
