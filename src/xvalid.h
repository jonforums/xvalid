#ifndef XVALID_H
#define XVALID_H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <libxml/parser.h>

#define MAX_FILE_COUNT    200
#define MAX_FILENAME_SIZE 100

#ifdef XVALID_DEBUG_BUILD
extern void start_document(void *ctx);
extern void end_document(void *ctx);
extern void start_element(void *ctx, const xmlChar *name, const xmlChar **atts);
extern void end_element(void *ctx, const xmlChar *name);
#endif /* XVALID_DEBUG_BUILD */

/* TODO extract to handlers.h and include? */
extern void warning(void *ctx, const char *msg, ...);
extern void error(void *ctx, const char *msg, ...);
extern void fatal_error(void *ctx, const char *msg, ...);
extern void say(void *ctx, const char *tag, const char *msg);

/**
 * Context for maintaining XValid's state during processing
 */
typedef struct _xvalid_context {
	char      *dtd;
	char      *schema;
	char      *handler_plugin;
	char      *current_filename;
	lua_State *L;
	int        parser_options;
	int        errors;
} xvalid_context;
typedef xvalid_context *xvalid_ctx_ptr;

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

#endif /* XVALID_H */
