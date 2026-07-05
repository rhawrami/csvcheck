#include <stdlib.h>
#include <stdio.h>

#define PROGRAM_NAME "csvcheck"

// `usage` prints how to use `csvcheck`.
void usage(void) {
    printf("\
usage: %s [flags ...] [file ...]\n", 
PROGRAM_NAME);

    printf("\
flags:\n\
-1:\n\
    descrip: only report first error per file, then exit\n\
    default: not set\n\
\n\
-d <delim>:\n\
    descrip: delimiter character (e.g. ',' | '\\t' | ':')\n\
    default: ','\n\
\n\
-b <line_break>:\n\
    descrip: line break identifier (e.g. '\\n' | '\\r\\n')\n\
    default: inferred from first line\n\
\n\
-n <n_columns>:\n\
    descrip: expected number of columns (e.g. '5' | '10')\n\
    default: inferred from first line\n\
");
}

int main(int argc, char **argv) {
    return EXIT_SUCCESS;
}