#include "attribute.hpp"

#include <iostream>
#include <format>

#include <string.h>

static const char *const ATTRIBUTE_TYPE_STR_NUM = "num";
static const char *const ATTRIBUTE_TYPE_STR_STR = "str";
static const char *const ATTRIBUTE_TYPE_STR_HEX = "hex";
static const char *const ATTRIBUTE_TYPE_STR_BIN = "bin";
static const char *const ATTRIBUTE_TYPE_STR_SKIP = "skip";

std::unique_ptr<Attribute> create_attribute_from_string(const char *const line, std::streamoff last_offset, std::streamoff last_section_offset)
{
    // Create a copy of the line string for strtok
    char *line_copy{new char[strlen(line) + 1]{}};
    strncpy(line_copy, line, strlen(line));

    // Get the attribute name
    char *name = strtok(line_copy, " :\r\n");
    char *attribute_size_str = strtok(NULL, " :\r\n");
    std::streamoff attribute_size;
    if (attribute_size_str == NULL || strlen(attribute_size_str) == 0 || strtol(attribute_size_str, NULL, 0) <= 0)
    {
        throw "Attribute size is not valid";
    }

    attribute_size = strtol(attribute_size_str, NULL, 0);

    // Get the section offset in the attribute
    std::streamoff attribute_offset = last_offset - last_section_offset;

    Attribute *attribute;
    char *attribute_type_str = strtok(NULL, " :\r\n");
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
}

Attribute::Attribute(const char *name, std::streamoff offset, std::streamoff size) : offset(offset), size(size)
{
    strncpy(this->name, name, MAX_ATTRIBUTE_NAME_LEN - 1);
}

Attribute::~Attribute() {}

void Attribute::print() const
{
    std::cout << std::format("    {:04x}: {} : {} = ", this->offset, this->size, this->name);
}

// IntAttribute declarations

IntAttribute::IntAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size) {}
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

void IntAttribute::print() const
{
    Attribute::print();
    if (this->invalid)
        std::cout << "invalid\n";
    else
        std::cout << this->value << "\n";
}

// StrAttribute declarations

StrAttribute::StrAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size)
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
    strncpy(this->str, reinterpret_cast<const char *>(bytes), this->size);
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

HexAttribute::HexAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size)
{
    this->bytes = new uint8_t[size]{};
}

HexAttribute::~HexAttribute()
{
    delete[] this->bytes;
    this->bytes = nullptr;
}

void HexAttribute::set_value(const uint8_t *bytes)
{
    memcpy(this->bytes, bytes, this->size);
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
            std::cout << std::format("{:02x} ", this->bytes[i]);
        }
        std::cout << "\n";
    }
}

// BinaryAttribute declarations

BinaryAttribute::BinaryAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size)
{
    this->bytes = new uint8_t[size]{};
}

BinaryAttribute::~BinaryAttribute()
{
    delete[] this->bytes;
    this->bytes = nullptr;
}

void BinaryAttribute::set_value(const uint8_t *bytes)
{
    memcpy(this->bytes, bytes, this->size);
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
            print_binary(this->bytes[i]);
        }
        std::cout << "\n";
    }
}

// SkipAttribute declarations

SkipAttribute::SkipAttribute(const char *name, std::streamoff offset, std::streamoff size) : Attribute(name, offset, size) {}
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