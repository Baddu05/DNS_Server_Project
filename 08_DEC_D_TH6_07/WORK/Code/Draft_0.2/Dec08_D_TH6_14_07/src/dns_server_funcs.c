
/**********************************************************************************************************************************************
*                       FILE NAME       :  dns_server_funcs.c
*
*                       DESCRIPTION     :  contains all the functions thatare called to serve client in dns 
*
*       DATE            NAME                    REFERENCE       REASON
*    								Project
*
*       copyright @2015,Aricent Technologies(Holdings) Ltd.,
**********************************************************************************************************************************************/


/***********************************************************************************************************************************************
*
*                                HEADER FILES
*
***********************************************************************************************************************************************/

#include "header_server.h"
#include <errno.h>
#include "logger.h"

/**********************************************************************************************************************************************
*
*       			GLOBAL VARIABLES                         
*
**********************************************************************************************************************************************/

int g_trace_level = DETAILED_TRACE;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

/*********************************************************************************************************************************************
*
* 	Function name: my_handler
*
* 	Description: Signal handler function to exit from server
*
* 	Return: no return value
*
**********************************************************************************************************************************************/

void my_handler( 
		int sig // return value of signal
		 )

{
	printf("\nExiting from Server .. \n");
	exit(1);
}

/*********************************************************************************************************************************************
*
* 	Function name: serve_client
*
* 	Description: Thread initiating function that call other funtions to serve client
*
* 	Return: no return value
*
**********************************************************************************************************************************************/


void serve_client(
		  void *arg //structure of Thread_client type
		)


{

	LOGGER(LOG_CRITICAL, "Entering funtion: serve_client for server...\n");
	
	int	search_ret	= 0,
		rec_count	= 0,
		len 		= 0;

	Thread_client *client = (Thread_client *)arg;
	
	char *buf = client->buffer;
	int sd = client->sd;
	socklen_t structlength;
	
	struct DNS_HEADER *head;
	struct QUESTION *qinfo;
	char *qname;
	
	

	structlength = sizeof(struct sockaddr_storage);
	
	/* Detaching  this thread */
	pthread_detach(pthread_self());
	
	/* decrementing global variable 'count' to decrease the number of clients to be served concurrently */

	pthread_mutex_lock(&mutex1);
	count = count - 1;
	pthread_mutex_unlock(&mutex1);
	



	/* set header and question fields in buffer */

	head = (struct DNS_HEADER *)buf;
	qname = (char *)&buf[sizeof(struct DNS_HEADER)];
	qinfo = (struct QUESTION *)&buf[sizeof(struct DNS_HEADER) + strlen((char *)qname)+1];
	
	/* Search record in database */

	search_ret = Search_Record("resource_records",buf,qinfo->qtype,&rec_count);
	if(SUCCESS == search_ret)
	{
		LOGGER(LOG_CRITICAL, "successful search\n");
		
		/* set response code in header to success */
		
		head->rcode = 0;
		
		/* assign the length of the message to be sent to client */
		
		len = sizeof(struct DNS_HEADER) + (strlen((char *)qname) + 1) + sizeof(struct QUESTION) + (rec_count * sizeof(struct RES_RECORD));
	}
	
	/* Search unsuccessful */
	else
	{	
		/* set response code in header to server failure */

		head->rcode = 2;
		
		/*assign the length of the message to be sent to client */

		len = sizeof(struct DNS_HEADER) + strlen((char *)qname) + 1;
	}
	
	/* set query or reponse code */

	head->qr = 1;
	
	/* assign number of answer records in header */

	head->ans_count = rec_count;
	
	/* send the response message in buffer to client */

	int ret = sendto(sd,client->buffer,len,0,(struct sockaddr *)&client->cliaddr,structlength);
	if (0 > ret)
	{
		ERROR(ERROR_MAJOR, "Error sending to client\n");
	
		LOGGER(LOG_CRITICAL, "Exiting funtion: server_client for server...\n");
		exit(FAILURE);
	}

	/*increment count variable to indicate one client is served */

	pthread_mutex_lock(&mutex1);
	count = count + 1;
	pthread_mutex_unlock(&mutex1);
	
	//free(client->th);
	free(client);
	
	LOGGER(LOG_CRITICAL, "Exiting function: server_client for server...\n");	
	
	//pthread_exit(NULL);
}

