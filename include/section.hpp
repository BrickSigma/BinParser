#ifndef SECTION_H
#define SECTION_H

#define MAX_SECTION_NAME_LEN (256)

#include <vector>
#include <memory>
#include <cstddef>

#include "attribute.hpp"

class Section
{
public:
    char name[MAX_SECTION_NAME_LEN];                    // Name of the section
    std::vector<std::unique_ptr<Attribute>> attributes; // List of attributes
    std::streamoff offset = 0;                              // Offset of the section in the file

    bool offset_is_ptr = false; // Indicates if the section offset is determined by another section.attribute
    size_t section_ptr = 0;     // Index of referenced section in sections list
    size_t attribute_ptr = 0;   // Index of referenced attribute in section
    std::streamoff scale = 1;   // Scale applied to offset

    // Create a new section with a fixed offset
    Section(const char *name, std::streamoff offset);

    // Create a new section which references another section.attribute and scale
    Section(const char *name, size_t section_ptr, size_t attribute_ptr, std::streamoff scale);

    // Section destructor
    ~Section();

    /**
     * Create a section from a section string in the BPS file.
     *
     * @param line line read from BPS file matching section header regex
     * @param last_offset last offset in the file recorded
     * @param sections list of sections already recorded, used for relative offsets (pointer dereference)
     */
    static Section create_from_sting(const char *const line, std::streamoff last_offset, const std::vector<Section> &sections);

    // Add an attribute to the section
    void add_attribute(std::unique_ptr<Attribute> attribute);

    // Initialize the offset if the section was pointed by another section.attribute
    void initialize_offset(const std::vector<Section> &sections);

    // Print the section's details
    void print() const;

    // Make the section move-only

    Section(const Section &) = delete;
    Section &operator=(const Section &) = delete;

    Section(Section &&) = default;
    Section &operator=(Section &&section) = default;
};

#endif // SECTION_H