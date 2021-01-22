/*****************************************************************************
**      FILE NAME : logger.h

**	DESCRIPTION : it contains the included files, prototypes of functions 
		      used and macros used throughout

**	copyright  ARICENT Inc. All Rights Reserved

******************************************************************************/


#ifndef _LOGGER_H_
#define _LOGGER_H_

/************************* INCLUDED FILES ********************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/**************************** MACROS *********************/
#define LOGGER logger
#define LOG_SIZE 100
#define BUFF_SIZE 512
#define LOOP 5
#define FILE_SIZE 50

#define LOG_CRITICAL 0
#define LOG_MAJOR 1
#define LOG_MINOR 2
#define LOG_LEVEL 4
#define LOG_MAX 10000L
#define ZERO 0

/* error details */

#define ERROR_CRITICAL 1
#define ERROR_MAJOR    2
#define ERROR_MINOR    3

/* ang_error is defined in anagram_error.c */
#define ERROR ang_error


#define NO_TRACE       1
#define BRIEF_TRACE    2
#define DETAILED_TRACE 3

//#ifdef TRACE_FEATURE
 #define TRACE(level, x)  if (g_trace_level >= level) printf x
//#else#
//#define TRACE(level, x) /* no definition */
//#endif

extern int g_trace_level;

char log_file[FILE_SIZE];

void logger(int,char*);
void get_now_time(void);
void roll_over_log(void);
void roll_over_err(void);
void ang_error(int ,char *);

#endif