/***********************************************************************************************************************************************
*
* 	Function name: Search_Record
*
* 	Description: searches the file containing all the resource records for query 
*	       or inverse query by client
*
* 	Return: SUCCESS OR FAILURE
*
***********************************************************************************************************************************************/

int Search_Record(
		  char *filename,	/*name of resource_record file*/
		  char *buf,		/*buffer*/
		  unsigned short type,	/*q_type*/
		  int *count		/* record count */
		)


{
	LOGGER(LOG_CRITICAL, "Entered function: Search_Record for server....\n");
	
	FILE *fp;
	
	int flag	= FAILURE;

	int	read_ret,
		offset;
	
	struct RES_RECORD file_res;
	struct DNS_HEADER *head;
	struct RES_RECORD *rec = NULL;
	struct QUESTION *qinfo;
	char *qname;
	
	char *host;
	char *free_temp = NULL;
	char temp[IP_SIZE];

	/* typecasting buffer to retreive DNS_HEADER */

	head = (struct DNS_HEADER *)buf;
	
	/* typecasting buffer to retrieve question name */

	qname = (char *)&buf[sizeof(struct DNS_HEADER)];

	
	/* typecasting buffer to retrieve question type and class fields */

	qinfo = (struct QUESTION *)&buf[sizeof(struct DNS_HEADER) + strlen((char *)qname)+1];
	

	if(0 == head->opcode)
	{
		host = dnstohost(qname);// if query, convert dns format to host format to search in records file (ex:3wwww3msn3com0 to www.msn.com)
		free_temp = host;
	}
	else if(1 == head->opcode) 
	{
		host = qname;
	}
	
	fp = fopen(filename,"r");
	if(NULL == fp)
	{
		ERROR(ERROR_MAJOR, "CANNOT OPEN RECORDS FILE...\n");
		LOGGER(LOG_CRITICAL, "Exiting function: Search_Record for server...\n");
		
		if(NULL != free_temp)
		{
			free(free_temp);
		}
		return FAILURE;
	}

	LOGGER(LOG_MINOR,"Reading from file...\n");

	/* allocating memory to Resource structure pointer to retrieve records from file */
	
	/*rec = malloc(sizeof(struct RES_RECORD));
	if(NULL == rec)
	{
		ERROR(ERROR_MAJOR, "Error allocating memory..\n");
		LOGGER(LOG_CRITICAL, "Exiting function: Search_Record for server...\n");
		
		if(NULL != free_temp)
		{
			free(free_temp);
		}
		return FAILURE;
	}*/

	
	/* Search records from file until End of file is reached */
	
	while(1)
	{
		memset(&file_res,0,sizeof(struct RES_RECORD));
		read_ret = fread(&file_res,sizeof(struct RES_RECORD),1,fp);
		if(0 > read_ret)
		{
			ERROR(ERROR_MAJOR, "Error reading from file\n");
			fclose(fp);
			LOGGER(LOG_CRITICAL, "Exiting function: Search_Records for server...\n");

			if(NULL != free_temp)
			{
				free(free_temp);
			}
			return FAILURE;
		}
		if(0 == read_ret)
		{
			LOGGER(LOG_MINOR,"Finished reading file...\n");	
			break;
		}
		
		/* check the type of query */
		switch(qinfo->qtype)
		{
			case T_A: /* case for DNS lookup or query  saved in records file as A Records */
				
				if(0 == strncasecmp((char *)host, file_res.name, strlen((char *)host) + 1))
				{
					/* if found set offset in buffer */
					offset = sizeof(struct DNS_HEADER) + strlen((char *)qname) + 1 + sizeof(struct QUESTION)+ (*count * sizeof(struct RES_RECORD));
					/* move to next record using offset */
					rec = (struct RES_RECORD *)&buf[offset];
					LOGGER(LOG_MINOR, "match found\n");

					/* assign the records fields to resource record structure fields to send as a resoponse to client */

					assign_record(rec,host,file_res.rdata,&file_res);
					
					/* increment the count of number of records count */
					*count = *count + 1;
					
					flag = SUCCESS;
				}
				break;

			case T_PTR:/* case for DNS Inverse query saved in records file as PTR records */
				
				if(0 == strncmp((char *)host,file_res.rdata,strlen((char *)host)))
				{
					offset = sizeof(struct DNS_HEADER) + strlen((char *)qname) + 1 + sizeof(struct QUESTION)+ (*count * sizeof(struct RES_RECORD));
					rec = (struct RES_RECORD *)&buf[offset];
					
					strcpy(temp,file_res.rdata);
					
					/* convert IN-ADDR.ARPA. domain format of IP to dotted decimal IP address format */

					//DomainToIP(temp);

					/* assign the records fields to resource record structure fields to send as a response to client */
					
					assign_record(rec,file_res.name,temp,&file_res);	

					/* there is only one domain name corresponding to IN-ADDR.ARPA. format of IP address */
					
					*count = 1;

					flag = SUCCESS;
				}
				break;
		}
	}

	fclose(fp);

	if(NULL != free_temp)
	{
		free(free_temp);
	}
	LOGGER(LOG_CRITICAL, "Exiting function: Search_Record for server...\n");
	return flag;
	
}

