#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "nucpack.h"

//#define DEBUG_TRACE
#include "debug.h"

#define VERSION "0.1"

static char* items_dir=NULL;
static char* ddr_ini_file="sys_cfg/NUC976DK62Y.ini";
static char* output_file=NULL;
static char* repack_file=NULL;

static PACK_ITEM item[] =
{
    {   .type=UBOOT, .name="u-boot.bin",      .start=0x00000000, .exec=0x00e00000 },
    {   .type=DATA,  .name="environment.img", .start=0x00040000 },
    {   .type=DATA,  .name="uImage",          .start=0x00050000 },
    {   .type=DATA,  .name="rootfs.jffs2",    .start=0x00360000 },
};

static size_t item_sz = sizeof(item)/sizeof(PACK_ITEM);        // количество частей


/**
 print usage information
 */
static void usage(FILE* fp, int argc, char** argv) {
    fprintf(fp, "\nNucPack Generator v.%s\n\n"
        "Usage: %s [options]\n\n"
        "Options:\n"
        "-h | --help           print this message\n"
        "-i | --items-dir      directory with u-boot.bin, environment.img, uImage, rootfs.jffs2\n"
        "-d | --ddr-ini-file   DDR ini file from sys_cfg\n"
        "-o | --output-file    output pack name\n"
    	"-r | --repack-file    input pack name\n"
        "\n", VERSION, argv[0]);
}

static const char short_options[] = "hd:i:o:r:";

static const struct option long_options[] = {
        { "help",           no_argument,        NULL, 'h' },
        { "items-dir",      required_argument,  NULL, 'd' },
        { "ddr-ini-file",   required_argument,  NULL, 'i' },
        { "output-file",    required_argument,  NULL, 'o' },
		{ "repack-file",    required_argument,  NULL, 'r' },
        { 0, 0, 0, 0 }
};


int getopt_proc(int argc, char **argv)
{
    for (;;)
    {
        int index, c = 0;

        c = getopt_long(argc, argv, short_options, long_options, &index);

        if (-1 == c)
            break;

        switch (c) {
        case 0: /* getopt_long() flag */
            break;

        case 'h':
            // print help
            usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);

        case 'i':
            // set directory with
            if(optarg)
            {
                items_dir=optarg;
            }

            break;

        case 'd':
            // set config file
            if(optarg)
            {
                ddr_ini_file=optarg;
            }

            break;

        case 'o':
            // set config file
            if(optarg)
            {
                output_file=optarg;
            }

            break;

        case 'r':
        	if(optarg)
        	{
        		repack_file=optarg;
        	}

        	break;

        default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    DPRINT("items_dir=%s\nddr_ini_file=%s\noutput_file=%s\n",
        items_dir, ddr_ini_file, output_file);

    if( (!items_dir && !repack_file) || !output_file)
    {
        usage(stderr, argc, argv);
        exit(EXIT_FAILURE);
    }

    if (items_dir && repack_file)
    {
    	fprintf(stdout, "Incompatible options\n");
    	exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	int exitcode;

    getopt_proc(argc, argv);

    if (!repack_file)
    	exitcode=nucpack_create(items_dir, item, item_sz, ddr_ini_file, output_file);
    else
    	exitcode=nucpack_repack(repack_file, ddr_ini_file, output_file);

    return exitcode;
}
