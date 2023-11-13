#include "libschrift/schrift.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void blit_image(
    char * src,
    int src_width,
    int src_height,
    char * dest,
    int dest_x,
    int dest_y,
    int dest_width
) {
    int pi = 0;
    for (int y = 0; y < src_height; ++y) {
        for (int x = 0; x < src_width; ++x) {
            int curr_x = dest_x + x;
            int curr_y = dest_y + y;

            int data_idx = curr_x + curr_y * dest_width;
            dest[data_idx] = src[pi];
            ++pi;
        }
    }
}

static void validate_char_set_has_no_duplicates(
    unsigned char const * char_set,
    unsigned size
) {
    int found[256] = {0};
    for (unsigned i = 0; i < size; ++i) {
        if (found[char_set[i]]) {
            fprintf(
                stderr,
                "ERROR: Found duplicate char \'%c\' in char_set.\n",
                char_set[i]
            );
            exit(1);
        }

        found[char_set[i]] = 1;
    }
}

typedef struct {
    unsigned pos_x, pos_y;
    unsigned width, height;
    int offset_y;
    float advance;
    float left_bearing;
} Glyph_Metadata;

typedef struct {
    char * pixels;
    int width;
    int height;

    Glyph_Metadata metadata[256];
} Atlas_Image;

static void rasterize_ttf(
    char const * path,
    double size,
    unsigned char const * char_set,
    unsigned char_set_size,
    Atlas_Image * out_image
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

    int max_glyph_height = 0;

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

        max_glyph_height = images[c].height > max_glyph_height ?
                                                images[c].height :
                                                max_glyph_height;

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

    memset(out_image->metadata, 0, 256);

    for (unsigned i = 0; i < char_set_size; ++i) {
        int c = (int)char_set[i];
        blit_image(
            images[c].pixels,
            images[c].width,
            images[c].height,
            atlas_image,
            curr_x,
            curr_y,
            total_width
        );

        out_image->metadata[c].pos_x = curr_x;
        out_image->metadata[c].pos_y = curr_y;
        out_image->metadata[c].width = images[c].width;
        out_image->metadata[c].height = images[c].height;
        out_image->metadata[c].offset_y = metrics[c].yOffset;
        out_image->metadata[c].advance = (float)metrics[c].advanceWidth;
        out_image->metadata[c].left_bearing = (float)metrics[c].leftSideBearing;

        curr_x += images[c].width;

        if ((i + 1) % n_cols == 0) {
            curr_x = 0;
            curr_y += row_height;
            row_height = 0;
        }

        row_height =
            images[c].height > row_height ? images[c].height : row_height;
    }

    out_image->pixels = atlas_image;
    out_image->width = total_width;
    out_image->height = total_height;

    /* free memory */
    for (unsigned i = 0; i < char_set_size; ++i) {
        int c = (int)char_set[i];
        free(images[c].pixels);
    }

    sft_freefont(font);
}

#define DEFAULT_CHAR_SET_SIZE 95
unsigned char default_char_set[DEFAULT_CHAR_SET_SIZE] = {
    " !\"#$%&'()*+,-./0123456789:;<=>?"\
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
    "[\\]^_`"\
    "abcdefghijklmnopqrstuvwxyz"\
    "{|}~"
};


static void print_usage(FILE * stream, char const * bin_name) {
    char const * usage_string = \
        "usage: %s file1 size1 [file2 size2 ...] [-o out-prefix]\n"\
        "  fileN: path to TrueType font file\n"
        "  sizeN: desired font size for font in fileN\n"
        "  -o out-prefix: image and metadata output prefix (default: atlas)\n";

    fprintf(stream, usage_string, bin_name);
}

typedef struct {
    char const * file;
    double size;
} TTF_Arg;

typedef struct {
    TTF_Arg * ttf_args;
    unsigned n_ttf_args;
    char const * out_prefix;
} FP_Args;

static int parse_args(int argc, char * argv[], FP_Args * args) {
    if (argc < 3) {
        return 0;
    }

    int out_index = -1;

    args->n_ttf_args = 0;
    args->ttf_args = NULL;

    /* First, we validate */
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 != argc) {
                fprintf(stderr, "ERROR: outfile should be specified last.\n");
                return 0;
            }

            if (out_index != -1) {
                fprintf(stderr, "ERROR: There can be only one outfile.\n");
                return 0;
            }

            out_index = i;
        } else if (i % 2 == 0) {
            double size;
            sscanf(argv[i], "%lf", &size);
            if (size <= 0.0) {
                fprintf(stderr, "ERROR: invalid size \"%s\".\n", argv[i]);
            }

            ++args->n_ttf_args;
        }
    }

    /* (file, size) pairs implies even number of args */
    if ((argc - 1) % 2 != 0) {
        return 0;
    }

    /* After validation, we extract the arguments */

    args->ttf_args = calloc(args->n_ttf_args, sizeof(TTF_Arg));

    unsigned ttf_i = 0;
    for (int i = 1; i < argc; ++i) {
        if (i == out_index) break;

        if (i % 2 == 1) {
            args->ttf_args[ttf_i].file = argv[i];
        } else {
            sscanf(argv[i], "%lf", &args->ttf_args[ttf_i].size);
            ++ttf_i;
        }
    }

    if (out_index == -1) {
        args->out_prefix = "atlas";
    } else {
        args->out_prefix = argv[out_index];
    }

    return 1;
}

