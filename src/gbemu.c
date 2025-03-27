#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "emu.h"

static const char *program = NULL;

static void usage(FILE *outstream)
{
    fprintf(outstream, "Usage: %s <filename>\n\n", program);
}

int main(int argc, const char *argv[])
{
    const char *filename;

    program = shift(argc, argv);

    filename = shift(argc, argv);
    if (filename == NULL)
    {
        usage(stderr);
        fprintf(stderr, "No file provided\n");
        return 1;
    }

    struct emulator emu = {0};
    return (emu_run(&emu, filename) == true ? EXIT_SUCCESS : EXIT_FAILURE);
}
