#include "debug_handlers.h"

#ifdef XVALID_DEBUG_BUILD
void start_document(void *ctx)
{
    fprintf(stdout, "  SAX.start_document()\n");
}

void end_document(void *ctx)
{
    fprintf(stdout, "  SAX.end_document()\n");
}

void start_element(void *ctx, const xmlChar *name, const xmlChar **atts)
{
	fprintf(stdout, "  SAX.start_element() for %s\n", (char *) name);
}

void end_element(void *ctx, const xmlChar *name)
{
	fprintf(stdout, "  SAX.end_element() for %s\n", (char *) name);
}
#endif /* XVALID_DEBUG_BUILD */
