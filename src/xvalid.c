/**
 * Copyright (c) 2011 Jon Maken, All Rights Reserved
 * License: 3-Clause BSD
 * Revision: 07/27/2011 1:08:59 PM
 */

#include "xvalid.h"

static struct stat stat_info;


static void xvalid_usage(void)
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

static int xvalid_embed_lua(xvalid_ctx_ptr ctx)
{
	assert(ctx != NULL);

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

/* TODO implement both DTD and XSD validation */
static int xvalid_validate_xml_file(xvalid_ctx_ptr ctx, const char *filename)
{
	assert(ctx != NULL);
	assert(filename != NULL);

	xmlSAXUserParseFile(handlers, NULL, filename);

	say(ctx, "INFO", "Validated");

	return 0;
}

static int xvalid_init(xvalid_ctx_ptr ctx)
{
	assert(ctx != NULL);

	ctx->dtd = NULL;
	ctx->schema = NULL;
	ctx->handler_plugin = NULL;
	ctx->current_filename = NULL;
	ctx->parser_options = 0;
	ctx->errors = 0;

	return 0;
}

static int xvalid_parse_options(xvalid_ctx_ptr ctx, int argc, char **argv)
{
	int i;

	assert(ctx != NULL);

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-")) break;

		if ((!strcmp(argv[i], "-dtd")) || (!strcmp(argv[i], "--dtd")))
		{
			i++;
			ctx->dtd = argv[i];
			ctx->parser_options |= XML_PARSE_DTDLOAD;
#ifdef XVALID_DEBUG_BUILD
			printf("Using external DTD file %s\n", ctx->dtd);
#endif
			continue;
		}
		else if ((!strcmp(argv[i], "-xsd")) || (!strcmp(argv[i], "--xsd")))
		{
			i++;
			ctx->schema = argv[i];
#ifdef XVALID_DEBUG_BUILD
			printf("Using external XSD file %s\n", ctx->schema);
#endif
			continue;
		}
		else if ((!strcmp(argv[i], "-handler")) || (!strcmp(argv[i], "--handler")))
		{
			i++;
			ctx->handler_plugin = argv[i];
#ifdef XVALID_DEBUG_BUILD
			printf("Using external handler plugin %s\n", ctx->handler_plugin);
#endif
			continue;
		}
		else break;
	}

#ifdef XVALID_DEBUG_BUILD
	printf("File start at argv[%d]\n", i);
#endif

	return i;
}

static int xvalid_check_config(xvalid_ctx_ptr ctx)
{
	assert(ctx != NULL);

	if ((ctx->dtd != NULL) && (ctx->schema != NULL))
	{
		fprintf(stderr, "[ERROR] choose one of --dtd or --xsd, not both\n");
		return 1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int i;
	int rv = 0;
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

	files_start = xvalid_parse_options(ctx, argc, argv);

	if (xvalid_check_config(ctx))
	{
		xvalid_usage();
		rv = EXIT_FAILURE;
		goto done;
	}

	/* embed Lua interpreter */
	if (xvalid_embed_lua(ctx))
	{
		fprintf(stderr, "Cannot initialize Lua; exiting...\n");
		rv = EXIT_FAILURE;
		goto done;
	}

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

		/* TODO confirm whether cleanup needed between iterations */
		xvalid_validate_xml_file(NULL, argv[i]);
	}

	/* cleanup function for the XML library; boilerplate */
	xmlCleanupParser();

	/* cleanup Lua */
	lua_close(ctx->L);

done:
	return rv;
}