static void free_args(FP_Args * args) {
    free(args->ttf_args);
}

static void write_atlas(
    Atlas_Image * images,
    unsigned n,
    char const * out_prefix
) {
    char * pixels;
    int w = 0;
    int h = 0;

    for (unsigned i = 0; i < n; ++i) {
        w += images[i].width;
        h = images[i].height > h ? images[i].height : h;
    }

    pixels = malloc(w * h);
    pixels = memset(pixels, 0, w * h);

    int curr_x = 0;
    for (unsigned i = 0; i < n; ++i) {
        blit_image(
            images[i].pixels,
            images[i].width,
            images[i].height,
            pixels,
            curr_x,
            0,
            w
        );

        /* update metadata now that it is in the combined atlas */
        for (unsigned c = 0; c < 256; ++c) {
            images[i].metadata[c].pos_x += curr_x;
        }

        curr_x += images[i].width;
    }

    char outfile[256] = {0};
    snprintf(outfile, 256, "%s.png", out_prefix);

    stbi_write_png(outfile, w, h, 1, pixels, 0);
    free(pixels);
}

static void write_metadata(
    Atlas_Image * images,
    TTF_Arg * args,
    unsigned n,
    char const * out_prefix
) {
    char outfile[256] = {0};
    snprintf(outfile, 256, "%s.fpmd", out_prefix);

    FILE * out = fopen(outfile, "wb");

    /* n fonts */
    uint32_t n_atlases = (uint32_t)n;
    fwrite(&n_atlases, sizeof(n_atlases), 1, out);

    for (unsigned i = 0; i < n; ++i) {
        /* font name */
        char const * path = args[i].file;

        char font_name[256] = {0};
        char const * ttf_name = strrchr(path, '/');
        ttf_name = ttf_name ? ttf_name + 1 : path;
        strncpy(font_name, ttf_name, 256);
        char * dot = (char *)strrchr(font_name, '.');
        if (dot) dot[0] = '\0';

        uint32_t font_name_len = (uint32_t)strlen(font_name);
        fwrite(&font_name_len, sizeof(font_name_len), 1, out);
        fwrite(font_name, font_name_len, 1, out);

        /* font size */
        fwrite(&args[i].size, sizeof(args[i].size), 1, out);

        /* n glyphs */
        uint32_t n_glyphs = 0;
        for (unsigned c = 0; c < 256; ++c) {
            Glyph_Metadata md = images[i].metadata[c];
            if (md.width == 0) continue;
            ++n_glyphs;
        }
        fwrite(&n_glyphs, sizeof(n_glyphs), 1, out);

        /* metadata */
        for (unsigned c = 0; c < 256; ++c) {
            Glyph_Metadata md = images[i].metadata[c];
            if (md.width == 0) continue;
            uint32_t code_point = (uint32_t)c;
            fwrite(&code_point, sizeof(code_point), 1, out);
            uint32_t x = (uint32_t)md.pos_x;
            uint32_t y = (uint32_t)md.pos_y;
            uint32_t w = (uint32_t)md.width;
            uint32_t h = (uint32_t)md.height;
            int32_t oy = (int32_t)md.offset_y;
            float a = md.advance;
            float l = md.left_bearing;

            fwrite(&x, sizeof(x), 1, out);
            fwrite(&y, sizeof(y), 1, out);
            fwrite(&w, sizeof(w), 1, out);
            fwrite(&h, sizeof(h), 1, out);
            fwrite(&oy, sizeof(oy), 1, out);
            fwrite(&a, sizeof(a), 1, out);
            fwrite(&l, sizeof(l), 1, out);
        }
    }

    fclose(out);
}

int main(int argc, char * argv[]) {
    FP_Args args;

    if (!parse_args(argc, argv, &args)) {
        print_usage(stderr, argv[0]);
        return 1;
    }

    Atlas_Image * images = calloc(args.n_ttf_args, sizeof(Atlas_Image));

    for (unsigned i = 0; i < args.n_ttf_args; ++i) {
        char const * ttf_path = args.ttf_args[i].file;
        double size = args.ttf_args[i].size;

        rasterize_ttf(
            ttf_path,
            size,
            default_char_set,
            DEFAULT_CHAR_SET_SIZE,
            &images[i]
        );
    }

    write_atlas(images, args.n_ttf_args, args.out_prefix);
    write_metadata(images, args.ttf_args, args.n_ttf_args, args.out_prefix);

    for (unsigned i = 0; i < args.n_ttf_args; ++i) {
        free(images[i].pixels);
    }

    free_args(&args);

    return 0;
}
