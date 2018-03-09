
#define DEBUG 1
#define MAX_BUFFER_LENGTH 2000
#ifdef _WIN32
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif