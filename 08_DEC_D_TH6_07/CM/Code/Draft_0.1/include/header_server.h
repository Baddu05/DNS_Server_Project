
/*****************************************************************************
**      FILE NAME :header_server.h 

**	DESCRIPTION : it contains the included files, prototypes of functions 
		      used and macros used in server

**	copyright  ARICENT Inc. All Rights Reserved

******************************************************************************/

#ifndef _HEADER_SERVER_H_
#define _HEADER_SERVER_H_

/***************************INCLUDED FILES ******************************/

#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include "header.h"
#include "message.h"
#include "resource.h"
#include "authenticate.h"

/*******************************MACROS************************************/
#define PORT1 12345

/**************************FUNCTION DECLARATIONS*************************/

extern int count;
extern int g_trace_level;
extern char * dnstohost(char *);
extern void DomainToIP(char *domain);
extern int Authenticate(int sd,struct sockaddr_storage *clientaddr);
extern void Update_Record(void *arg);
extern int update_res_record(char *filename,char *buffer,unsigned char update_option);
extern void serve_client(void *arg);
extern int Search_Record(char *filename,char *buf,unsigned short type,int *count);
extern void assign_record(struct RES_RECORD *rec, char *request,char *response,struct RES_RECORD *file_res);
extern void my_handler( int sig );

/******************************STRUCTURE DEFINITION *********************/
typedef struct 
{
	int sd;
	struct sockaddr_storage cliaddr;
	char buffer[MAX];
	pthread_t *th;
}Thread_client;

#endif


