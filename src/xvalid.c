/**
 * Copyright (c) 2011 Jon Maken, All Rights Reserved
 * License: 3-Clause BSD
 * Revision: 07/21/2011 2:44:32 PM
 */
#include <stdio.h>

static void usage(void)
{
	printf("XVal: Validate XML documents\n");
	printf("Usage: xval [--dtd DTD_FILE | --xsd XSD_FILE] XML_FILE ...\n\n");
	printf("  --dtd DTD_FILE  validate against external DTD file\n");
	printf("  --xsd XSD_FILE  validate against external XSD schema file\n");
}

static int process_options(int argc, char **argv)
{
	if (argc == 1) return(1);

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

	/* cleanup function for the XML library */
	/* xmlCleanupParser(); */

	return(0);
}
