#include <stdio.h>

#include "bpsparser.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Not enough arguments passed!\n");
        printf("Run using ./binary-parser [binary-file] [bps-format]\n");
        return -1;
    }

    char *bin_file = argv[1];
    char *script_file = argv[2];

    BPSParser bps(script_file);

    bps.parse_binary(bin_file);
    bps.print();
    return 0;
}