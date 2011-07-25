/**
 * Copyright (c) 2011 Jon Maken, All Rights Reserved
 * License: 3-Clause BSD
 * Revision: 07/24/2011 8:40:06 PM
 */

#include "xvalid.h"

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

static struct stat stat_info;

static lua_State *L = NULL;


static lua_State *embed_lua(void)
{
	L = luaL_newstate();
	if (L == NULL) return NULL;

	/* stop the GC during Lua library (all std) initialization and function
	 * registrations
	 */
	lua_gc(L, LUA_GCSTOP, 0);
	luaL_openlibs(L);

	/* TODO register functions here */

	lua_gc(L, LUA_GCRESTART, 0);

	return(L);
}

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

	/* embed Lua interpreter */
    L = embed_lua();
	if (L == NULL)
	{
		fprintf(stderr, "Cannot initialize Lua; exiting...");
		return EXIT_FAILURE;
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

	/* cleanup Lua */
	lua_close(L);

	/* cleanup function for the XML library; boilerplate */
	xmlCleanupParser();

	return EXIT_SUCCESS;
}
