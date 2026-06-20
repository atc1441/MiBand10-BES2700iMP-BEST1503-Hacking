#include    <stdio.h>
#include    <string.h>

#include    "SFT.h"

#if UNIT_TEST_ENABLE
#include "unit.h"
#endif

#include    "Common.h"

int SFT_REFER;
#if !(defined __LINUX__ || defined __NUCLEUS__ )
FILE *f_casename;
#endif

/* The global fb object for on-screen rendering. */
vg_lite_buffer_t *g_fb = NULL;
vg_lite_rectangle_t g_fb_rect;
vg_lite_buffer_t g_buffer;
vg_lite_filter_t filter;

/* name of file that is used to dump exception information. */
#ifdef _DEBUG
#define EXCEPT_DUMP_FILE    "exception_d.txt"
#else
#define EXCEPT_DUMP_FILE    "exception_r.txt"
#endif

#if (defined __LINUX__ || defined __NUCLEUS__ )
#define __TRY__
#define __EXCEPTION__(name)
#else
#define __TRY__ \
    _try {

#define __EXCEPTION__(name) \
}               \
    _except(EXCEPTION_EXECUTE_HANDLER){      \
    FILE* flog=NULL;                        \
    flog = fopen(EXCEPT_DUMP_FILE, "a");    \
    fprintf(flog, "Case cause exception: %s \n", name); \
    fclose(flog);                          \
}
#endif

char * Cmd = NULL;
char LogString[2048] = {0};

typedef struct _SFT_CASE {
    vg_lite_error_t (*cases)();
    void (*case_log)();
    int seed_offset;
    char * case_name;
} SFT_CASE;

#define CASE_ENTRY(cases,log,seed_offset)	{cases, log, seed_offset, #cases}

char *error_type[] = 
{
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",      
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};

int random_seed = 32557;
SFT_CASE SFT_cases[] = 
{
    CASE_ENTRY(Clear,                       Clear_Log,                          0),
    CASE_ENTRY(Tessellation,                Tessellation_Log,                   2),
    CASE_ENTRY(Draw_Image,                  Draw_Image_Log,                     1),
    CASE_ENTRY(Combination,                 Combination_Log,                    3),
    CASE_ENTRY(NULL,                        NULL,                               -1)
};

/* RUN SFT */
vg_lite_error_t Run_SFT()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i = 0;
    void (*run)();
    vg_lite_error_t (*run1)();
    BOOL specify;

    /*  Need to generate all the log files before running cases,
        thus we need two whiles. */
    while(SFT_cases[i].cases != NULL){
        specify = FALSE;
        if (Cmd == NULL || (specify = (strcmp (SFT_cases[i].case_name, Cmd) == 0)) ){
            if(SFT_REFER == 0){
                run = SFT_cases[i].case_log;
                (*run)();
                /* clear the common LogString each time finishing case log process */
                memset(LogString, 0, 2048);
            }
            if (specify)
                break;
        }
        i++;
    }

    i = 0;
    while(SFT_cases[i].cases != NULL){
        specify = FALSE;
        if (Cmd == NULL || (specify = (strcmp (SFT_cases[i].case_name, Cmd) == 0)) ){
            random_srand(random_seed - SFT_cases[i].seed_offset);
            printf("VGLite Test: Run case %s.\n", SFT_cases[i].case_name);
            run1 = SFT_cases[i].cases;
            error = (*run1)();
            if (error)
            {
                printf("[%s: %d]run case failed.error type is %s\n", __func__, __LINE__,error_type[error]);
                return error;
            }
            /* clear the common LogString each time finishing running case */
            memset(LogString, 0, 2048);
            if (specify)
                break;
        }

        i++;
    }
    return error;
}

vg_lite_error_t Render()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    InitBMP(1920, 1080);

    error = Run_SFT();
    if (error)
    {
        printf("[%s: %d]Run_SFT failed.error type is %s\n", __func__, __LINE__,error_type[error]);
        return error;
    }
    DestroyBMP();
    return error;
}

