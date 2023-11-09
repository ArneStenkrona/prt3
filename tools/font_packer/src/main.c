#include "libschrift/schrift.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#include <math.h>
#include <stdint.h>
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

void validate_char_set_has_no_duplicates(
    unsigned char const * char_set,
    unsigned size
) {
    int found[256] = {0};
    for (unsigned i = 0; i < size; ++i) {
        if (found[char_set[i]]) {
            fprintf(
                stderr,
                "Found duplicate char \'%c\' in char_set.\n",
                char_set[i]
            );
            exit(1);
        }

        found[char_set[i]] = 1;
    }
}

static void rasterize_ttf(
    char const * path,
    double size,
    unsigned char const * char_set,
    unsigned char_set_size
) {
    validate_char_set_has_no_duplicates(char_set, char_set_size);

    SFT_Font * font = sft_loadfile(path);

    SFT sft;
    sft.font = font;
    sft.xScale = size;
    sft.yScale = size;
    sft.xOffset = 0;
    sft.yOffset = 0;
    sft.flags = SFT_DOWNWARD_Y;

    SFT_Image images[256];
    SFT_GMetrics metrics[256];

    int padding = 1;

    /* render glyphs */
    for (unsigned i = 0; i < char_set_size; ++i) {
        int c = (int)char_set[i];
        SFT_UChar uc = (SFT_UChar)c;
        SFT_Glyph glyph;
        sft_lookup(&sft, uc, &glyph);

        sft_gmetrics(&sft, glyph, &metrics[c]);

        images[c].width = metrics[c].minWidth + padding;
        images[c].height = metrics[c].minHeight + padding;
        images[c].pixels = malloc(images[c].width * images[c].height);

        sft_render(&sft, glyph, images[c]);
    }

    /* we pack the glyphs into a square so we calculate the side length of such
     * a square.
     */
    int n_cols = (int)ceilf(sqrtf((float)char_set_size));

    int total_width = 0;
    int curr_width = 0;
    for (unsigned i = 0; i < char_set_size; ++i) {
        int c = (int)char_set[i];
        curr_width += images[c].width;

        if (i + 1 == char_set_size || (i + 1) % n_cols == 0) {
            total_width = curr_width > total_width ? curr_width : total_width;
            curr_width = 0;
        }

    }

    curr_width = 0;
    int row_height = 0;
    int total_height = 0;
    for (unsigned i = 0; i < char_set_size; ++i) {
        int c = (int)char_set[i];
        curr_width += images[c].width;

        if (i + 1 == char_set_size || (i + 1) % n_cols == 0) {
            total_height += row_height;
            row_height = 0;
            curr_width = 0;
        }

        row_height =
            images[c].height > row_height ? images[c].height : row_height;
    }

    /* write glyphs to atlas */
    void * atlas_image = malloc(total_width * total_height);
    memset(atlas_image, 0, total_width * total_height);

    int curr_x = 0;
    int curr_y = 0;
    row_height = 0;
    curr_width = 0;
    for (unsigned i = 0; i < char_set_size; ++i) {
        int c = (int)char_set[i];
        blit_image(
            images[c],
            atlas_image,
            curr_x,
            curr_y,
            total_width
        );

        curr_x += images[c].width;

        if ((i + 1) % n_cols == 0) {
            curr_x = 0;
            curr_y += row_height;
            row_height = 0;
        }

        row_height =
            images[c].height > row_height ? images[c].height : row_height;
    }

#define BUF_SIZE 256
    char font_name[BUF_SIZE];
    char const * ttf_name = strrchr(path, '/');
    ttf_name = ttf_name ? ttf_name + 1 : path;
    strncpy(font_name, ttf_name, BUF_SIZE - 1);
    char * dot = (char *)strrchr(font_name, '.');
    if (dot) dot[0] = '\0';

    char file_name[BUF_SIZE] = {0};
    snprintf(file_name, BUF_SIZE, "atlas_%s.bmp", font_name);

    stbi_write_bmp(file_name, total_width, total_height, 1, atlas_image);
    printf("wrote atlas to: %s\n", file_name);

    /* free memory */
    for (unsigned i = 0; i < char_set_size; ++i) {
        int c = (int)char_set[i];
        free(images[c].pixels);
    }

    sft_freefont(font);
}

#define DEFAULT_CHAR_SET_SIZE 94
unsigned char default_char_set[DEFAULT_CHAR_SET_SIZE] = {
    "!\"#$%&'()*+,-./0123456789:;<=>?"\
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
    "[\\]^_`"\
    "abcdefghijklmnopqrstuvwxyz"\
    "{|}~"
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: \n%s <ttf path> <font size>\n", argv[0]);
        return 1;
    }

    char const * ttf_path = argv[1];
    double size;
    sscanf(argv[2], "%lf", &size);

    rasterize_ttf(ttf_path, size, default_char_set, DEFAULT_CHAR_SET_SIZE);

    return 0;
}
