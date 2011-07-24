#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <libxml/parser.h>

#ifdef XVALID_DEBUG_BUILD
extern void start_document(void *ctx);
extern void end_document(void *ctx);
extern void start_element(void *ctx, const xmlChar *name, const xmlChar **atts);
extern void end_element(void *ctx, const xmlChar *name);
#endif /* XVALID_DEBUG_BUILD */

extern void warning(void *ctx, const char *msg, ...);
extern void error(void *ctx, const char *msg, ...);
extern void fatal_error(void *ctx, const char *msg, ...);

static xmlSAXHandler sax_handlers = {
    NULL, /* internalSubset */
    NULL, /* isStandalone */
    NULL, /* hasInternalSubset */
    NULL, /* hasExternalSubset */
    NULL, /* resolveEntity */
    NULL, /* getEntity */
    NULL, /* entityDecl */
    NULL, /* notationDecl */
    NULL, /* attributeDecl */
    NULL, /* elementDecl */
    NULL, /* unparsedEntityDecl */
    NULL, /* setDocumentLocator */
#ifdef XVALID_DEBUG_BUILD
    start_document,
    end_document,
    start_element,
    end_element,
#else
    NULL, /* startDocument */
    NULL, /* endDocument */
    NULL, /* startElement */
    NULL, /* endElement */
#endif /* XVALID_DEBUG_BUILD */
    NULL, /* reference */
    NULL, /* characters */
    NULL, /* ignorableWhitespace */
    NULL, /* processingInstruction */
    NULL, /* comment */
    warning,
    error,
    fatal_error,
    NULL, /* getParameterEntity */
    NULL, /* cdataBlock; */
    NULL, /* externalSubset; */
    XML_SAX2_MAGIC,
    NULL,
    NULL, /* startElementNs */
    NULL, /* endElementNs */
    NULL  /* xmlStructuredErrorFunc */
};
static xmlSAXHandlerPtr handlers = &sax_handlers;
