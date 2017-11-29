#include "smn8/rom.h"

#define VERSION_STRING "1.0.0"

int disasm(smn8_rom *rom);
void print_version(const char *name);
void print_usage(const char *name);
int print_version_or_usage(const char *argv[]);

int main(int argc, char *argv[])
{
	smn8_rom rom;
	const char *filename;
	FILE *fp;
	int ret;

	if(argc > 1)
	{
		if(*argv[1] == '-')
			return print_version_or_usage((const char **) argv);

		filename = argv[1];
	}
	else
	{
#ifdef __EMSCRIPTEN__
		filename = "roms/TETRIS";
#else
		filename = NULL;
#endif
	}

	if(filename != NULL)
	{
		fp = fopen(filename, "rb");

		if(fp == NULL)
		{
			fprintf(stderr, "Could not open ROM '%s'.\n", filename);
			return EXIT_FAILURE;
		}
	}
	else
	{
		fp = stdin;
	}

	ret = smn8_rom_load(&rom, fp) ? disasm(&rom) : EXIT_FAILURE;

	if(argc > 1 && fp != NULL)
		fclose(fp);

	return ret;
}

int disasm(smn8_rom *rom)
{
	smn8_rom_disasm(rom, stdout);
	return EXIT_SUCCESS;
}

void print_version(const char *name)
{
	fprintf(stdout, "%s version %s\n", name, VERSION_STRING);
}

void print_usage(const char *name)
{
	fprintf(stdout, "usage: %s [FILE]\n", name);
	fprintf(stdout, "options:\n");
	fprintf(stdout, "\t-h\tdisplay this help and exit\n");
	fprintf(stdout, "\t-v\toutput version information and exit\n");
}

int print_version_or_usage(const char *argv[])
{
	if(*++argv[1] == 'v')
	{
		print_version(argv[0]);
		return EXIT_SUCCESS;
	}

	print_usage(argv[0]);
	return *argv[1] == 'h' ? EXIT_SUCCESS : EXIT_FAILURE;
}
