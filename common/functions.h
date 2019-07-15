#pragma once

#include <string>

/**
 * Wrapper for fork()
 */
pid_t create_process();

/**
 * Wrapper for read() to actually read count bytes
 * instead of reading up to count bytes
 */
void read_count(int fd, void *buf, size_t count);

/**
 * Feed it a path to a file name, will return whether the file
 * exists or not on the current machine
 */
bool file_exists(const std::string& name);

/**
 * Concatenates two C strings
 */
char* concat(const char *s1, const char *s2);

/**
 * Insensitive "string in string"
 */
bool strstri(const std::string & strHaystack, const std::string & strNeedle);
