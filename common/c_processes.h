#pragma once

#include <string>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

pid_t create_process();
bool file_exists(const std::string& name);