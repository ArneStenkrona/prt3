#include "libschrift/schrift.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#include <stdio.h>
#include <string.h>

static void blit_image(
    SFT_Image src,
    char * dest,
    int dest_x,
    int dest_y,
    int dest_width
) {
    int pi = 0;
    for (int y = 0; y < src.height; ++y) {
        for (int x = 0; x < src.width; ++x) {
            int curr_x = dest_x + x;
            int curr_y = dest_y + y;

            int data_idx = curr_x + curr_y * dest_width;
            dest[data_idx] = ((char *)src.pixels)[pi];
            ++pi;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: \n%s <ttf path> <font size>\n", argv[0]);
        return 1;
    }

    char const * ttf_path = argv[1];
    SFT_Font * font = sft_loadfile(ttf_path);

    double size;
    sscanf(argv[2], "%lf", &size);

    SFT sft;
    sft.font = font;
    sft.xScale = size;
    sft.yScale = size;
    sft.xOffset = 0;
    sft.yOffset = 0;
    sft.flags = SFT_DOWNWARD_Y;

    SFT_Image images[255];

    int total_width = 0;
    int total_height = 0;

    /* render glyphs */
    for (unsigned char c = 0; c < 255; ++c) {
        SFT_UChar uc = (SFT_UChar)c;
        SFT_Glyph glyph;
        sft_lookup(&sft, uc, &glyph);

        SFT_GMetrics metrics;
        sft_gmetrics(&sft, glyph, &metrics);

        images[c].width = metrics.minWidth;
        images[c].height = metrics.minHeight;
        images[c].pixels = malloc(images[c].width * images[c].height);

        sft_render(&sft, glyph, images[c]);

        total_width += images[c].width;
        total_height =
            images[c].height > total_height ? images[c].height : total_height;
    }

    /* write glyphs to atlas */
    void * atlas_image = malloc(total_width * total_height);
    memset(atlas_image, 0, total_width * total_height);

    int curr_x = 0;
    for (unsigned char c = 0; c < 255; ++c) {
        blit_image(
            images[c],
            atlas_image,
            curr_x,
            0,
            total_width
        );
        curr_x += images[c].width;
    }

#define BUF_SIZE 255
    char font_name[BUF_SIZE];
    char const * ttf_name = strrchr(ttf_path, '/');
    ttf_name = ttf_name ? ttf_name + 1 : ttf_path;
    strncpy(font_name, ttf_name, BUF_SIZE - 1);
    char * dot = (char *)strrchr(font_name, '.');
    if (dot) dot[0] = '\0';

    char file_name[BUF_SIZE] = {0};
    snprintf(file_name, BUF_SIZE, "atlas_%s.bmp", font_name);

    printf("file_name: %s\n", file_name);

    stbi_write_bmp(file_name, total_width, total_height, 1, atlas_image);

    /* free memory */
    for (unsigned char c = 0; c < 255; ++c) {
        free(images[c].pixels);
    }

    sft_freefont(font);

    return 0;
}
