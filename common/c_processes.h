#pragma once

#include <string>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <cstring>

/**
 * Wrapper for fork()
 */
pid_t create_process();

/**
 * Feed it a path to a file name, will return whether the file
 * exists or not on the current machine
 */
bool file_exists(const std::string& name);

/**
 * Concatenates two C strings
 */
char* concat(const char *s1, const char *s2);