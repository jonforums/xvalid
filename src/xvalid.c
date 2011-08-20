/**
 * Copyright (c) 2011 Jon Maken, All Rights Reserved
 * License: 3-Clause BSD
 * Revision: 08/20/2011 10:45:33 AM
 */

#include "xvalid.h"

static struct stat stat_info;


static void xvalid_usage(void)
{
	printf("XValid v%s - validate XML documents\n", XVALID_VERSION_STRING);
	printf("Usage: xval [options] XML_FILE ...\n");
	printf("\n");
	printf("where validation options are:\n");
	printf("  --dtd ROOT FILE  validate ROOT element with external DTD file\n");
	printf("  --xsd FILE       validate with external XSD schema file\n");
	printf("\n");
	printf("where general options are:\n");
	printf("  --handler FILE   use external error handler (.so|.dll|.lua)\n");
}

static int xvalid__embed_lua(xvalid_ctx_ptr ctx)
{
	assert(ctx != NULL);
	assert(ctx->L == NULL);

	ctx->L = luaL_newstate();
	if (ctx->L == NULL) return 1;

	/* stop the GC during Lua library (all std) initialization and function
	 * registrations
	 */
	lua_gc(ctx->L, LUA_GCSTOP, 0);
	luaL_openlibs(ctx->L);

	/* TODO register functions here */

	lua_gc(ctx->L, LUA_GCRESTART, 0);

	return 0;
}

/* TODO implement */
static int xvalid__dtd_validate(xvalid_ctx_ptr ctx)
{
	int rv = 0;

	assert(ctx != NULL);
	assert(ctx->dtd_file != NULL);
	assert(ctx->current_file != NULL);

	return 0;
}

static int xvalid__xsd_validate(xvalid_ctx_ptr ctx)
{
	int rv = 0;
	int ret = 0;
	xmlSchemaValidCtxtPtr xsd_valid_ctx = NULL;
	xmlParserInputBufferPtr input = NULL;

	assert(ctx != NULL);
	assert(ctx->schema != NULL);
	assert(ctx->current_file != NULL);

	input = xmlParserInputBufferCreateFilename(ctx->current_file,
				XML_CHAR_ENCODING_NONE);
	if (input == NULL)
	{
		fprintf(stderr, "[ERROR] unable to open %s for XSD validation\n\n",
				ctx->current_file);
		rv = 1;
		goto done;
	}

	/* TODO memoize XSD validation context */
	xsd_valid_ctx = xmlSchemaNewValidCtxt(ctx->schema);
	if (xsd_valid_ctx == NULL)
	{
		fprintf(stderr, "[ERROR] unable to create an XSD validation context\n\n");
		rv = 1;
		goto done;
	}
	if ((ret = xmlSchemaValidateStream(
					xsd_valid_ctx,
					input,
					XML_CHAR_ENCODING_NONE,
					ctx->handlers,
					NULL)))
	{

		if (ret > 0)
		{
			fprintf(stderr, "%s fails XSD validation\n", ctx->current_file);
		}
		else
		{
			fprintf(stdout, "%s XSD validation caused an internal error\n",
					ctx->current_file);
		}
	}
	else if (ret == 0)
	{
		fprintf(stdout, "Successfully XSD validated %s\n", ctx->current_file);
	}

	/* cleanup */
	xmlSchemaFreeValidCtxt(xsd_valid_ctx);
	xsd_valid_ctx = NULL;

done:
	return rv;
}

static int xvalid_validate_xml_file(xvalid_ctx_ptr ctx, const char *filename)
{
	int rv = 0;
	xmlSchemaParserCtxtPtr xsd_ctx = NULL;

	assert(ctx != NULL);
	assert(filename != NULL);

	ctx->current_file = filename;

	/* validate with given XSD schema file if given */
	if (ctx->schema_file != NULL)
	{
		/* compile and memoize XSD schema if not already present */
		if (ctx->schema == NULL)
		{
			xsd_ctx = xmlSchemaNewParserCtxt(ctx->schema_file);
			if (xsd_ctx == NULL)
			{
				fprintf(stderr, "[ERROR] unable to create an XSD parse context\n\n");
				rv = 1;
				goto done;
			}
			ctx->schema = xmlSchemaParse(xsd_ctx);
			if (ctx->schema == NULL)
			{
				fprintf(stderr, "[ERROR] unable to compile XSD %s\n\n", ctx->schema_file);
				rv = 1;
				goto done;
			}
			xmlSchemaFreeParserCtxt(xsd_ctx);
			xsd_ctx = NULL;
		}
		/* TODO check return value */
		xvalid__xsd_validate(ctx);
	}
	/* validate with DTD file if given */
	else if (ctx->dtd_file != NULL)
	{
		/* TODO check return value */
		xvalid__dtd_validate(ctx);
	}
	/* parse and potentially validate with embedded DTD/XSD info
	 * TODO any parser configuration settings needed?
	 */
	else
	{
		/* TODO check return value */
		xmlSAXUserParseFile(ctx->handlers, NULL, filename);
	}

done:
	return rv;
}

