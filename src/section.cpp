#include "section.hpp"

#include <iostream>
#include <format>

#include <stdlib.h>
#include <string.h>

Section::Section(const char *name, size_t offset) : attributes(), offset(offset)
{
    strncpy(this->name, name, MAX_SECTION_NAME_LEN - 1);
}

Section Section::create_from_sting(const char *line, size_t last_offset)
{
    // Create a copy of the line string for strtok
    char *line_copy{new char[strlen(line) + 1]{}};
    strncpy(line_copy, line, strlen(line));

    char *name = strtok(line_copy, " [:]\r\n");
    char *section_offset_str = strtok(NULL, " [:]\r\n");
    size_t section_offset;
    if (section_offset_str == NULL || strlen(section_offset_str) == 0)
    {
        section_offset = last_offset;
    }
    else
    {
        if (
            section_offset_str[0] < '0' ||
            section_offset_str[0] > '9')
        {
            section_offset = last_offset; // TODO: Handle relative offsets
        }
        else
        {
            section_offset = strtoul(section_offset_str, NULL, 0);
        }
    }

    Section section(name, section_offset);
    delete[] line_copy;

    return section;
}

void Section::add_attribute(std::unique_ptr<Attribute> attribute)
{
    this->attributes.push_back(std::move(attribute));
}

void Section::print() const
{
    std::cout << std::format("{}: 0x{:08x}\n", this->name, this->offset);
    for (const std::unique_ptr<Attribute> &attribute : this->attributes)
    {
        attribute->print();
    }
    std::cout << "\n";
}

Section::~Section() {}