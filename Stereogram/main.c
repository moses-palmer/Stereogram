#include <stdio.h>

#include <effect.h>
#include <stereo.h>

#include <arguments.h>

/**
 * Calculates the mean of the colour values for every pixel and sets all colour
 * values to that.
 *
 * @param depth_pattern
 *     The depth pattern to modify.
 */
static void
luminance_apply(StereoPattern *depth_pattern)
{
    int x, y;

    for (y = 0; y < depth_pattern->height; y++) {
        PatternPixel *d = stereo_pattern_row_get(depth_pattern, y);

        for (x = 0; x < depth_pattern->width; x++) {
            memset(d, (d->r + d->g + d->b) / 3, 3);
            d++;
        }
    }
}

static int
start(int argc, char *argv[],
    FILE* in_file,
    FILE* out_file,
    StereoPattern* pattern,
    double strength,
    int invert_depth,
    int channel)
{
    StereoPattern *depth_pattern;
    int result = 0;

    result++;

    /* First read the input PNG */
    depth_pattern = stereo_pattern_create_from_png(in_file);
    if (!depth_pattern) {
        fprintf(stderr, "Failed to read the depth image.");
    }
    else {
        ZBuffer *zbuffer;
        StereoImage *stereogram;

        result++;

        /* If luminance is specified, we need to calculate the luminance of
           every pixel */
        if (channel < 0) {
            luminance_apply(depth_pattern);
            channel = 0;
        }

        /* Create the z-buffer; this cannot fail */
        zbuffer = stereo_zbuffer_create_from_pattern(depth_pattern);

        /* Create a stereogram image with the same dimensions */
        stereogram = stereo_image_create(
            depth_pattern->width, depth_pattern->height,
            pattern, strength, invert_depth);
        if (!stereogram) {
            fprintf(stderr, "Failed to created the stereogram buffer.");
        }
        else {
            result++;

            /* Apply the stereogram effect */
            if (!stereo_image_apply(stereogram, zbuffer, channel)) {
                fprintf(stderr, "Failed to apply the stereogram effect.");
            }
            else {
                result++;

                /* Store the stereogram to the output file */
                if (!stereo_pattern_save_to_png(stereogram->image, out_file)) {
                    fprintf(stderr, "Failed to save the stereogram to the "
                        "output file.");
                }
                else {
                    result = 0;
                }
            }

            stereo_image_free(stereogram);
        }

        stereo_zbuffer_free(zbuffer);
        stereo_pattern_free(depth_pattern);
    }

    return result;
}
