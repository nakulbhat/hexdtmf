#ifndef ARGS_H
#define ARGS_H

#include <string.h>

/**
 * Parses command line arguments and sets global flags/variables.
 */
void parse_args(int argc, char **argv);

/* Helper macro for string comparison */
#define ACMP(x, y) (strcmp((x), (y)) == 0)

#endif /* ARGS_H */
