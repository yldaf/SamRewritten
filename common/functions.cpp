#include "functions.h"
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <cstring>
#include <algorithm>
#include <cctype>

pid_t create_process()
{
    pid_t pid;

    do {
	pid = fork();
    } while ((pid == -1) && (errno == EAGAIN));

    return pid;
}

void read_count(int fd, void *buf, size_t count)
{
  size_t bytes_read;
  for (size_t i = 0; i < count; i += bytes_read) {
    bytes_read = read(fd, (void*)((char*)buf+i), count-i);
  }
}

bool file_exists(const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

char* concat(const char *s1, const char *s2)
{
    char *result = (char*)malloc(strlen(s1)+strlen(s2)+1); //+1 for the null-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


bool strstri(const std::string & strHaystack, const std::string & strNeedle)
{
  auto it = std::search(
    strHaystack.begin(), strHaystack.end(),
    strNeedle.begin(),   strNeedle.end(),
    [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
  );
  return (it != strHaystack.end() );
}
