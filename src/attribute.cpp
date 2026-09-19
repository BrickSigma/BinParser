#include "attribute.hpp"

#include <cstring>
#include <iostream>
#include <format>
#include <sstream>

#include "utils.hpp"

static const char *const ATTRIBUTE_TYPE_STR_NUM = "num";
static const char *const ATTRIBUTE_TYPE_STR_STR = "str";
static const char *const ATTRIBUTE_TYPE_STR_HEX = "hex";
static const char *const ATTRIBUTE_TYPE_STR_BIN = "bin";
static const char *const ATTRIBUTE_TYPE_STR_SKIP = "skip";

std::unique_ptr<Attribute> create_attribute_from_string(const char *const line, std::streamoff last_offset, std::streamoff last_section_offset)
{
    // Create a copy of the line string for strtok
    char *line_copy{new char[strlen(line) + 1]{}};
    copy_string(line_copy, strlen(line) + 1, line, strlen(line));

    std::vector<char*> tokens = split_string(line_copy, " :\r\n");

    // Get the attribute name
    char *name = tokens[0];
    char *attribute_size_str = tokens[1];
    std::streamoff attribute_size;
    if (attribute_size_str == NULL || strlen(attribute_size_str) == 0 || strtol(attribute_size_str, NULL, 0) <= 0)
    {
        std::ostringstream error{};
        error << std::format("{} attribute size is not valid (must be a positive, non-zero value)", name);
        throw error.str();
    }

    attribute_size = strtol(attribute_size_str, NULL, 0);

    // Get the section offset in the attribute
    std::streamoff attribute_offset = last_offset - last_section_offset;

    Attribute *attribute;
    char *attribute_type_str = tokens[2];
    if (attribute_type_str == NULL || strlen(attribute_type_str) == 0)
    {
        throw "Attribute type is invalid";
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_NUM) == 0)
    {
        if (!(attribute_size == 1 || attribute_size == 2 || attribute_size == 4 || attribute_size == 8))
            throw "Attribute of type `num` must be of size 1, 2, 4, or 8 bytes!";

        attribute = new IntAttribute(name, attribute_offset, attribute_size);
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_STR) == 0)
    {
        attribute = new StrAttribute(name, attribute_offset, attribute_size);
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_HEX) == 0)
    {
        attribute = new HexAttribute(name, attribute_offset, attribute_size);
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_BIN) == 0)
    {
        attribute = new BinaryAttribute(name, attribute_offset, attribute_size);
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_SKIP) == 0)
    {
        attribute = new SkipAttribute(name, attribute_offset, attribute_size);
    }
    else
    {
        throw "Attribute type is invalid";
    }

    delete[] line_copy;
    return std::unique_ptr<Attribute>{attribute};
}

// Print a byte in binary
static void print_binary(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        std::cout << ((byte >> i) & 1);
    }
    std::cout << " ";
}

Attribute::Attribute(const char *name, std::streamoff offset, std::streamoff size, AttributeType type) : offset(offset), size(size), type(type)
{
    copy_string(this->name, MAX_ATTRIBUTE_NAME_LEN, name, MAX_ATTRIBUTE_NAME_LEN - 1);
}

Attribute::~Attribute() {}

void Attribute::print() const
{
    std::cout << std::format("    {:04x}: {} : {} = ", this->offset, this->size, this->name);
}

// IntAttribute declarations

IntAttribute::IntAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size, AttributeType::Int) {}
IntAttribute::~IntAttribute() {}

void IntAttribute::set_value(const uint8_t *bytes)
{
    switch (this->size)
    {
    case 1:
        this->value = *bytes;
        break;
    case 2:
        this->value = *reinterpret_cast<const uint16_t *>(bytes);
        break;
    case 4:
        this->value = *reinterpret_cast<const uint32_t *>(bytes);
        break;
    case 8:
        this->value = *reinterpret_cast<const uint64_t *>(bytes);
        break;
    }
}

uint64_t IntAttribute::get_value() const
{
    return this->value;
}

void IntAttribute::print() const
{
    Attribute::print();
    if (this->invalid)
        std::cout << "invalid\n";
    else
        std::cout << this->value << "\n";
}

// StrAttribute declarations

StrAttribute::StrAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size, AttributeType::Str)
{
    this->str = new char[size + 1]{};
}

StrAttribute::~StrAttribute()
{
    delete[] this->str;
    this->str = nullptr;
}

void StrAttribute::set_value(const uint8_t *const bytes)
{
    copy_string(this->str, this->size + 1, reinterpret_cast<const char*>(bytes), this->size);
}

void StrAttribute::print() const
{
    Attribute::print();
    if (this->invalid)
        std::cout << "invalid\n";
    else
        std::cout << this->str << "\n";
}

// HexAttribute declarations

HexAttribute::HexAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size, AttributeType::Hex)
{
    this->array = new uint8_t[size]{};
}

HexAttribute::~HexAttribute()
{
    delete[] this->array;
    this->array = nullptr;
}

void HexAttribute::set_value(const uint8_t *bytes)
{
    memcpy(this->array, bytes, this->size);
}

void HexAttribute::print() const
{
    Attribute::print();
    if (this->invalid)
    {
        std::cout << "invalid\n";
    }
    else
    {
        for (std::streamoff i = 0; i < this->size; i++)
        {
            std::cout << std::format("{:02x} ", this->array[i]);
        }
        std::cout << "\n";
    }
}

// BinaryAttribute declarations

BinaryAttribute::BinaryAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size, AttributeType::Bin)
{
    this->array = new uint8_t[size]{};
}

BinaryAttribute::~BinaryAttribute()
{
    delete[] this->array;
    this->array = nullptr;
}

void BinaryAttribute::set_value(const uint8_t *bytes)
{
    memcpy(this->array, bytes, this->size);
}

void BinaryAttribute::print() const
{
    Attribute::print();
    if (this->invalid)
    {
        std::cout << "invalid\n";
    }
    else
    {
        for (std::streamoff i = 0; i < this->size; i++)
        {
            print_binary(this->array[i]);
        }
        std::cout << "\n";
    }
}

// SkipAttribute declarations

SkipAttribute::SkipAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size, AttributeType::Skip) {}
SkipAttribute::~SkipAttribute() {}
void SkipAttribute::set_value(const uint8_t *) {}

void SkipAttribute::print() const
{
    Attribute::print();
    if (this->invalid)
        std::cout << "invalid\n";
    else
        std::cout << "skipped\n";
}