
/*******************************************************************
*
* File NAME :  message.h
*
* Description: data structures
*
*******************************************************************/

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

struct DNS_HEADER 
{ 
    unsigned short id; // identification number 
    unsigned char rd :1; // recursion desired     
    unsigned char tc :1; // truncated message     
    unsigned char aa :1; // authoritive answer     
    unsigned char opcode :4; // purpose of message     
    unsigned char qr :1; // query/response flag 
    unsigned char rcode :4; // response code     
    unsigned char cd :1; // checking disabled     
    unsigned char ad :1; // authenticated data     
    unsigned char z :1; // its z! reserved     
    unsigned char ra :1; // recursion available 
    unsigned short q_count; // number of question entries     
    unsigned short ans_count; // number of answer entries     
    unsigned short auth_count; // number of authority entries     
    unsigned short add_count; // number of resource entries 
}; 



//Constant sized fields of query structure 
struct QUESTION 
{ 
    unsigned short qtype; 
    unsigned short qclass; 
}; 
 //Constant sized fields of the resource record structure 
#pragma pack(push, 1) 
struct R_DATA 
{ 
    unsigned short type; 
    unsigned short _class; 
    unsigned int ttl; 
    unsigned short data_len; 
}; 
#pragma pack(pop) 
struct RES_RECORD 
{ 
    char name[HOST_SIZE]; 
    struct R_DATA resource; 
    char rdata[IP_SIZE]; 
}; 
 //Structure of a Query 
typedef struct
{ 
    char *name; 
    struct QUESTION *ques; 
}QUERY;

#endif
