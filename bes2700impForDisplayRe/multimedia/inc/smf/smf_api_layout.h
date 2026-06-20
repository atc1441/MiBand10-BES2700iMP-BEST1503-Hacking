#pragma once
#include <smf_common.h>
#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

/** create a object from layout script string.
 * @param script[in] pipeline script string.
 * @param parent[in] set the parent for pipeline.
 * @return return the object pointer.
 */
EXTERNC void* smf_create_object_from_string(const char* script, void* parent);

/** create a object from layout script string.
 * @param url[in] the url of config file.
 * @return return the object pointer.
 */
EXTERNC void* smf_create_object_from_io(const char* url);

/** create a object from layout script string.
 * @param script[in] pipeline script string.
 * @param parent[in] the parent.
 * @param params[in] the param table.
 * @return return the object pointer.
 */
EXTERNC void* smf_create_object_with_params(const char* script, void* parent, uint32_t* params);

/** register pipeline
 */
EXTERNC void smf_pipeline_register();

EXTERNC bool smf_test_layout(const char* serial, uint32_t*);
