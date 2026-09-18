#include "bpsparser.hpp"

#include <fstream>
#include <iostream>
#include <regex>

BPSParser::BPSParser(const char *bps_file)
{
    this->sections = std::vector<Section>();

    std::ifstream file{bps_file};

    std::regex section_header_re("^\\[\\s*\\D[\\w]*\\s*(:\\s*(0[xX][0-9a-fA-F]+|\\d+))?\\s*\\]$");
    std::regex attribute_re("^\\s*\\D[\\w]*\\s*:\\s*(0[xX][0-9a-fA-F]+|\\d+)\\s*:\\s*(num|str|hex|bin|skip)\\s*$");

    if (!file)
        throw "Could not open BPS file";

    // Current offset in the binary file
    std::streamoff offset = 0;

    std::string buffer{};
    while (std::getline(file, buffer))
    {
        if (std::regex_match(buffer, section_header_re))
        {
            Section section = Section::create_from_sting(buffer.c_str(), offset);
            offset = section.offset;
            this->sections.push_back(std::move(section));
        }
        else if (std::regex_match(buffer, attribute_re))
        {
            if (this->sections.empty())
            {
                std::cerr << "No section has been defined yet! Skipping attribute...\n";
                break;
            }

            Section &section = this->sections.back();
            std::unique_ptr<Attribute> new_attribute = create_attribute_from_string(buffer.c_str(), offset, section.offset);
            offset += new_attribute->size;
            section.add_attribute(std::move(new_attribute));
        }
        else
        {
            if (buffer.length() == 0)
                continue;
            std::cerr << "Unknown line read!\n";
        }
    }
}

BPSParser::~BPSParser() {}

void BPSParser::parse_binary(const char *file)
{
    std::ifstream bin_file{file, std::ios::binary};

    if (!bin_file)
        throw "Could not open binary file";

    if (this->sections.empty())
        return;

    bin_file.seekg(0, std::ios::end);
    std::streamoff file_size = bin_file.tellg();
    std::cout << "File size: " << file_size << "\n";
    bin_file.seekg(0, std::ios::beg);

    for (Section &section : this->sections)
    {
        if (section.offset >= file_size)
        {
            std::cout << section.name << " section offset is outside of the file, skipping...\n";
            // Set all attributes to be invalidated
            for (std::unique_ptr<Attribute> &attribute : section.attributes)
            {
                attribute->invalid = true;
            }
            continue;
        }

        if (section.attributes.empty())
            continue; // Skip the section if no attributes are present

        bin_file.seekg(section.offset, std::ios::beg);

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
                std::cout << attribute->name << " attribute offset is outside of the file, skipping all attributes in section after it...\n";
                attribute->invalid = true;
                attributes_valid = false; // Mark all further attributes as invalid
                continue;
            }

            uint8_t *bytes = new uint8_t[attribute->size]{};
            bin_file.read(reinterpret_cast<char *>(bytes), attribute->size);
            attribute->set_value(bytes);
            delete[] bytes;
        }
    }

    this->binary_file_parsed = true;
}

void BPSParser::print() const noexcept
{
    if (!this->binary_file_parsed)
    {
        std::cerr << "No binary file has been parsed yet!\n";
        return;
    }

    if (this->sections.empty())
        return;

    for (const Section &section : this->sections)
    {
        section.print();
    }
}