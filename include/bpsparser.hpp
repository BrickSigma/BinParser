#ifndef BPSPARSER_HPP
#define BPSPARSER_HPP

#include <vector>

#include "section.hpp"

class BPSParser
{
private:
    std::vector<Section> sections;   // List of all sections
    bool binary_file_parsed = false; // Indicates if the file has been parsed yet

    // Parsed line type
    typedef enum class LineType
    {
        SECTION,
        ATTRIBUTE,
        IGNORE
    } LineType;

public:
    // Create a BPS Parser instance from a BPS file
    BPSParser(const char *bps_file);

    ~BPSParser();

    // Parse a binary file
    void parse_binary(const char *file);

    void print() const;
};

#endif // BPSPARSER_HPP