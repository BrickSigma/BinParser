#ifndef SECTION_H
#define SECTION_H

#define MAX_SECTION_NAME_LEN (256)

#include <vector>
#include <memory>

#include "attribute.hpp"

class Section
{
public:
    char name[MAX_SECTION_NAME_LEN];                    // Name of the section
    std::vector<std::unique_ptr<Attribute>> attributes; // List of attributes
    std::streamoff offset;                              // Offset of the section in the file

    // Create a new section
    Section(const char *name, std::streamoff offset);

    // Section destructor
    ~Section();

    // Create a section from a section string in the BPS file
    static Section create_from_sting(const char *const line, std::streamoff last_offset);

    // Add an attribute to the section
    void add_attribute(std::unique_ptr<Attribute> attribute);

    // Print the section's details
    void print() const;

    // Make the section move-only

    Section(const Section &) = delete;
    Section &operator=(const Section &) = delete;

    Section(Section &&) = default;
    Section &operator=(Section &&section) = default;
};

#endif // SECTION_H