#include "section.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Section *Section_Create(const char *name, size_t offset)
{
    Section *section = (Section *)malloc(sizeof(Section));
    if (section == NULL)
    {
        return NULL;
    }
    strncpy(section->name, name, MAX_SECTION_NAME_LEN - 1);
    section->offset = offset;
    section->next = NULL;
    section->attributes = AttributeList_Create();
    if (section->attributes == NULL)
    {
        free(section);
        return NULL;
    }
    return section;
}

Section *Section_CreateFromString(const char *line, size_t last_offset)
{
    // Create a copy of the line string for strtok
    char *line_copy = (char *)calloc(strlen(line) + 1, sizeof(char));
    if (line_copy == NULL)
    {
        return NULL;
    }
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

    // Add the section to the list of sections
    Section *section = Section_Create(name, section_offset);

    free(line_copy);

    return section;
}

void Section_Print(Section *section)
{
    printf("%s: 0x%08x\n", section->name, section->offset);
    AttributeList_Print(section->attributes);
    printf("\n");
}

void Section_Destroy(Section *section)
{
    printf("Freeing %s section\n", section->name);
    AttributeList_Destroy(section->attributes);
    free(section);
}

SectionList *SectionList_Create()
{
    SectionList *list = (SectionList *)malloc(sizeof(SectionList));
    if (list == NULL)
    {
        return NULL;
    }

    list->len = 0;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

void SectionList_Add(SectionList *list, Section *section)
{
    if (list->len == 0)
    {
        list->head = section;
        list->tail = section;
    }
    else
    {
        list->tail->next = section;
        list->tail = section;
    }

    list->len++;
}

Section *SectionList_GetLastSection(SectionList *list)
{
    return list->tail;
}

void SectionList_ParseBinaryFile(SectionList *list, char *file)
{
    printf("Parsing %s...\n", file);
    FILE *bin_file = fopen(file, "rb");
    if (bin_file == NULL)
    {
        perror("Could not open file");
        return;
    }

    if (list->len == 0)
    {
        return;
    }

    fseek(bin_file, 0, SEEK_END);
    long file_size = ftell(bin_file);
    fseek(bin_file, 0, SEEK_SET);

    printf("File size: %lu\n", file_size);

    Section *next_section = list->head;
    do
    {
        Section *current_section = next_section;
        printf("Parsing %s section at offset 0x%08x...\n", current_section->name, current_section->offset);
        if (current_section->offset >= file_size)
        {
            printf("%s section does not lie in the file, skipping...\n", current_section->name);
            // Set all attributes to be skipped
            Attribute *next_attribute = current_section->attributes->head;
            do
            {
                Attribute *current_attribute = next_attribute;
                current_attribute->type = ATTR_TYPE_SKIP;
                next_attribute = current_attribute->next;
            } while (next_attribute != NULL);
            next_section = current_section->next;
            continue;
        }

        if (current_section->attributes->len == 0)
        {
            next_section = current_section->next;
            continue; // Skip the section if not attributes are present
        }

        fseek(bin_file, current_section->offset, SEEK_SET);

        Attribute *next_attribute = current_section->attributes->head;
        do
        {
            Attribute *current_attribute = next_attribute;
            if (current_attribute->offset + current_section->offset >= file_size)
            {
                printf("%s attribute does not lie in the file, skipping...\n");
                current_attribute->type = ATTR_TYPE_SKIP;
                break;
            }
            switch (current_attribute->type)
            {
            case ATTR_TYPE_INT:
                current_attribute->value.int_val.value = 0;
                fread(&(current_attribute->value.int_val.value), current_attribute->len, 1, bin_file);
                break;
            case ATTR_TYPE_STR:
                char *str = (char *)calloc(current_attribute->len + 1, sizeof(char));
                fread(str, sizeof(char), current_attribute->len, bin_file);
                current_attribute->value.str_val.str = str;
                break;

            case ATTR_TYPE_HEX: // Both hex and binary values are read in the same way
            case ATTR_TYPE_BIN:
                uint8_t *array = (uint8_t *)calloc(current_attribute->len, sizeof(uint8_t));
                fread(array, sizeof(uint8_t), current_attribute->len, bin_file);
                current_attribute->value.byte_val.array = array;
                break;

            case ATTR_TYPE_INVALID:
            case ATTR_TYPE_SKIP:
                fseek(bin_file, current_attribute->len, SEEK_CUR);
                break;
            }
            next_attribute = current_attribute->next;
        } while (next_attribute != NULL);

        next_section = current_section->next;
    } while (next_section != NULL);

    list->parsed_file = true;
    fclose(bin_file);
}

void SectionList_Print(SectionList *list)
{
    if (!list->parsed_file)
    {
        printf("No binary file has been parsed yet!\n");
        return;
    }
    if (list->len == 0)
    {
        return;
    }

    Section *next_section = list->head;
    do
    {
        Section *current_section = next_section;
        Section_Print(current_section);
        next_section = current_section->next;
    } while (next_section != NULL);
}

void SectionList_Destroy(SectionList *list)
{
    if (list->len != 0)
    {
        Section *next_section = list->head;
        do
        {
            Section *current_section = next_section;
            next_section = current_section->next;
            Section_Destroy(current_section);
        } while (next_section != NULL);
    }

    free(list);
}