#include <iostream>

#include "bpsparser.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Not enough arguments passed!\n";
        std::cout << "Run using ./binary-parser [binary-file] [bps-format]\n";
        return -1;
    }

    char *bin_file = argv[1];
    char *script_file = argv[2];

    try
    {
        BPSParser bps(script_file);

        bps.parse_binary(bin_file);
        bps.print();
    }
    catch (const char *error)
    {
        std::cerr << "Error: " << error << "\n";
        return -1;
    }
    catch (std::string error)
    {
        std::cerr << error << "\n";
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unknown error occured!\n";
        return -1;
    }

    return 0;
}