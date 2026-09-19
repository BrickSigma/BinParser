#include "utils.hpp"

#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
#define strtok_r strtok_s
#endif

char *copy_string(char* dest, size_t destsz, const char* src, size_t count)
{
	(void)destsz;  // Prevent unused variable compiler warning
#if defined(_WIN32) || defined(_WIN64)
	errno_t error = strncpy_s(dest, destsz, src, count);
	if (error != 0)
		return nullptr;
	return dest;
#else
	return strncpy(dest, src, count);
#endif
}

std::vector<char*> split_string(char* str, const char* delimeters)
{
	std::vector<char*> strings{};
	char* saveptr{ nullptr };

	char* token = strtok_r(str, delimeters, &saveptr);
	while (token != nullptr)
	{
		strings.push_back(token);
		token = strtok_r(nullptr, delimeters, &saveptr);
	}

	return strings;
}