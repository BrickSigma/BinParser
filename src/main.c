#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "section.h"
#include "attribute.h"

typedef enum LineType
{
    SECTION,
    ATTRIBUTE,
    IGNORE
} LineType;

/**
 * Read a line from a file.
 *
 * This returns a string allocated with malloc,
 * make sure to `free` it later!
 *
 * @return A pointer to the line string, or `NULL` if EOF reached or on error with errno set.
 */
char *readline(FILE *file)
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
        return NULL;
    }

    char *line = (char *)malloc(sizeof(char) * (len + 1));
    if (line == NULL)
    {
        return NULL;
    }

    fseek(file, current_position, SEEK_SET);
    for (size_t i = 0; i < len; i++)
    {
        line[i] = (char)fgetc(file);
    }
    line[len] = 0;
    fseek(file, end_position, SEEK_SET);

    return line;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Not enough arguments passed!\n");
        printf("Run using ./binary-parser [binary-file] [bps-format]\n");
        return -1;
    }

    char *bin_file = argv[1];
    char *script_file = argv[2];

    FILE *file_ptr = fopen(script_file, "r");
    if (file_ptr == NULL)
    {
        perror("Could not open file");
        return -1;
    }

    char *line = NULL;

    // Current offset in the binary file
    size_t offset = 0;

    SectionList *sections = SectionList_Create();
    if (sections == NULL)
    {
        perror("Could not create sections list");
        return -1;
    }

    // Parse the BPS file
    while (1)
    {
        line = readline(file_ptr);
        if (line == NULL)
        {
            break;
        }

        // Check what type the line is
        LineType type;
        switch (line[0])
        {
        case '[':
            type = SECTION;
            break;
        case '\n':
            type = IGNORE;
            break;
        case '\r':
            type = IGNORE;
            break;
        default:
            type = ATTRIBUTE;
            break;
        }

        // Parse the line based on it's type
        switch (type)
        {
        case SECTION:
            Section *new_section = Section_CreateFromString(line, offset);
            if (new_section == NULL)
            {
                perror("Could not create section");
                break;
            }
            offset = new_section->offset;

            SectionList_Add(sections, new_section);
            break;

        case ATTRIBUTE:
            Section *section = SectionList_GetLastSection(sections);

            if (section == NULL)
            {
                printf("No section has been defined yet! Skipping attribute...\n");
                break;
            }

            Attribute *new_attribute = Attribute_CreateFromString(line, offset, section->offset);
            if (new_attribute == NULL)
            {
                perror("Could not create attribute entry");
                break;
            }
            offset += new_attribute->len;

            AttributeList_Add(section->attributes, new_attribute);

            break;
        default:
            printf("Unknown line read: %s", line);
            break;
        }

        free(line);
    }

    printf("\n");

    SectionList_ParseBinaryFile(sections, bin_file);
    SectionList_Print(sections);

    SectionList_Destroy(sections);
    fclose(file_ptr);
    return 0;
}