#include "SFT.h"

static int fb_width = 320, fb_height = 480;
static vg_lite_buffer_t raw_fb;

void cleanup(void)
{
    if (raw_fb.handle != NULL) {
        vg_lite_free(&raw_fb);
    }

    vg_lite_close();
}

int parse_argv(const char * argv[])
{
    const char* arg = argv[1];
    if (!strcmp(arg, "gfx1"))
        return 1;
    else if ((!strcmp(arg, "gfx2")) || (!strcmp(arg, "gfx3")))
        return 2;
    else if (!strcmp(arg, "gfx4"))
        return 4;
    else if (!strcmp(arg, "gfx5"))
        return 5;
    else if (!strcmp(arg, "gfx6"))
        return 6;
    else if (!strcmp(arg, "gfx7"))
        return 7;
    else if (!strcmp(arg, "gfx8"))
        return 8;
    else if ((!strcmp(arg, "gfx9")) || (!strcmp(arg, "gfx10")))
        return 9;
    else if (!strcmp(arg, "gfx11"))
        return 11;
    else if (!strcmp(arg, "concur"))
        return 12;
    else
        return 0;
}

int render(const char * argv[])
{
    int ret = TB_OK;
    int retarg;
    int i;
    
    if (argv[1] != NULL) {
        retarg = parse_argv(argv);

        switch (retarg) {
            case 1:
                VG_LITE_ONERROR(exe_clear(fb_width, fb_height, &raw_fb));
                break;
                
            case 2:
                VG_LITE_ONERROR(exe_ui(fb_width, fb_height, &raw_fb));
                break;
                
            case 4:
                VG_LITE_ONERROR(SFT_Blit(SFT_BLIT_COLOR_FORMAT_TEST_CODE, &raw_fb, fb_width, fb_height));
                break;
                
            case 5:
                VG_LITE_ONERROR(SFT_Path_Draw(SFT_DRAW_FILL_RULE_TEST_CODE, &raw_fb, fb_width, fb_height));
                break;
                
            case 6:
                VG_LITE_ONERROR(SFT_Path_Draw(SFT_DRAW_PATH_COORD_TEST_CODE, &raw_fb, fb_width, fb_height));
                break;
                
            case 7:
                VG_LITE_ONERROR(SFT_Path_Draw(SFT_DRAW_ANTI_ALIAS_TEST_CODE, &raw_fb, fb_width, fb_height));
                break;
                
            case 8:
                VG_LITE_ONERROR(SFT_Path_Draw(SFT_DRAW_GENERIC_TEST_CODE, &raw_fb, fb_width, fb_height));
                break;
                
            case 9:
                VG_LITE_ONERROR(SFT_Blit(SFT_BLIT_RESOLUTION_TEST_CODE, &raw_fb, fb_width, fb_height));
                break;
                
            case 11:
                VG_LITE_ONERROR(SFT_Blit(SFT_BLIT_STRIDE_TEST_CODE, &raw_fb, fb_width, fb_height));
                break;
                
            case 12:
                VG_LITE_ONERROR(exe_clear(fb_width, fb_height, &raw_fb));
                VG_LITE_ONERROR(exe_ui(fb_width, fb_height, &raw_fb));
                for (i = 0; i < 3; i++) {
                    VG_LITE_ONERROR(SFT_Blit(i + 1, &raw_fb, fb_width, fb_height));
                }
                for (i = 0; i < 4; i++) {
                    VG_LITE_ONERROR(SFT_Path_Draw(i + 1, &raw_fb, fb_width, fb_height));
                }
                break;
                
            default:
                printf("incorrect argument\n");
                ret = TB_BAD;
                break;
        }
    } else {
        printf("no input argument\n");
    }
    
    return ret;
}

int main(int argc, const char * argv[])
{
    int ret;

    /* Draw. */
    ret = render(argv);

    cleanup();

    return ret;
}
