#include "handlers.h"

void warning(void *ctx, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fprintf(stdout, "SAX.warning()\n");
	vfprintf(stdout, msg, args);
	va_end(args);
}

void error(void *ctx, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fprintf(stdout, "SAX.error()\n");
	vfprintf(stdout, msg, args);
	va_end(args);
}

void fatal_error(void *ctx, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fprintf(stdout, "SAX.fatal_error()\n");
	vfprintf(stdout, msg, args);
	va_end(args);
}

void say(void *ctx, const char *tag, const char *msg)
{
	fprintf(stdout, "[%s] %s\n", tag, msg);
}