static int xvalid_init(xvalid_ctx_ptr ctx)
{
	assert(ctx != NULL);

	ctx->dtd_file = NULL;
	ctx->root = NULL;
	ctx->schema_file = NULL;
	ctx->handlers = &sax_handlers;
	ctx->handler_plugin = NULL;
	ctx->current_file = NULL;
	ctx->schema = NULL;
	ctx->L = NULL;
	ctx->parser_options = 0;
	ctx->errors = 0;

	return 0;
}

static int xvalid__parse_options(xvalid_ctx_ptr ctx, int argc, char **argv)
{
	int i;

	assert(ctx != NULL);

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-")) break;

		if ((!strcmp(argv[i], "-dtd")) || (!strcmp(argv[i], "--dtd")))
		{
			i++;
			ctx->root = argv[i];
			i++;
			ctx->dtd_file = argv[i];
			ctx->parser_options |= XML_PARSE_DTDLOAD;
#ifdef XVALID_CHATTY_BUILD
			printf("Using external DTD file %s to parse %s root element\n",
					ctx->dtd_file,
					ctx->root);
#endif
			continue;
		}
		else if ((!strcmp(argv[i], "-xsd")) || (!strcmp(argv[i], "--xsd")))
		{
			i++;
			ctx->schema_file = argv[i];
#ifdef XVALID_CHATTY_BUILD
			printf("Using external XSD file %s\n", ctx->schema_file);
#endif
			continue;
		}
		else if ((!strcmp(argv[i], "-handler")) || (!strcmp(argv[i], "--handler")))
		{
			i++;
			ctx->handler_plugin = argv[i];
#ifdef XVALID_CHATTY_BUILD
			printf("Using external handler plugin %s\n", ctx->handler_plugin);
#endif
			continue;
		}
		else break;
	}

#ifdef XVALID_CHATTY_BUILD
	printf("File start at argv[%d]\n", i);
#endif

	return i;
}

static int xvalid__check_config(xvalid_ctx_ptr ctx)
{
	assert(ctx != NULL);

	if ((ctx->dtd_file != NULL) && (ctx->schema_file != NULL))
	{
		fprintf(stderr, "[ERROR] choose one of --dtd or --xsd, not both\n\n");
		return 1;
	}

	if (ctx->dtd_file != NULL)
	{
		if ((ctx->root == NULL) || (stat(ctx->root, &stat_info) == 0))
		{
			fprintf(stderr, "[ERROR] must give root XML element for DTD validation\n\n");
			return 1;
		}

		if (stat(ctx->dtd_file, &stat_info) < 0)
		{
			switch (errno)
			{
				case ENOENT:
					fprintf(stderr, "[ERROR] DTD file '%s' not found\n\n", ctx->dtd_file);
					return 1;
			}
		}
	}
	else if (ctx->schema_file != NULL)
	{
		if (stat(ctx->schema_file, &stat_info) < 0)
		{
			switch (errno)
			{
				case ENOENT:
					fprintf(stderr, "[ERROR] XSD file '%s' not found\n\n", ctx->schema_file);
					return 1;
			}
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	int i;
	int rv = 0;
	int res = 0;
	int files_start;
	xvalid_context context;
	xvalid_ctx_ptr ctx = &context;

	if (argc == 1)
	{
		xvalid_usage();
		rv = EXIT_FAILURE;
		goto done;
	}

	if (xvalid_init(ctx))
	{
		fprintf(stderr, "Unable to initialized XValid application; exiting\n");
		rv = EXIT_FAILURE;
		goto done;
	}

	files_start = xvalid__parse_options(ctx, argc, argv);

	if (xvalid__check_config(ctx))
	{
		xvalid_usage();
		rv = EXIT_FAILURE;
		goto done;
	}

	/* embed Lua interpreter */
	if (xvalid__embed_lua(ctx))
	{
		fprintf(stderr, "Cannot initialize Lua; exiting...\n");
		rv = EXIT_FAILURE;
		goto done;
	}

	/* validate files given on cmd line */
	for (i = files_start; i < argc; i++)
	{
		if (stat(argv[i], &stat_info) < 0)
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

		if ((res = xvalid_validate_xml_file(ctx, argv[i])))
		{
			if (res == 1)
			{
				rv = EXIT_FAILURE;
				break;
			}
		}
	}

	/* cleanup */
	xmlCleanupParser();
	lua_close(ctx->L);

done:
	return rv;
}
