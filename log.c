// Log function to save log messages to a file "log.txt" in the same directory as the executable
#include "log.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define LOG_FILE "log.txt" // Default log file name, used as a fallback.

int log_message(const char *format, ...)	 
{
	// Check if the environment variable MSM_OUTPUT is set
	if (getenv("MSM_OUTPUT") == NULL)
		return 0;

	va_list args, args_copy;
	va_start(args, format);
	
    // Determine the required buffer size for the formatted string
	va_copy(args_copy, args);
    size_t size = vsnprintf(NULL, 0, format, args_copy) + 1; // +1 for the null terminator
	va_end(args_copy);
		
    // Allocate buffer on the stack
    char *buffer = (char *)alloca(size);

    // Write formatted data to the buffer
    vsnprintf(buffer, size, format, args);
	va_end(args);

    // Open log file with appropriate flags and permissions
	int fd = open(getenv("MSM_OUTPUT"), O_CREAT | O_APPEND | O_WRONLY, 0600); // 600 - rw for owner
	if (fd == -1) {
		return -1;
	}
	
	// Write log message to log file
	int ret = write(fd, buffer, strlen(buffer));
	close(fd);

	return ret == -1 ? -1 : 0;	
}

int log_event(log_type type, log_action action, void *address, size_t size)
{
    // Convert enum values to strings for logging
	const char *type_str = type == MALLOC ? "MALLOC" : "FREE";
	const char *action_str = action == START ? "START" : "END"; 	
	struct timeval tv;
	struct tm *tm;
	
	// Get current time
	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	if (tm == NULL) {
		return -1;
	}

	// Log different messages based on the type and action
	if (type == MALLOC && action == START)
		return log_message("%02d:%02d:%02d.%06ld %s %s %d\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec, type_str, action_str, size);
	else
		return log_message("%02d:%02d:%02d.%06ld %s %s %p %d\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec, type_str, action_str, address, size);
}

int log_new_execution()
{
    // Get the current time
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
    if (tm == NULL) {
        return -1;
    }

	// Log the new execution start time
	return log_message("NEW EXECUTION : %04d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}