
/*******************************************************************
*
* File NAME : header.h
*
* Description: header file
*
*******************************************************************/
#ifndef _DNS_H
#define _DNS_H

/*************************HEADER FILES****************************/
#include<stdio.h> 
#include<string.h>    
#include<stdlib.h>    
#include<sys/socket.h>    
#include<arpa/inet.h> 
#include<netinet/in.h> 
#include<unistd.h>   
#include<sys/types.h>
#include<ctype.h>
/*************************MACROS********************************/

#define T_A 1 //Ipv4 address 
#define T_NS 2 //Nameserver 
#define T_CNAME 5 // canonical name 
#define T_SOA 6 /* start of authority zone */ 
#define T_PTR 12 /* domain name pointer */ 
#define T_MX 15 //Mail server //Function Prototypes 

#define SUCCESS 1
#define FAILURE 0
#define IP 1
#define DOMAIN 2
#define MAX 1024
#define SERVER "127.0.0.1"
#define PORT 1234
#define HOST_SIZE 256
#define IP_SIZE 40
#define ADD 0
#define DELETE 1

#define Display_Help() {\
			 printf("***************************************************\n");\
			 printf("      DNS RESOLVER : USER MANUAL\n\n");\
			 printf("***************************************************\n\n");\
			 printf("USAGE:\n\n");\
			 printf("\tdns [OPTION] [DOMAIN NAME/IP ADDRESS]...\n\n");\
			 printf("Description:\n\n");\
			 printf("\n\t-DI --domain_name\n");\
			 printf("\n\tfor resolving IP address from 'domain_name'\n\n");\
			 printf("\n\t-ID --ip_address\n");\
			 printf("\n\tfor resolving domain_name from 'ip_address'\n\n");\
			 printf("\n\tBelow options only for Administrators\n");\
			 printf("\n\t-U --domain_name --ip_address\n");\
			 printf("\n\tadd a resource record to name server\n\n");\
			 printf("\n\t-D --domain_name --ip_address\n");\
			 printf("\n\tdelete a resource record from name server\n\n");\
			}


/**************************** Function declarations *********************/

extern char *remove_prefix(char *input);
extern void Dns_Query(int sd,struct sockaddr_in *nameserver,char *domain_name,int option);
extern char *search_local_cache(char *file,char *query,int option);
extern char *HostToDns(char *domain_name);
extern int create_message(char *buffer,char *domain_name,int qtype);
extern void save_and_display(char *buffer,int length,char *file);
extern char *IPToDomain( char *IP_address);
extern int validate_ip(char *ip);
extern int Login_server(int sd,struct sockaddr_in *nameserver);
extern void Update(int sd, struct sockaddr_in *nameserver, int update_option, char *domain_name, char *ip);
extern void update_local_cache(char *filename,int update_option,char *domain_name, char *ip);
extern int create_update_message(char *buffer,int update_option,char *domain_name,char *ip);




#endif  


