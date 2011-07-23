#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#if defined (_WIN32) && !defined(__CYGWIN__)
#  include <sys/types.h>
#  include <sys/stat.h>
#  define stat(f,b) _stat((f), (b))
#else
#  include <sys/stat.h>
#endif /* _WIN32 && !__CYGWIN__ */
#include <errno.h>

#include <libxml/parser.h>
