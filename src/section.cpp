#include "section.hpp"

#include <algorithm>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>
#include <sstream>

#include "utils.hpp"

Section::Section(const char *name, std::streamoff offset) : attributes(), offset(offset)
{
    copy_string(this->name, MAX_SECTION_NAME_LEN, name, MAX_SECTION_NAME_LEN - 1);
}

Section::Section(const char *name, size_t section_ptr, size_t attribute_ptr, std::streamoff scale) : section_ptr(section_ptr), attribute_ptr(attribute_ptr), scale(scale)
{
    this->offset_is_ptr = true;
    copy_string(this->name, MAX_SECTION_NAME_LEN, name, MAX_SECTION_NAME_LEN - 1);
}

Section Section::create_from_sting(const char *const line, std::streamoff last_offset, const std::vector<Section> &sections)
{
    // Create a copy of the line string for strtok
    std::unique_ptr<char> line_copy{new char[strlen(line) + 1]{}};
    copy_string(line_copy.get(), strlen(line) + 1, line, strlen(line));

    std::vector<char *> tokens = split_string(line_copy.get(), " [:]");

    if (tokens.size() < 1)
        throw "Section header is empty";

    const char *name = tokens[0];

    if (tokens.size() == 1)
    {
        // No offset was specified, so use the last offset passed in.
        return Section(name, last_offset);
    }

    // Check if the section offset is a number or pointer.
    // We only need to check the first character as the regex before guarantees that
    // the if the string doesn't start with a number, it's a pointer.
    if (tokens[1][0] < '0' || tokens[1][0] > '9')
    {
        std::streamoff scale = 1;

        // First, get the section and attribute being referenced
        std::vector<char *> reference = split_string(tokens[1], " .");
        char *section_name = reference[0];
        char *section_attribute = reference[1];

        // Find the section that matches the section name
        auto found_section = std::find_if(sections.begin(), sections.end(),
                                          [section_name](const Section &section)
                                          {
                                              return std::strcmp(section.name, section_name) == 0;
                                          });

        if (found_section == sections.end())
        {
            std::ostringstream error{};
            error << std::format("No section with the name '{}' has been declared before this line!", section_name);
            throw error.str();
        }

        size_t section_ptr = found_section - sections.begin();

        const Section &section = sections[section_ptr];

        // Now find the attribute in the section that matches the attribute name
        auto found_attribute = std::find_if(section.attributes.begin(), section.attributes.end(),
                                            [section_attribute](const std::unique_ptr<Attribute> &attribute)
                                            {
                                                return std::strcmp(attribute->name, section_attribute) == 0;
                                            });

        if (found_attribute == section.attributes.end())
        {
            std::ostringstream error{};
            error << std::format("No attribute with the name '{}' exists in '{}' section", section_attribute, section_name);
            throw error.str();
        }

        size_t attribute_ptr = found_attribute - section.attributes.begin();

        // Verify the attribute type is an Int
        const std::unique_ptr<Attribute>& attribute = section.attributes[attribute_ptr];
        if (attribute->type != AttributeType::Int)
        {
            std::ostringstream error{};
            error << std::format("{}.{} is not a numerical/integer type!", section_name, section_attribute);
            throw error.str();
        }

        // We need to determine if the pointer is followed by a scale as well.
        // This is always true if the number of `tokens` is exactly 4.
        if (tokens.size() == 4)
            scale = std::strtol(tokens[3], nullptr, 0);

        return Section(name, section_ptr, attribute_ptr, scale);
    }

    std::streamoff section_offset = std::strtol(tokens[1], nullptr, 0);
    return Section(name, section_offset);
}

void Section::add_attribute(std::unique_ptr<Attribute> attribute)
{
    this->attributes.push_back(std::move(attribute));
}

void Section::initialize_offset(const std::vector<Section> &sections)
{
    if (!this->offset_is_ptr)
        return; // Don't do anything if the section offset isn't a ptr

    const Section &section = sections[this->section_ptr];
    const Attribute* attribute = section.attributes[this->attribute_ptr].get();
    // We can safely cast the attribute to an IntAttribute as it was verified during the section construction
    const IntAttribute *int_attribute = static_cast<const IntAttribute *>(attribute);

    this->offset = static_cast<std::streamoff>(int_attribute->get_value()) * this->scale;
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