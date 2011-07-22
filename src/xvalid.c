/**
 * Copyright (c) 2011 Jon Maken, All Rights Reserved
 * License: 3-Clause BSD
 * Revision: 07/21/2011 7:46:40 PM
 */

#include "xvalid_private.h"

static void usage(void)
{
	printf("XVal: Validate XML documents\n");
	printf("Usage: xval [--dtd DTD_FILE | --xsd XSD_FILE] XML_FILE ...\n\n");
	printf("  --dtd DTD_FILE  validate against external DTD file\n");
	printf("  --xsd XSD_FILE  validate against external XSD schema file\n");
}

/**
 * SAX handler callbacks
 *
 * TODO ifdef in the document and element callbacks only in debug mode
 */
static void start_document(void *ctx)
{
    fprintf(stdout, "SAX.start_document()\n");
}

static void end_document(void *ctx)
{
    fprintf(stdout, "SAX.end_document()\n");
}

static void start_element(void *ctx, const xmlChar *name, const xmlChar **atts)
{
	fprintf(stdout, "SAX.start_element() for %s\n", (char *) name);
}

static void end_element(void *ctx, const xmlChar *name)
{
	fprintf(stdout, "SAX.end_element() for %s\n", (char *) name);
}

static void warning(void *ctx, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fprintf(stdout, "SAX.warning()\n");
	vfprintf(stdout, msg, args);
	va_end(args);
}

static void error(void *ctx, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fprintf(stdout, "SAX.error()\n");
	vfprintf(stdout, msg, args);
	va_end(args);
}

static void fatal_error(void *ctx, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fprintf(stdout, "SAX.fatal_error()\n");
	vfprintf(stdout, msg, args);
	va_end(args);
}


/* TODO ifdef in the document and element callbacks only in debug mode */
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
    start_document,
    end_document,
    start_element,
    end_element,
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

static int process_options(int argc, char **argv)
{
	if (argc == 1) return(1);

	return(0);
}

static int validate_xml_file(const char *filename)
{
	xmlSAXUserParseFile(handlers, NULL, filename);

	return(0);
}

int main(int argc, char **argv)
{
	int rv;

	if ((rv = process_options(argc, argv) != 0))
	{
		usage();
		return(rv);
	}

	validate_xml_file(argv[1]);

	/* cleanup function for the XML library; boilerplate */
	xmlCleanupParser();

	return(0);
}