/***********************************************************************************************************************************************
*
* 	Function name: assign_record
*
* 	Description: this function sets the resource record fields from the fields of 
*		Resource record found in file
*
* 	Return: no return value
*
**********************************************************************************************************************************************/

void assign_record(
			struct RES_RECORD *rec,		//resource record structure type
			char *request,			//domain name
			char *response,			// ip address
			struct RES_RECORD *file_res	//pointer of RES_RECORD structure type
			)

{
	
	LOGGER(LOG_CRITICAL, "Entering function: assign_record for server...\n");
	
	strcpy(rec->name,request);
	rec->resource.type = file_res->resource.type;
	rec->resource._class = file_res->resource._class;
	rec->resource.ttl = file_res->resource.ttl;
	rec->resource.data_len = file_res->resource.data_len;
	strcpy(rec->rdata,response);		
	
	LOGGER(LOG_CRITICAL, "Exiting function: assign_record for server...\n");
}
	
/***********************************************************************************************************************************************
*
* 	Function name: dnstohost 
*
*	Description: converts dns format of domain_name to host
*	       (ex: 3www3msn3com0 to www.msn.com
*
* 	Return: return address of converted string or NULL on failure in allocation
*
***********************************************************************************************************************************************/

char *dnstohost(
		char *dns	//dns
		)

{
	
	int	num	= 0,
		num1	= 0,
		num2	= 0,
		j; /*index for for loop */
	
	char *temp;
	char *host;
	
	LOGGER(LOG_CRITICAL, "Entered function: dnstohost for server\n");	
	
	/* allcating memory for host */

	host = malloc((strlen((char *)dns) +1) * sizeof(char));
	if(NULL == host)
	{
		ERROR(ERROR_MAJOR, "Memory allcation error\n");
		LOGGER(LOG_CRITICAL, "Exiting function: dnstohost for server\n");	
		return NULL;
	}


	temp = host;
	/* read charaters untill null character is reached */

	while(*dns != '\0')
	{
		if(isdigit(*dns))
		{
			num1 = *dns++ - 48;
			if(isdigit(*dns))
			{
				num2 = *dns++ - 48;
				num = num1*10 + num2;
			}
			else
			{
				num = num1;
			}

			for(j=0; j<num; j++)
			{
				*temp++ = *dns++;
			}
			*temp++ = '.';
		}
		
	}
	
	temp = temp - 1;
	*temp = '\0';

	LOGGER(LOG_CRITICAL, "Exiting function: dnstohost for server\n");		
	return host;
}

/*********************************************************************************************************************************************
*
* 	Function name: DomainToIP
*
* 	Description: converts IN-ADDR.ARPA. domain format of IP to IP address format
*
* 	Return: no return value
*
**********************************************************************************************************************************************/

/*void DomainToIP(
		char *domain	//domain name
		)

{
	
	char temp[IP_SIZE];
	char *token[4];
	int index=0;
	
	LOGGER(LOG_CRITICAL, "Entering  function: DomainToIP for server\n");		
	
	strcpy(temp,domain);
	strcpy(domain,"");
	
	token[0] = strtok(temp,".");

	while(NULL != token[index])
	{
		index += 1;
		if(index == 4)
			break;
		token[index] = strtok(NULL,".");
	}

	for(index = 3;index >= 0; index--)
	{
		strncat(domain,token[index],strlen(token[index])+1);
		strcat(domain,".");
	}
	domain[strlen(domain)-1] = '\0';

	LOGGER(LOG_CRITICAL, "Exiting function: dnstohost for server\n");		
}
*/
/*******************************************************************************************************************************************
*
* 	Function name: Authenticate
*
* 	Description: recieves login information from client and authenticates 
*
* 	Return: returns SUCCESS or FAILURE
*
******************************************************************************************************************************************/

