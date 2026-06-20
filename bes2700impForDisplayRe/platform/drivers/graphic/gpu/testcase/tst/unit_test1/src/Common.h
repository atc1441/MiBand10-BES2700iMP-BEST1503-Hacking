#ifndef __COMMON_H__
#define __COMMON_H__

#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }

extern char *error_type[];
extern vg_lite_blend_t blend_mode[];
extern vg_lite_filter_t filter_modes[];
extern vg_lite_format_t formats[];
extern vg_lite_quality_t qualities[];
extern int32_t *test_path[9];
extern int32_t length[];
extern int8_t path_data10[];
extern int16_t path_data11[];
extern int32_t path_data12[];


void build_paths(vg_lite_path_t * path,vg_lite_quality_t qualities,vg_lite_filter_t filter_mode,vg_lite_blend_t blend_mode);
void output_string(char *str);
vg_lite_error_t Allocate_Buffer(vg_lite_buffer_t *buffer,
                                       vg_lite_buffer_format_t format,
                                       int32_t width, int32_t height);
void Free_Buffer(vg_lite_buffer_t *buffer);
vg_lite_error_t gen_buffer(int type, vg_lite_buffer_t *buf, vg_lite_buffer_format_t format, uint32_t width, uint32_t height);
void _memcpy(void *dst, void *src, int size) ;
vg_lite_error_t Render();

#endif
