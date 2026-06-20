
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "gpu_port.h"
#include "gpu_common.h"

#define PRINT_MATRX   0
#define RAD(d)        (d*3.1415926/180.0)

static int fb_width  = GPU_FB_WIDTH;
static int fb_height = GPU_FB_HEIGHT;//4

static vg_lite_buffer_t buffer;     // offscreen framebuffer object for rendering.
static vg_lite_buffer_t *sys_fb;   // system framebuffer object to show the rendering result.
static vg_lite_buffer_t *fb;
static vg_lite_buffer_t raw;
static int has_fb = 0;

static vg_lite_buffer_t image0, image1, image2, image3, image4, image5;

typedef struct VertexRec
{
    float x;
    float y;
    float z;
} Vertex_t;

typedef struct NormalRec
{
    float x;
    float y;
    float z;
} Normal_t;

Vertex_t cube_v0 = {-1.0, -1.0, -1.0};
Vertex_t cube_v1 = {1.0, -1.0, -1.0};
Vertex_t cube_v2 = {1.0, 1.0, -1.0};
Vertex_t cube_v3 = {-1.0, 1.0, -1.0};
Vertex_t cube_v4 = {-1.0, -1.0, 1.0};
Vertex_t cube_v5 = {1.0, -1.0, 1.0};
Vertex_t cube_v6 = {1.0, 1.0, 1.0};
Vertex_t cube_v7 = {-1.0, 1.0, 1.0};

Normal_t normal0321 = {0.0, 0.0, -1.0};
Normal_t normal4567 = {0.0, 0.0, 1.0};
Normal_t normal1265 = {1.0, 0.0, 0.0};
Normal_t normal0473 = {-1.0, 0.0, 0.0};
Normal_t normal2376 = {0.0, 1.0, 0.0};
Normal_t normal0154 = {0.0, -1.0, 0.0};

void scale_cube(Vertex_t *vertex, float scale)
{
    // Scale cube vertex coordinates to proper size.
    //
    vertex->x *= scale;
    vertex->y *= scale;
    vertex->z *= scale;
}

void compute_rotate(float rx, float ry, float rz, vg_lite_matrix_t *rotate)
{
    // Rotation angles rx, ry, rz (degree) for axis X, Y, Z
    // Compute 3D rotation matrix base on rotation angle rx, ry, rz about axis X, Y, Z.
    //
    rotate->m[0][0] = cos(RAD(rz)) * cos(RAD(ry));
    rotate->m[0][1] = cos(RAD(rz)) * sin(RAD(ry)) * sin(RAD(rx)) - sin(RAD(rz)) * cos(RAD(rx));
    rotate->m[0][2] = cos(RAD(rz)) * sin(RAD(ry)) * cos(RAD(rx)) + sin(RAD(rz)) * sin(RAD(rx));
    rotate->m[1][0] = sin(RAD(rz)) * cos(RAD(ry));
    rotate->m[1][1] = sin(RAD(rz)) * sin(RAD(ry)) * sin(RAD(rx)) + cos(RAD(rz)) * cos(RAD(rx));
    rotate->m[1][2] = sin(RAD(rz)) * sin(RAD(ry)) * cos(RAD(rx)) - cos(RAD(rz)) * sin(RAD(rx));
    rotate->m[2][0] = -sin(RAD(ry));
    rotate->m[2][1] = cos(RAD(ry)) * sin(RAD(rx));
    rotate->m[2][2] = cos(RAD(ry)) * cos(RAD(rx));
}

void transfrom_rotate(vg_lite_matrix_t *rotate, Vertex_t *vertex, Vertex_t *rc, float tx, float ty)
{
    // Compute the new cube vertex coordinates transformed by the rotation matrix.
    //
    rc->x = rotate->m[0][0] * vertex->x + rotate->m[0][1] * vertex->y + rotate->m[0][2] * vertex->z;
    rc->y = rotate->m[1][0] * vertex->x + rotate->m[1][1] * vertex->y + rotate->m[1][2] * vertex->z;
    rc->z = rotate->m[2][0] * vertex->x + rotate->m[2][1] * vertex->y + rotate->m[2][2] * vertex->z;

    // Translate the vertex in XY plane.
    //
    rc->x += tx;
    rc->y += ty;
}

