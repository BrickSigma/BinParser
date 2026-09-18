#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <cstdint>
#include <memory>

#define MAX_ATTRIBUTE_NAME_LEN (256)

class Attribute
{
public:
    char name[MAX_ATTRIBUTE_NAME_LEN]; // Attribute name
    std::streamoff offset;             // Offset of the attribute relative to the start of a section
    std::streamoff size;               // Size of the attribute in bytes
    bool invalid = false;              // Indicates if the field is invalid

    // Attribute constructor
    Attribute(const char *name, std::streamoff offset, std::streamoff size);
    virtual ~Attribute() = 0;
    // Set the attribute's value using an array of bytes
    virtual void set_value(const uint8_t *bytes) = 0;
    // Print the attribute's details
    virtual void print() const;
};

// Attribute Factory method (no need to put in a class though)
std::unique_ptr<Attribute> create_attribute_from_string(const char *const line, std::streamoff last_offset, std::streamoff last_section_offset);

// Integer attribute type
class IntAttribute : public Attribute
{
private:
    uint64_t value = 0;

public:
    IntAttribute(const char *name, std::streamoff offset, std::streamoff size);
    ~IntAttribute();
    void set_value(const uint8_t *bytes) override;
    void print() const override;
};

// String attribute type
class StrAttribute : public Attribute
{
private:
    char *str = nullptr;

public:
    StrAttribute(const char *name, std::streamoff offset, std::streamoff size);
    ~StrAttribute();
    void set_value(const uint8_t *bytes) override;
    void print() const override;
};

// Hex attribute type
class HexAttribute : public Attribute
{
private:
    uint8_t *bytes = nullptr;

public:
    HexAttribute(const char *name, std::streamoff offset, std::streamoff size);
    ~HexAttribute();
    void set_value(const uint8_t *bytes) override;
    void print() const override;
};

// Binary attribute type
class BinaryAttribute : public Attribute
{
private:
    uint8_t *bytes = nullptr;

public:
    BinaryAttribute(const char *name, std::streamoff offset, std::streamoff size);
    ~BinaryAttribute();
    void set_value(const uint8_t *bytes) override;
    void print() const override;
};

// Skip attribute type
class SkipAttribute : public Attribute
{
public:
    SkipAttribute(const char *name, std::streamoff offset, std::streamoff size);
    ~SkipAttribute();
    void set_value(const uint8_t *bytes) override;
    void print() const override;
};

#endif // ATTRIBUTE_H