int Authenticate(
		int sd,			//socket descriptor
		struct sockaddr_storage *clientaddr	//socket structure type 
		)

{

	
	LOGGER(LOG_CRITICAL, "Entering function: Authenticate for server\n");	
	
	/* login structure variable to read from file and recieve from client */

	login login_info;
	login file_log;
	int	recv_ret 	= 0,
		send_ret 	= 0,
		 n		= 0;

	socklen_t len = sizeof(struct sockaddr_storage);
	
	FILE *fp;	

	
	recv_ret = recvfrom(sd,&login_info,sizeof(login),0,(struct sockaddr *)clientaddr,&len);
	if(0 > recv_ret)
	{
		ERROR(ERROR_MAJOR,"Error in recieveing message\n");
		LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
		return FAILURE;
	}

	/* open login file */

	fp = fopen("login","r");
	if(NULL == fp)
	{	
		ERROR(ERROR_MAJOR,"Error in opening file\n");
		LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
		return FAILURE;
	}


	while(1)
	{
		n = fread(&file_log,sizeof(login),1,fp);
		if(0 > n)
		{
			ERROR(ERROR_MAJOR,"Error in reading from file\n");
			LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
			fclose(fp);
			return FAILURE;
		}
			
		if(0 == n)// end of file
		{
			/* failure in finding login information sent from client */

			login_info.response = FAILURE;
			
			/* send response to client */

			send_ret = sendto(sd,&login_info,sizeof(login_info),0,(struct sockaddr *)clientaddr,sizeof(*clientaddr));
			if(0 > send_ret)
			{	
				ERROR(ERROR_MAJOR,"Error in sending to client\n");
				LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
				return FAILURE;
			}

			LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
			fclose(fp);
			return FAILURE;
		}
		if((0 == strcmp(login_info.user,file_log.user)) && (0 == strcmp(login_info.password, file_log.password)))
		{
			/* successfull login */

			login_info.response = SUCCESS;

			/* send response to  client */
			send_ret = sendto(sd,&login_info,sizeof(login_info),0,(struct sockaddr *)clientaddr,sizeof(*clientaddr));
			if(0 > send_ret)
			{	
				ERROR(ERROR_MAJOR,"Error in sending to client\n");
				LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
				fclose(fp);
				return FAILURE;
			}	
			
			LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
			fclose(fp);
			return SUCCESS;
		}
	}
}

/*******************************************************************************************************************************************
*
* 	Function name: Update_Record
*
*	Description: Update the record sent from client 
*
*	Return: no return value
*
*********************************************************************************************************************************************/

void Update_Record(
			void *arg //structure of Thread_Client structure type
		)

{

	
	LOGGER(LOG_CRITICAL, "Entering function: Update_Record for server...\n");

	Thread_client *client = (Thread_client *)arg;
	
	char *buf = client->buffer;

	int sd = client->sd;

	struct DNS_HEADER *head;

	int	status		= 0,	
		send_ret	= 0;
	
	
	head = (struct DNS_HEADER *)buf;
	
	status = update_res_record("resource_records",buf,head->z);

	head->qr = 1; // set qr field to response
	
	if(SUCCESS == status)
	{
		head->rcode = 0; // set reponse code success
	}
	else
	{	
		head->rcode = 1;// set response code to server failure
	}
	
	/* sending response to client */

	send_ret = sendto(sd,(char *)buf,sizeof(struct DNS_HEADER),0,(struct sockaddr *)&client->cliaddr,sizeof(client->cliaddr));
	if(0 > send_ret)
	{
		ERROR(ERROR_MAJOR, "Error in sending to client..\n");
		LOGGER(LOG_CRITICAL, "Entering function: Update_Record for server...\n");
		return;
	}

	LOGGER(LOG_CRITICAL, "Entering function: Update_Record for server...\n");
}

