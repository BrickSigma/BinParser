#include "attribute.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Attribute related functions
Attribute *Attribute_Create(
    const char *name,
    size_t offset,
    size_t len,
    AttributeType type,
    AttributeValue value)
{
    Attribute *attribute = (Attribute *)malloc(sizeof(Attribute));
    if (attribute == NULL)
    {
        return NULL;
    }
    strncpy(attribute->name, name, MAX_ATTRIBUTE_NAME_LEN - 1);
    attribute->offset = offset;
    attribute->len = len;
    attribute->type = type;
    attribute->value = value;
    attribute->next = NULL;
    return attribute;
}

static const char *ATTRIBUTE_TYPE_STR_NUM = "num";
static const char *ATTRIBUTE_TYPE_STR_STR = "str";
static const char *ATTRIBUTE_TYPE_STR_HEX = "hex";
static const char *ATTRIBUTE_TYPE_STR_BIN = "bin";
static const char *ATTRIBUTE_TYPE_STR_SKIP = "skip";

Attribute *Attribute_CreateFromString(const char *line, size_t last_offset, size_t last_section_offset)
{
    // Create a copy of the line string for strtok
    char *line_copy = (char *)calloc(strlen(line) + 1, sizeof(char));
    if (line_copy == NULL)
    {
        return NULL;
    }
    strncpy(line_copy, line, strlen(line));

    // Get the attribute name
    char *name = strtok(line_copy, " :\r\n");
    char *attribute_size_str = strtok(NULL, " :\r\n");
    size_t attribute_size;
    if (attribute_size_str == NULL || strlen(attribute_size_str) == 0 || strtoul(attribute_size_str, NULL, 0) == 0)
    {
        printf("Attribute size is not valid!\n");
        free(line_copy);
        return NULL;
    }

    attribute_size = strtoul(attribute_size_str, NULL, 0);

    AttributeType attribute_type;
    char *attribute_type_str = strtok(NULL, " :\r\n");
    if (attribute_type_str == NULL || strlen(attribute_type_str) == 0)
    {
        printf("Attribute type is not specified!\n");
        attribute_type = ATTR_TYPE_INVALID;
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_NUM) == 0)
    {
        attribute_type = ATTR_TYPE_INT;
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_STR) == 0)
    {
        attribute_type = ATTR_TYPE_STR;
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_HEX) == 0)
    {
        attribute_type = ATTR_TYPE_HEX;
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_BIN) == 0)
    {
        attribute_type = ATTR_TYPE_BIN;
    }
    else if (strcmp(attribute_type_str, ATTRIBUTE_TYPE_STR_SKIP) == 0)
    {
        attribute_type = ATTR_TYPE_SKIP;
    }
    else
    {
        attribute_type = ATTR_TYPE_INVALID;
    }

    if (attribute_type == ATTR_TYPE_INT && !(attribute_size == 1 || attribute_size == 2 || attribute_size == 4 || attribute_size == 8))
    {
        printf("Attribute of type `num` must be of sizes 1, 2, 4, or 8 bytes!\n");
        attribute_type = ATTR_TYPE_INVALID;
    }

    // Get the section offset in the attribute
    size_t attribute_offset = last_offset - last_section_offset;

    // Dummy value for attribute value
    AttributeValue *attribute_value = (AttributeValue *)calloc(1, sizeof(AttributeValue));

    Attribute *attribute = Attribute_Create(name, attribute_offset, attribute_size, attribute_type, *attribute_value);

    free(attribute_value);

    free(line_copy);
    return attribute;
}

// Print a byte in binary
static void print_binary(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        printf("%d", (byte >> i) & 1);
    }
}

void Attribute_Print(Attribute *attribute)
{
    printf("    %04x: %lu : %s = ", attribute->offset, attribute->len, attribute->name);
    size_t len = attribute->len;
    switch (attribute->type)
    {
    case ATTR_TYPE_INT:
        printf("%ld", attribute->value.int_val.value);
        break;
    case ATTR_TYPE_STR:
        printf("%s", attribute->value.str_val.str);
        break;
    case ATTR_TYPE_HEX:
        printf("0x");
        for (size_t i = 0; i < len; i++)
        {
            printf("%02x ", attribute->value.byte_val.array[i]);
        }
        break;
    case ATTR_TYPE_BIN:
        for (size_t i = 0; i < len; i++)
        {
            print_binary(attribute->value.byte_val.array[i]);
        }
        break;
    case ATTR_TYPE_SKIP:
        printf("skipped");
        break;
    case ATTR_TYPE_INVALID:
        printf("invalid");
        break;
    }
    printf("\n");
}

void Attribute_Destroy(Attribute *attribute)
{
    printf("\tFreeing %s attribute\n", attribute->name);

    switch (attribute->type)
    {
    case ATTR_TYPE_STR:
        free(attribute->value.str_val.str);
        break;
    case ATTR_TYPE_HEX:
        free(attribute->value.byte_val.array);
        break;
    case ATTR_TYPE_BIN:
        free(attribute->value.byte_val.array);
        break;
    default:
        break;
    }

    free(attribute);
}

AttributeList *AttributeList_Create()
{
    AttributeList *list = (AttributeList *)malloc(sizeof(AttributeList));
    if (list == NULL)
    {
        return NULL;
    }

    list->len = 0;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

void AttributeList_Add(AttributeList *list, Attribute *attribute)
{
    if (list->len == 0)
    {
        list->head = attribute;
        list->tail = attribute;
    }
    else
    {
        list->tail->next = attribute;
        list->tail = attribute;
    }

    list->len++;
}

void AttributeList_Print(AttributeList *list)
{
    if (list->len == 0)
    {
        return;
    }

    Attribute *next_attribute = list->head;
    do
    {
        Attribute *current_attribute = next_attribute;
        Attribute_Print(current_attribute);
        next_attribute = current_attribute->next;
    } while (next_attribute != NULL);
}

void AttributeList_Destroy(AttributeList *list)
{
    if (list->len != 0)
    {
        Attribute *next_attribute = list->head;
        do
        {
            Attribute *current_attribute = next_attribute;
            next_attribute = current_attribute->next;
            Attribute_Destroy(current_attribute);
        } while (next_attribute != NULL);
    }

    free(list);
}