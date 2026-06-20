#ifndef __SFT_COMMON_H__
#define __SFT_COMMON_H__

#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }
void StatesReset();

#if UNIT_TEST_ENABLE
    void Run_Unit_Test();
#endif

vg_lite_error_t Run_SFT();
vg_lite_error_t Render();

extern char * Cmd;
extern char *error_type[];

#endif