/**********************************************************************************************************************************************
*
* 	Function name: update_res_record
*
* 	Description: update the resource reoord in file
*
* 	Return: returns SUCCESS OR FAILURE
*
**********************************************************************************************************************************************/

int update_res_record(
			char *filename,
			char *buffer,
			unsigned char update_option
			)

{

	
	LOGGER(LOG_CRITICAL, "Entering function: update_res_record for server...\n");

	struct RES_RECORD *record;
	struct RES_RECORD file_res;
	
	int	n		= 0,
		flag		= FAILURE,
		write_ret	= 0;

	FILE *fp;
		
	/* typecast buffer to RES_RECORD field */

	record = (struct RES_RECORD *)&buffer[sizeof(struct DNS_HEADER)];
	
	memset(&file_res,0,sizeof(struct RES_RECORD));

	pthread_mutex_lock(&mutex2);

	fp = fopen(filename,"r+");
	if(NULL == fp)
	{	
		ERROR(ERROR_MAJOR,"Error in opening file\n");
		LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
		return FAILURE;
	}
	
	while(1)
	{
		
		n = fread(&file_res,sizeof(struct RES_RECORD),1,fp);
		if(0 > n)
		{
			ERROR(ERROR_MAJOR,"Error in reading from file\n");
			LOGGER(LOG_CRITICAL,"Exiting function: update_res_record for server...\n");
			pthread_mutex_unlock(&mutex2);
			return FAILURE;
		}

		if(n == 0)// end of file reached
		{
			if(ADD == update_option && FAILURE == flag) //no duplicates found, safe to add record
			{
				/* Set all the fields to the records file */
				memset(&file_res.name,0,sizeof(struct RES_RECORD));

				strncpy(file_res.name,(char *)record->name,strlen((char *)record->name)+1);
				file_res.resource.type = 1;
				file_res.resource._class = 1;
				file_res.resource.ttl = 1800;
				file_res.resource.data_len = strlen((char *)record->rdata);
				strncpy(file_res.rdata,(char *)record->rdata,strlen((char *)record->rdata)+1);
				
				write_ret = fwrite(&file_res,sizeof(struct RES_RECORD),1,fp);
				if(0 > write_ret)
				{
					ERROR(ERROR_MAJOR,"Error in writing to file\n");
					LOGGER(LOG_CRITICAL,"Exiting function: update_res_record for server...\n");
					pthread_mutex_unlock(&mutex2);
					return FAILURE;
				}
			}
			pthread_mutex_unlock(&mutex2);
			fclose(fp);
			return SUCCESS;
		}
		else if((0 == strcmp(record->rdata,file_res.rdata)) && (0 != strcasecmp(record->name,file_res.name)))
		{
			pthread_mutex_unlock(&mutex2);
			fclose(fp);
			LOGGER(LOG_CRITICAL,"Exiting function: update_res_record for server...\n");
			return FAILURE;
		}
		else if((0 == strcasecmp((char *)record->name,file_res.name))&& (0  == strcmp((char *)record->rdata,file_res.rdata)))
		{
			flag = SUCCESS;
			
			/* record to be updated only if the option is DELETE */
			if(DELETE == update_option)
			{
				/* Set all the fields to null to make it invalid */

				fseek(fp,-(sizeof(struct RES_RECORD)),SEEK_CUR);
				strncpy(file_res.name," ",strlen(file_res.name)+1);
				file_res.resource.type = 0;
				file_res.resource._class = 0;
				file_res.resource.ttl = 0;
				file_res.resource.data_len = 0;
				strncpy(file_res.rdata," ",strlen(file_res.rdata)+1);
				write_ret = fwrite(&file_res,sizeof(struct RES_RECORD),1,fp);
				if(0 > write_ret)
				{
					ERROR(ERROR_MAJOR,"Error in writing to file\n");
					LOGGER(LOG_CRITICAL,"Exiting function: update_res_record for server...\n");
					pthread_mutex_unlock(&mutex2);
					return FAILURE;
				}
				
				fclose(fp);	
				pthread_mutex_unlock(&mutex2);
				return SUCCESS;
			}
		}
	}
	LOGGER(LOG_CRITICAL, "Exiting function: update_res_record for server...\n");


	pthread_mutex_unlock(&mutex2);
}
		

		
