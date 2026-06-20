/*
 Description: Read raw files and use upload to 
 generate gradient character images.It can count
 the running time if each frame on the linux platform.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

/*
 Header files and variables are used for time
 statistics under the linux platform.
 */
#if (defined __LINUX__)
#include<sys/time.h>
#include<unistd.h>
struct timeval time1,time2;
#endif

#define DEFAULT_SIZE   20;
static int fb_width =600, fb_height = 32;
static float fb_scale = 1.0f;

/*offscreen framebuffer object for rendering.*/
static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;

void cleanup(void)
{
    if (buffer.handle != NULL) {
        /* Free the offscreen framebuffer memory.*/
        vg_lite_free(&buffer);
    }
    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    vg_lite_linear_gradient_t grad;
    uint32_t ramps[] = {0xffffffff, 0xff000000};
    uint32_t stops[] = {0, 36};
    vg_lite_matrix_t *matGrad;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix[24];
    float offsetX = 0;
    float startX = 0.0f;
    float startY = -5;
    FILE *fd[12];
    float *path_data[11];
    static vg_lite_path_t path[11];
    int32_t length;
    int row = 0;
    int column = 0;
    int frame = 0;
    int character = 0;
    int fileid = 0;
    int index = 0;

    /* Initialize the draw.*/
    error = vg_lite_init(640,320);
    if (error) {
        printf("vg_lite engine init failed: vg_lite_init() returned error %d\n", error);
        cleanup();
        return -1;
    }

    fb_scale = 1.3;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    /*Allocate the off-screen buffer.*/
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_BGRA8888;

    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;
    /* Clear the buffer with white. */
    vg_lite_clear(fb, NULL, 0xFFFFFFFF);

    for(row = 0; row < 1; row++) {
        for(column = 0; column < 24; column++) {
            vg_lite_identity(&matrix[row*24+column]);
            /* Translate the matrix to the center of the buffer.*/
            vg_lite_scale(fb_scale, fb_scale, &matrix[row*24+column]);
            vg_lite_translate(18*column, 0, &matrix[row*24+column]);
            vg_lite_translate(startX, -7, &matrix[row*24+column]);
            }
    }

    fd[0] = fopen("./a.raw","rb");
    fd[1] = fopen("./b.raw","rb");
    fd[2] = fopen("./c.raw","rb");
    fd[3] = fopen("./d.raw","rb");
    fd[4] = fopen("./e.raw","rb");
    fd[5] = fopen("./f.raw","rb");
    fd[6] = fopen("./g.raw","rb");
    fd[7] = fopen("./h.raw","rb");
    fd[8] = fopen("./i.raw","rb");
    fd[9] = fopen("./j.raw","rb");
    fd[10] = fopen("./k.raw","rb");

    if(fd[0] == NULL || fd[1] == NULL || fd[2] == NULL ||
       fd[3] == NULL || fd[4] == NULL || fd[5] == NULL ||
       fd[6] == NULL || fd[7] == NULL || fd[8] == NULL ||
       fd[9] == NULL || fd[10] == NULL) {
        printf("File open failed.\n");
        return -1;
    }
    for(fileid = 0; fileid < 11; fileid++) {
        memset(&path[fileid], 0, sizeof(path[fileid]));
        fread(&path[fileid], sizeof(path[fileid]), 1, fd[fileid]);
        length = path[fileid].path_length;
        path_data[fileid] = malloc(path[fileid].path_length);
        fread(path_data[fileid], 1, path[fileid].path_length, fd[fileid]);
        fclose(fd[fileid]);
        path[fileid].path = path_data[fileid];
    }

    for(index = 0; index < 11; index++) {
        error = vg_lite_upload_path(&path[index]);
    }
    index = 0;
/*
 Record system time before drawing.
*/
#if (defined __LINUX__)
    gettimeofday(&time1,NULL);
#endif

    memset(&grad, 0, sizeof(grad));
    if (VG_LITE_SUCCESS != vg_lite_init_grad(&grad)) {
            printf("Linear gradient is not supported.\n");
            vg_lite_close();
            return 0;
    }
    vg_lite_set_grad(&grad, 2, ramps, stops);
    vg_lite_update_grad(&grad);
    matGrad = vg_lite_get_grad_matrix(&grad);
    vg_lite_identity(matGrad);
    vg_lite_rotate(90.0f, matGrad);
    vg_lite_scale(1, 2, matGrad);
    vg_lite_translate(0, -5, matGrad);
  
    for(frame = 0; frame < 1; frame++) {
        for(character = 0; character<24; character++) {
            index = character%11;
            error = vg_lite_draw_gradient(fb, &path[index], VG_LITE_FILL_EVEN_ODD, &matrix[character], &grad, VG_LITE_BLEND_NONE);
            if (error) {
                printf("vg_lite_draw() returned error %d\n", error);
                cleanup();
                free(path_data);
                return -1;
            }
            
        }
        printf("frame%d\n",frame);
        error = vg_lite_finish();
    }
/*
 Record system time after drawing and calculate drawing time.
*/
#if (defined __LINUX__)
    gettimeofday(&time2,NULL);
    double times =(time2.tv_sec - time1.tv_sec)*1000. +  (time2.tv_sec - time1.tv_sec)/1000.;
    printf("%lf(ms)\n",times);
#endif

    for(index = 0; index < 11; index++) {
        free(path_data[index]);
        error = vg_lite_clear_path(&path[index]);
    }

    vg_lite_clear_grad(&grad); 
    vg_lite_save_png("plyhs2_1.png", fb); 
    cleanup();
}

