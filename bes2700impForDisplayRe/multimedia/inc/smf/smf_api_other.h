#pragma once

#include "smf_common.h"

EXTERNC void smf_io_file_register(void);

EXTERNC void scan_folder(const char* path, void(*cb)(const char* fname), bool recursion, char skipFolderPrefix);
EXTERNC void create_folder(const char* path);