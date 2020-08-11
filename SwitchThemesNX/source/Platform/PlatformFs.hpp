#pragma once

#ifndef __SWITCH__
#define ASSET(_str) "./romfs/" _str
#define SD_PREFIX "F:"
#else
#include <switch.h>
#define ASSET(_str) "romfs:/" _str
#define SD_PREFIX "sdmc:"
#endif

#if WIN32
#include <io.h>
#include <direct.h>
#include "Windows\dirent.h"
#define mkdir(x,y) _mkdir(x)
#define rmdir(x) _rmdir(x)
#define unlink(x) _unlink(x)
#undef CreateDirectory
#undef max
#else
#include <unistd.h>
#include <dirent.h>
#endif

#define LOGf(...) printf(__VA_ARGS__)