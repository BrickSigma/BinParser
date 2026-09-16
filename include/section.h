#ifndef SECTION_H
#define SECTION_H

#define MAX_SECTION_NAME_LEN (256)

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "attribute.h"

typedef struct Section
{
    char name[MAX_SECTION_NAME_LEN]; // Name of the section
    size_t offset;                   // Offset of the section in the file
    void *next;                      // Pointer to next section
    AttributeList *attributes;       // List of attributes part of the section
} Section;

typedef struct SectionList
{
    size_t len;
    Section *head;
    Section *tail;
    bool parsed_file; // Indicates if the binary file has been parsed yet
} SectionList;

Section *Section_Create(const char *name, size_t offset);

// Create a section from a section string in the file
Section *Section_CreateFromString(const char *line, size_t last_offset);

// Print a section's details
void Section_Print(Section *section);

// Free any allocated memory for the section
void Section_Destroy(Section *section);

// Create a new list for sections
SectionList *SectionList_Create();

// Add a section to the list
void SectionList_Add(SectionList *list, Section *section);

// Get the last section added to the list
Section *SectionList_GetLastSection(SectionList *list);

// Parse a binary file and populate the section attributes with their values
void SectionList_ParseBinaryFile(SectionList *list, char *file);

// Print all the sections and their attributes in the list
void SectionList_Print(SectionList *list);

// Free any allocated memory for the section list
void SectionList_Destroy(SectionList *list);

#endif // SECTION_H