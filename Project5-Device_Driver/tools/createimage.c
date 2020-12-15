#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

/* structure to store command line options */
static struct {
    int vm;
    int extended;
} options;

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt, ...);
static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp);
static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr);
static void write_segment(Elf64_Phdr phdr, FILE * fp,
                          FILE * img, int *nbytes);
static void write_os_size(int nbytes, FILE * img);

int main(int argc, char **argv)
{
    char *progname = argv[0];

    /* process command line options */
    options.vm = 0;
    options.extended = 0;
    while ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == '-')) {
        char *option = &argv[1][2];

        if (strcmp(option, "vm") == 0) {
            options.vm = 1;
        } else if (strcmp(option, "extended") == 0) {
            options.extended = 1;
        } else {
            error("%s: invalid option\nusage: %s %s\n", progname,
                  progname, ARGS);
        }
        argc--;
        argv++;
    }
    if (options.vm == 1) {
        error("%s: option --vm not implemented\n", progname);
    }
    if (argc < 3) {
        /* at least 3 args (createimage bootblock kernel) */
        error("usage: %s %s\n", progname, ARGS);
    }
    create_image(argc - 1, argv + 1);
    return 0;
}

static void create_image(int nfiles, char *files[])
{
    int ph, nbytes = 0;
    FILE *fp, *img;
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr;

    /* open the image file */
	img = fopen(IMAGE_FILE, "wb");
	
    /* for each input file */
    while (nfiles-- > 0) {

        /* open input file */
		fp = fopen(*files, "rb");
		
        /* read ELF header */
        read_ehdr(&ehdr, fp);
        printf("0x%04lx: %s\n", ehdr.e_entry, *files);

        /* for each program header */
        for (ph = 0; ph < ehdr.e_phnum; ph++) {

            /* read program header */
            read_phdr(&phdr, fp, ph, ehdr);

            /* write segment to the image */
            write_segment(phdr, fp, img, &nbytes);
            
            /* print info of segment */
            if(options.extended) {
	        	printf("\tsegment %d\n", ph);
				printf("\t\toffset 0x%04lx\t\tvaddr 0x%08lx\n", phdr.p_offset,phdr.p_vaddr);
				printf("\t\tfilesz 0x%04lx\t\tmemsz 0x%04lx\n", phdr.p_filesz,phdr.p_memsz);
				printf("\t\twriting 0x%04lx bytes\n", phdr.p_memsz);
				printf("\t\tpadding up to 0x%04x\n", nbytes);
			}
        }

        fclose(fp);
        files++;
    }

    write_os_size(nbytes, img);
    fclose(img);
    
    return;
}

static void read_ehdr(Elf64_Ehdr * ehdr, FILE * fp)
{
	fread(ehdr, sizeof(Elf64_Ehdr), 1, fp);
	return;
}

static void read_phdr(Elf64_Phdr * phdr, FILE * fp, int ph,
                      Elf64_Ehdr ehdr)
{
	fseek(fp, ehdr.e_phoff + ph * (sizeof(Elf64_Phdr)), SEEK_SET);
	fread(phdr, sizeof(Elf64_Phdr), 1, fp);
	return;
}

static void write_segment(Elf64_Phdr phdr, FILE * fp,
                          FILE * img, int *nbytes)
{
	/* 512 bytes align */
	int bytes;
	if(phdr.p_memsz % 512 == 0) {
		bytes = phdr.p_memsz;
	}
	else {
		bytes = phdr.p_memsz + (512 - phdr.p_memsz % 512);
	}
	
	/* initialize the data array */
	char segment_file[bytes];
	int i;
	for(i = 0; i < bytes; i++) {
		segment_file[i] = 0;
	}
	
	/* write data to img */
	fseek(fp, phdr.p_offset, SEEK_SET);
	fread(segment_file, phdr.p_filesz, 1, fp);
	fseek(img, (*nbytes), SEEK_SET);
	fwrite(segment_file, bytes, 1, img);
	(*nbytes) += bytes;
	
	return;
}

static void write_os_size(int nbytes, FILE * img)
{
	long OS_SIZE_LOC = 508;
	nbytes = nbytes / 512 - 1;
	fseek(img, OS_SIZE_LOC, SEEK_SET);
	fwrite(&nbytes, 2, 1, img);
	printf("os_size: %d sectors\n", nbytes);
	return;
}

/* print an error message and exit */
static void error(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    if (errno != 0) {
        perror(NULL);
    }
    exit(EXIT_FAILURE);
}