void transfrom_normalZ(vg_lite_matrix_t *rotate, Normal_t *nVec, float *nZ)
{
    // Compute the new normal Z coordinate transformed by the rotation matrix.
    //
    *nZ = rotate->m[2][0] * nVec->x + rotate->m[2][1] * nVec->y + rotate->m[2][2] * nVec->z;
}

// From 6.5.2 of OpenVG1.1 Spec: An affine transformation maps a point (x, y) into the point
// (x*sx + y*shx + tx, x*shy + y*sy + ty) using homogeneous matrix multiplication.
// To map a rectangle image (0,0),(w,0),(w,h),(0,h) to a parallelogram (x0,y0),(x1,y1),(x2,y2),(x3,y3).
// We get the following equations:
//
//     x0 = tx;
//     y0 = ty;
//     x1 = w*sx + tx;
//     y1 = w*shy + ty;
//     x3 = h*shx + tx;
//     y3 = h*sy + ty;
//
// So the homogeneous matrix sx, sy, shx, shy, tx, ty can be easily solved from above equations.
//
void transfrom_blit(float w, float h, Vertex_t *v0, Vertex_t *v1, Vertex_t *v2, Vertex_t *v3, vg_lite_matrix_t *matrix)
{
    float sx, sy, shx, shy, tx, ty;

    // Compute 3x3 image transform matrix to map a rectangle image (w,h) to
    // a parallelogram (x0,y0), (x1,y1), (x2,y2), (x3,y3) counterclock wise.
    //
    sx = (v1->x - v0->x) / w;
    sy = (v3->y - v0->y) / h;
    shx = (v3->x - v0->x) / h;
    shy = (v1->y - v0->y) / w;
    tx = v0->x;
    ty = v0->y;

    // Set the Blit transformation matrix
    matrix->m[0][0] = sx;
    matrix->m[0][1] = shx;
    matrix->m[0][2] = tx;
    matrix->m[1][0] = shy;
    matrix->m[1][1] = sy;
    matrix->m[1][2] = ty;
    matrix->m[2][0] = 0.0;
    matrix->m[2][1] = 0.0;
    matrix->m[2][2] = 1.0;
}

void print_matrix(vg_lite_matrix_t *matrix)
{
#if PRINT_MATRX
    printf("Blit Matrix:\n");
    printf("    %f    %f    %f \n", matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);
    printf("    %f    %f    %f \n", matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);
    printf("    %f    %f    %f \n", matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);
#endif
}

static void cleanup(void)
{
    if (has_fb) {
        // Close the framebuffer.
        vg_lite_fb_close(sys_fb);
    }

    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    if (image0.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image0);
    }
    if (image1.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image1);
    }
    if (image2.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image2);
    }
    if (image3.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image3);
    }
    if (image4.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image4);
    }
    if (image5.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image5);
    }

    if (raw.handle != NULL) {
        // Free the raw memory.
        vg_lite_free(&raw);
    }

    vg_lite_close();
}

