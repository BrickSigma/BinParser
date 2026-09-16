#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <stddef.h>
#include <stdint.h>

#define MAX_ATTRIBUTE_NAME_LEN (256)

// Integer sizes
typedef enum IntAttributeSize
{
    INT_1_BYTE = 1,
    INT_2_BYTE = 2,
    INT_4_BYTE = 4,
    INT_8_BYTE = 8,
} IntAttributeSize;

typedef enum AttributeType
{
    ATTR_TYPE_INT,
    ATTR_TYPE_STR,
    ATTR_TYPE_HEX,
    ATTR_TYPE_BIN,
    ATTR_TYPE_SKIP,   // Used to skip attributes
    ATTR_TYPE_INVALID // Represents invalid attributes
} AttributeType;

// Integer attribute
typedef struct IntAttribute
{
    uint64_t value;
} IntAttribute;

typedef struct StrAttribute
{
    char *str;
} StrAttribute;

// Used for hexadecimal and binary types
typedef struct ByteArrayAttribute
{
    uint8_t *array;
} ByteArrayAttribute;

typedef union AttributeValue
{
    IntAttribute int_val;        // Integer value
    StrAttribute str_val;        // String value
    ByteArrayAttribute byte_val; // Byte array
} AttributeValue;

typedef struct Attribute
{
    char name[MAX_ATTRIBUTE_NAME_LEN]; // Attribute name
    size_t offset;                     // Offset of the attribute relative to the section
    size_t len;                        // Number of bytes the attribute uses
    AttributeType type;                // The type of attribute stored
    AttributeValue value;              // The value of the attribute
    void *next;                        // Pointer to next attribute
} Attribute;

typedef struct AttributeList
{
    size_t len;
    Attribute *head;
    Attribute *tail;
} AttributeList;

// Attribute related functions

Attribute *Attribute_Create(const char *name, size_t offset, size_t len, AttributeType type, AttributeValue value);

// Create a Attribute from a Attribute string in the file
Attribute *Attribute_CreateFromString(const char *line, size_t last_offset, size_t last_section_offset);

// Print the attribute's details
void Attribute_Print(Attribute *attribute);

// Free any allocated memory for an attribute
void Attribute_Destroy(Attribute *attribute);

// Create an attributes list
AttributeList *AttributeList_Create();

// Add an attribute to a list
void AttributeList_Add(AttributeList *list, Attribute *attribute);

// Print the attributes in a list
void AttributeList_Print(AttributeList *list);

// Free any allocated memory
void AttributeList_Destroy(AttributeList *list);

#endif // ATTRIBUTE_H