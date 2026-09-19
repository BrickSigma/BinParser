#include "utils.hpp"

#include <cstring>

std::vector<char *> split_string(char *str, const char *delimeters)
{
    std::vector<char *> strings{};

    char *token = std::strtok(str, delimeters);
    while (token != nullptr)
    {
        strings.push_back(token);
        token = std::strtok(nullptr, delimeters);
    }

    return strings;
}