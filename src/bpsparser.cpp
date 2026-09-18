#include "bpsparser.hpp"

#include <stdio.h>
#include <stdlib.h>

/**
 * Read a line from a file.
 *
 * This returns a string allocated with malloc,
 * make sure to `free` it later!
 *
 * @return A pointer to the line string, or `NULL` if EOF reached or on error with errno set.
 */
static char *readline(FILE *file)
{
    size_t len = 0;
    long current_position = ftell(file);
    while (1)
    {
        int c = fgetc(file);
        if (c == EOF)
        {
            break;
        }
        if (c == '\n')
        {
            len++;
            break;
        }
        len++;
    }
    long end_position = ftell(file);

    if (len == 0)
    {
        return nullptr;
    }

    char *line{new char[len + 1]{}};

    fseek(file, current_position, SEEK_SET);
    for (size_t i = 0; i < len; i++)
    {
        line[i] = (char)fgetc(file);
    }
    line[len] = 0;
    fseek(file, end_position, SEEK_SET);

    return line;
}

BPSParser::BPSParser(const char *bps_file)
{
    this->sections = std::vector<Section>();

    FILE *file = fopen(bps_file, "r");
    if (file == nullptr)
    {
        perror("Could not open file");
        exit(-1);
    }

    char *line = nullptr;

    // Current offset in the binary file
    size_t offset = 0;

    while (true)
    {
        line = readline(file);
        if (line == nullptr)
            break;

        LineType type;
        switch (line[0])
        {
        case '[':
            type = LineType::SECTION;
            break;
        case '\n':
            type = LineType::IGNORE;
            break;
        case '\r':
            type = LineType::IGNORE;
            break;
        default:
            type = LineType::ATTRIBUTE;
            break;
        }

        switch (type)
        {
        case LineType::SECTION:
        {
            Section section = Section::create_from_sting(line, offset);
            offset = section.offset;
            this->sections.push_back(std::move(section));
        }
        break;
        case LineType::ATTRIBUTE:
        {
            if (this->sections.empty())
            {
                printf("No section has been defined yet! Skipping attribute...\n");
                break;
            }

            Section &section = this->sections.back();

            std::unique_ptr<Attribute> new_attribute = create_attribute_from_string(line, offset, section.offset);

            offset += new_attribute->size;

            section.add_attribute(std::move(new_attribute));
        }
        break;
        }

        delete[] line;
    }

    fclose(file);
}

BPSParser::~BPSParser() {}

void BPSParser::parse_binary(const char *file)
{
    FILE *bin_file = fopen(file, "rb");
    if (bin_file == NULL)
    {
        perror("Could not open file");
        return;
    }

    if (this->sections.empty())
        return;

    fseek(bin_file, 0, SEEK_END);
    long file_size = ftell(bin_file);
    fseek(bin_file, 0, SEEK_SET);

    for (Section &section : this->sections)
    {
        if (section.offset >= file_size)
        {
            printf("%s section offset is outside of the file, skipping...\n", section.name);
            // Set all attributes to be invalidated
            for (std::unique_ptr<Attribute> &attribute : section.attributes)
            {
                attribute->invalid = true;
            }
            continue;
        }

        if (section.attributes.empty())
            continue; // Skip the section if no attributes are present

        fseek(bin_file, section.offset, SEEK_SET);

        bool attributes_valid = true; // Used to indicate if the attributes read are valid or not
        for (std::unique_ptr<Attribute> &attribute : section.attributes)
        {
            if (!attributes_valid)
            {
                attribute->invalid = true;
                continue;
            }

            if (attribute->offset + section.offset >= file_size)
            {
                printf("%s attribute offset is outside of the file, skipping all attributes in section after it...\n", attribute->name);
                attribute->invalid = true;
                attributes_valid = false; // Mark all further attributes as invalid
                continue;
            }

            uint8_t *bytes = new uint8_t[attribute->size]{};
            fread(bytes, sizeof(uint8_t), attribute->size, bin_file);
            attribute->set_value(bytes);
            delete[] bytes;
        }
    }

    this->binary_file_parsed = true;
    fclose(bin_file);
}

void BPSParser::print() const
{
    if (!this->binary_file_parsed)
    {
        printf("No binary file has been parsed yet!\n");
        return;
    }

    if (this->sections.empty())
        return;

    for (const Section &section : this->sections)
    {
        section.print();
    }
}