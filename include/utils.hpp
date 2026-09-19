#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>

/**
 * Split a C-style string using multiple delimeters
 * 
 * @warning Each element in the returned vector has a lifetime linked to
 * the original `str` passed into the function. The `str` is also modified by
 * this function and should be considered useless afterwards.
 */
std::vector<char *> split_string(char *str, const char *delimeters);

#endif  // UTILS_HPP