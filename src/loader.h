#ifndef LOADER_H
#define LOADER_H

#include <stdio.h>

#ifndef _WIN32
#  include <dlfcn.h>
#  define HANDLE void*
#  define load_lib(i) dlopen((i), RTLD_LAZY | RTLD_GLOBAL)
#  define symbol_from_lib dlsym
#  define close_lib dlclose
#else
#  include <windows.h>
#  define load_lib xvalid_lib_loader
#  define symbol_from_lib GetProcAddress
#  define close_lib FreeLibrary
#endif /* _WIN32 */

HANDLE xvalid_lib_loader(char *filename);

#endif /* LOADER_H */
