/**
 * Copyright (c) 2011 Jon Maken, All Rights Reserved
 * License: 3-Clause BSD
 * Revision: 07/23/2011 3:03:37 PM
 */

#include "xvalid_private.h"

static void usage(void)
{
	printf("XValid v%s - validate XML documents\n", XVALID_VERSION_STRING);
	printf("Usage: xval [options] XML_FILE ...\n");
	printf("\n");
	printf("where validation are:\n");
	printf("  --dtd FILE      validate against external DTD file\n");
	printf("  --xsd FILE      validate against external XSD schema file\n");
	printf("\n");
	printf("where general options are:\n");
	printf("  --handler FILE  use external error handler (.so|.dll|.lua)\n");
}

static char *handler_plugin = NULL;

static char *dtd_file = NULL;
static char *schema_file = NULL;

static int parser_options = 0;

#if defined (_WIN32) && !defined(__CYGWIN__)
static struct _stat stat_info;
#else
static struct stat stat_info;
#endif

/**
 * debugging SAX handler callbacks
 */
#ifdef XVALID_DEBUG_BUILD
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
#endif /* XVALID_DEBUG_BUILD */

/* TODO extract error handling callbacks into their own module */
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

static int validate_xml_file(const char *filename)
{
	xmlSAXUserParseFile(handlers, NULL, filename);

	return(0);
}

static int check_config(void)
{
	if ((dtd_file != NULL) && (schema_file != NULL))
	{
		fprintf(stderr, "[ERROR] choose one of --dtd or --xsd, not both\n");
		return(1);
	}

	return(0);
}

int main(int argc, char **argv)
{
	int i, rv;
	int file_start = 1;


	if (argc == 1)
	{
		usage();
		return(1);
	}

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-")) break;

		if ((!strcmp(argv[i], "-dtd")) || (!strcmp(argv[i], "--dtd")))
		{
			i++;
			dtd_file = argv[i];
			parser_options |= XML_PARSE_DTDLOAD;
#ifdef XVALID_DEBUG_BUILD
			printf("Using external DTD file %s\n", dtd_file);
#endif
			file_start = i + 1;
			continue;
		}
		else if ((!strcmp(argv[i], "-xsd")) || (!strcmp(argv[i], "--xsd")))
		{
			i++;
			schema_file = argv[i];
#ifdef XVALID_DEBUG_BUILD
			printf("Using external XSD file %s\n", schema_file);
#endif
			file_start = i + 1;
			continue;
		}
		else if ((!strcmp(argv[i], "-handler")) || (!strcmp(argv[i], "--handler")))
		{
			i++;
			handler_plugin = argv[i];
#ifdef XVALID_DEBUG_BUILD
			printf("Using external handler plugin %s\n", handler_plugin);
#endif
			file_start = i + 1;
			continue;
		}
		else break;
	}

	if (check_config())
	{
		usage();
		return(1);
	}

	for (i = file_start; i < argc; i++)
	{
		rv = stat(argv[i], &stat_info);
		if (rv < 0)
		{
			switch (errno)
			{
				case ENOENT:
					fprintf(stderr, "File '%s' not found\n", argv[i]);
					break;
				default:
					fprintf(stderr, "Unexpected error while checking %s\n", argv[i]);
			}
			continue;
		}

#ifdef XVALID_DEBUG_BUILD
		printf("Validating XML file %s\n", argv[i]);
#endif

		/* TODO confirm whether cleanup needed between iterations */
		validate_xml_file(argv[i]);
	}

	/* cleanup function for the XML library; boilerplate */
	xmlCleanupParser();

	return(0);
}