//int main(int argc, const char * argv[])
int ui_cube_main_entry(int argc, const char * argv[])
{
    uint32_t feature_check = 0;
    uint32_t frames = 500;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix, rotate_3D, ident;
    Vertex_t rv0, rv1, rv2, rv3, rv4, rv5, rv6, rv7;
    float nz0321, nz4567, nz5126, nz0473, nz7623, nz0154;
    float cbsize, xoff, yoff, xrot, yrot, zrot, rotstep;
    // Initialize vg_lite engine.
    vg_lite_error_t error = vg_lite_init(fb_width, fb_height);

    if (error) {
        printf("vg_lite engine init failed: vg_lite_blit_init() returned error %d\n", error);
        cleanup();
        return -1;
    }

    vg_lite_identity(&ident);

    /* Set image filter type. */
    filter = VG_LITE_FILTER_POINT;

    // Load the image0.
    if (vg_lite_load_png(&image0, "tiger_blue.png") != 1) {
        printf("load png file error\n");
        cleanup();
        return -1;
    }
    /* Load the image1. */
    if (vg_lite_load_png(&image1, "tiger_grey.png") != 1) {
        printf("load png file error\n");
        cleanup();
        return -1;
    }

    // Load the image2.
    if (vg_lite_load_png(&image2, "tiger_lime.png") != 1) {
        printf("load png file error\n");
        cleanup();
        return -1;
    }

    // Load the image3.
    if (vg_lite_load_png(&image3, "tiger_laven.png") != 1) {
        printf("load png file error\n");
        cleanup();
        return -1;
    }

    // Load the image4.
    if (vg_lite_load_png(&image4, "tiger_turk.png") != 1) {
        printf("load png file error\n");
        cleanup();
        return -1;
    }

    // Load the image5.
    if (vg_lite_load_png(&image5, "tiger_yellow.png") != 1) {
        printf("load png file error\n");
        cleanup();
        return -1;
    }
    
    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
   // buffer.format = VG_LITE_RGB565;
    buffer.format = VG_LITE_BGR565;

    error = vg_lite_allocate(&buffer);
    if (error) {
     //   printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;
    // Scale the cube to proper size
    cbsize = fb_height / 6.0;
    scale_cube(&cube_v0, cbsize);
    scale_cube(&cube_v1, cbsize);
    scale_cube(&cube_v2, cbsize);
    scale_cube(&cube_v3, cbsize);
    scale_cube(&cube_v4, cbsize);
    scale_cube(&cube_v5, cbsize);
    scale_cube(&cube_v6, cbsize);
    scale_cube(&cube_v7, cbsize);

    // Translate the cube to the center of framebuffer.
    xoff = (fb_width - cbsize) / 2.0;
    yoff = (fb_height - cbsize) / 2.0;

    // Set the intial cube rotation degree and step.
    xrot = 20.0;
    yrot = 0.0;
    zrot = 20.0;
    rotstep = 5.0;

// Loop the rotating cube for number of frames
//
//while (frames--)
while (1)
{
    // Clear the buffer with black color.
    if (frames-- == 0){


    //Vertex_t rv0, rv1, rv2, rv3, rv4, rv5, rv6, rv7;
    memset(&rv0, 0 , sizeof(Vertex_t));
    memset(&rv1, 0 , sizeof(Vertex_t));
    memset(&rv2, 0 , sizeof(Vertex_t));
    memset(&rv3, 0 , sizeof(Vertex_t));
    memset(&rv4, 0 , sizeof(Vertex_t));
    memset(&rv5, 0 , sizeof(Vertex_t));
    memset(&rv6, 0 , sizeof(Vertex_t));
    memset(&rv7, 0 , sizeof(Vertex_t));

    nz0321 = 0, nz4567 = 0, nz5126 = 0, nz0473 = 0, nz7623 =0, nz0154 = 0;

    // Set the intial cube rotation degree and step.
    xrot = 20.0;
    yrot = 0.0;
    zrot = 20.0;
    rotstep = 5.0;
    frames = 500;
    memset(&rotate_3D, 0, sizeof(vg_lite_matrix_t));
    memset(&matrix, 0, sizeof(vg_lite_matrix_t));
    //vg_lite_matrix_t matrix, rotate_3D, ident;
    }
    vg_lite_clear(fb, NULL, 0x00000000);

    // Rotation angles (degree) for axis X, Y, Z
    compute_rotate(xrot, yrot, zrot, &rotate_3D);
    //xrot += rotstep;
    yrot += rotstep;
    //zrot += rotstep;

    // Compute the new cube vertex coordinates transformed by the rotation matrix.
    transfrom_rotate(&rotate_3D, &cube_v0, &rv0, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v1, &rv1, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v2, &rv2, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v3, &rv3, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v4, &rv4, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v5, &rv5, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v6, &rv6, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v7, &rv7, xoff, yoff);

    // Compute the surface normal direction to determine the front/back face.
    transfrom_normalZ(&rotate_3D, &normal0321, &nz0321);
    transfrom_normalZ(&rotate_3D, &normal4567, &nz4567);
    transfrom_normalZ(&rotate_3D, &normal1265, &nz5126);
    transfrom_normalZ(&rotate_3D, &normal0473, &nz0473);
    transfrom_normalZ(&rotate_3D, &normal2376, &nz7623);
    transfrom_normalZ(&rotate_3D, &normal0154, &nz0154);

        if (nz0321 > 0.0)
        {
            /* Compute 3x3 image transform matrix to map a rectangle image (w,h) to
               a parallelogram (x0,y0), (x1,y1), (x2,y2), (x3,y3) counterclock wise.  */

            transfrom_blit(image0.width, image0.height, &rv0, &rv3, &rv2, &rv1, &matrix);
            print_matrix(&matrix);

        // Blit the image using the matrix.
        vg_lite_blit(fb, &image0, &matrix, VG_LITE_BLEND_SCREEN, 0, filter);
    }

        if (nz4567 > 0.0)
        {
            /* Compute 3x3 image transform matrix to map a rectangle image (w,h) to
               a parallelogram (x0,y0), (x1,y1), (x2,y2), (x3,y3) counterclock wise. */

            transfrom_blit(image1.width, image1.height, &rv4, &rv5, &rv6, &rv7, &matrix);
            print_matrix(&matrix);

            /* Blit the image using the matrix. */
            vg_lite_blit(fb, &image1, &matrix, VG_LITE_BLEND_SCREEN, 0, filter);
        }

        if (nz5126 > 0.0)
        {
            /* Compute 3x3 image transform matrix to map a rectangle image (w,h) to
               a parallelogram (x0,y0), (x1,y1), (x2,y2), (x3,y3) counterclock wise. */

            transfrom_blit(image2.width, image2.height, &rv5, &rv1, &rv2, &rv6, &matrix);
            print_matrix(&matrix);

            /* Blit the image using the matrix. */
            vg_lite_blit(fb, &image2, &matrix, VG_LITE_BLEND_SCREEN, 0, filter);
        }

        if (nz0473 > 0.0)
        {
            /* Compute 3x3 image transform matrix to map a rectangle image (w,h) to
               a parallelogram (x0,y0), (x1,y1), (x2,y2), (x3,y3) counterclock wise. */

            transfrom_blit(image3.width, image3.height, &rv0, &rv4, &rv7, &rv3, &matrix);
            print_matrix(&matrix);

            /* Blit the image using the matrix. */
            vg_lite_blit(fb, &image3, &matrix, VG_LITE_BLEND_SCREEN, 0, filter);
        }

        if (nz7623 > 0.0)
        {
            /* Compute 3x3 image transform matrix to map a rectangle image (w,h) to
               a parallelogram (x0,y0), (x1,y1), (x2,y2), (x3,y3) counterclock wise. */

            transfrom_blit(image4.width, image4.height, &rv7, &rv6, &rv2, &rv3, &matrix);
            print_matrix(&matrix);

            /* Blit the image using the matrix. */
            vg_lite_blit(fb, &image4, &matrix, VG_LITE_BLEND_SCREEN, 0, filter);
        }

        if (nz0154 > 0.0)
        {
            /* Compute 3x3 image transform matrix to map a rectangle image (w,h) to
               a parallelogram (x0,y0), (x1,y1), (x2,y2), (x3,y3) counterclock wise. */

            transfrom_blit(image5.width, image5.height, &rv0, &rv1, &rv5, &rv4, &matrix);
            print_matrix(&matrix);

            /* Blit the image using the matrix. */
            vg_lite_blit(fb, &image5, &matrix, VG_LITE_BLEND_SCREEN, 0, filter);
        }
    vg_lite_finish();

    // Show rendering on screen.
    if (has_fb) {
        vg_lite_blit(sys_fb, fb, &ident, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
        vg_lite_finish();
    }

    gpu_lcdc_display(fb);

    // Save PNG file.
    gpu_save_raw("clear.raw", fb);

    gpu_set_soft_break_point();

}
// while (frames--)

    // Cleanup.
    cleanup();

    return 0;
}
