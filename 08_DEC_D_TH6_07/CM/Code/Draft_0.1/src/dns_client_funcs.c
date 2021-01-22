/**********************************************************************************************************************************************
*
*	FILE NAME 		: dns_client_funcs.c
*	
*	FILE DESCRIPTION	: contains all the functions needed by the client.
*
*
***********************************************************************************************************************************************/

/**********************************************************************************************************************************************
*	INCLUDING HEADER FILES
*
**********************************************************************************************************************************************/

#include "header.h"
#include "authenticate.h"
#include "message.h"
#include "resource.h"
#include "logger.h"

/***********************************************************************************************************************************************
*
* 	Function name: Update
*
* 	Description:  function to add or delete record in resource record file
*
* 	Return: nothing
*
***********************************************************************************************************************************************/

void Update(int sd, struct sockaddr_in *nameserver, int update_option, char *domain_name, char *ip)
{
	char buffer[MAX];
	int msg_length = 0;
	socklen_t addrlen;
	struct DNS_HEADER *head = NULL;
	int send_ret = 0;
	int recv_ret = 0;
	LOGGER(LOG_CRITICAL,"Entering function:UPDATE\n");


	msg_length = create_update_message(buffer,update_option,domain_name,ip);
	
	TRACE(DETAILED_TRACE,("sending message length %d\n",msg_length));

	/*sending update information to name server*/
	send_ret = sendto(sd,(char *)buffer,msg_length,0,(struct sockaddr *)nameserver,sizeof(*nameserver));
	
	if(0 > send_ret)
	{	
		ERROR(ERROR_MAJOR,"Error in sending to client\n");
		LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
		return;
	}
	/*receiving update status from name server*/
	recv_ret = recvfrom(sd,(char *)buffer,sizeof(buffer),0,(struct sockaddr *)nameserver,&addrlen);

	if(0 > recv_ret)
	{
		ERROR(ERROR_MAJOR,"Error in recieveing message\n");
		LOGGER(LOG_CRITICAL,"Exiting function: Authenticate for server...\n");
		return;
	}
	/* reading header sent from server */

	head = (struct DNS_HEADER *)&buffer;

	if(0 == head->rcode)/*on successfull updation*/
	{
		LOGGER(LOG_CRITICAL,"UPDATE SUCCESSFULL\n" );
		printf("UPDATE SUCCESSFULL\n");
		update_local_cache("hosts",update_option,domain_name,ip);
		
	}
	else/*on failure to update*/
	{
		printf("UPDATE UNSUCCESSFULL\n");
		LOGGER(LOG_CRITICAL,"UPDATE UNSUCCESSFULL\n");
	}
	
	LOGGER(LOG_CRITICAL,"Exiting function:UPDATE\n");
}//end update

/***********************************************************************************************************************************************
*
* 	Function name: create_update_message
*
* 	Description:  function creates a DNS message to update the header fields according to DNS Header format
*
* 	Return: message length
*
***********************************************************************************************************************************************/

int create_update_message(char *buffer,int update_option,char *domain_name,char *ip)
{
	struct DNS_HEADER *header = NULL;
	struct RES_RECORD *update_record = NULL;
	int msg_length = 0;


	LOGGER(LOG_CRITICAL,"Entering function:CREATE_UPDATE_MESSAGE\n");


	/*initialising header fields according to DNS Header format*/
	header = (struct DNS_HEADER *)buffer;
	header->id = (unsigned short)htons(getpid());
	header->qr = 0;// query
	header->opcode = 5;//update
	header->z = (unsigned char)update_option;// add record
	header->aa = 0;
	header->tc = 1;
	header->rd = 1;
	header->ra = 0;
	header->ad = 0;
	header->cd = 1;
	header->rcode = 0;//response code to success
	header->q_count = htons(1);
	header->ans_count = 0;// answer count
	header->auth_count = 0;
	header->add_count = 0;
	
	/*initialising Update record*/
	update_record = (struct RES_RECORD *)&buffer[sizeof(struct DNS_HEADER)];
	strncpy((char *)update_record->name,domain_name,strlen(domain_name) + 1);
	strncpy((char *)update_record->rdata,ip,strlen(ip)+1);
	
	TRACE(BRIEF_TRACE,("%s %s updating\n",update_record->name,update_record->rdata));


	update_record->resource.type = 1;
	update_record->resource.data_len = strlen(ip);
	update_record->resource._class = 1;
	update_record->resource.ttl = 0;

	msg_length = sizeof(struct DNS_HEADER) + sizeof(struct RES_RECORD);
	
	TRACE(DETAILED_TRACE, ("update message length %d\n",msg_length));

	LOGGER(LOG_CRITICAL,"Exiting function:CREATE_UPDATE_MESSAGE\n");
	return msg_length;
}

/***********************************************************************************************************************************************
*
* Function name: update_local_cache
*
* Description:  function updates the local cache on successful updation on of resource record file
*
* Return: nothing
*
***********************************************************************************************************************************************/

void update_local_cache(char *filename,int update_option,char *domain_name, char *ip)
{
	FILE *fp;
	char file_buf[MAX];
	char file_host[HOST_SIZE];
	char file_ip[IP_SIZE];
	char *ret = NULL;
	int flag = 0;

	LOGGER(LOG_CRITICAL,"exiting function:UPDATE_LOCAL_CACHE\n");

	/*opening cache file*/
	fp = fopen(filename,"r+");
	if(fp == NULL)
	{
		fp = fopen(filename,"w+");
		if(fp == NULL)
		{
			ERROR(ERROR_MAJOR, "error opening file\n");
			LOGGER(LOG_CRITICAL, "exiting function:update_local_cache\n");
			return;
		}
	}
	while(1)
	{
		memset(file_buf,0,MAX);
		memset(file_host,0,HOST_SIZE);
		memset(file_ip,0,IP_SIZE);
		ret = fgets(file_buf,MAX-1,fp);
		if(NULL == ret)
		{
			if(ADD == update_option && 0 == flag)/*adding record to local cache provided its not already present in the */
			{
				fprintf(fp,"%s\t%s\t\tA\tIN\n",ip,domain_name);	
			}

			fclose(fp);/*closing cache file*/

			LOGGER(LOG_CRITICAL,"Exiting function:UPDATE_LOCAL_CACHE\n");
			return;
		}
		sscanf(file_buf,"%s\t%s",file_ip,file_host);

		/*searching local cache for domain name/IP*/
		if((0 == strncmp(domain_name,file_host,strlen(domain_name)+1)) && (0 == strncmp(ip,file_ip,strlen(ip)+1)))
		{
			flag = 1;/*if record found*/
			if(DELETE == update_option)
			{
				fseek(fp,-(strlen(file_buf)),SEEK_CUR);
				fprintf(fp,"#");/*beginning of the record marked with # to signify that record is deleted*/
				fclose(fp);/*closing cache file*/

				LOGGER(LOG_CRITICAL,"Exiting function:UPDATE_LOCAL_CACHE\n");
				return;
			}
		}
	}
	LOGGER(LOG_CRITICAL,"Exiting function:UPDATE_LOCAL_CACHE\n");
	return;
}


/**********************************************************************************************************************************************
*
* 	Function name: remove_prefix
*
* 	Description:  function to preprocess the input 
*		ex:http://wwww.google.com will be preprocessed as www.google.com
* 	Return: pointer to the preprocessed string
*
***********************************************************************************************************************************************/

char *remove_prefix(
		    char *input
		   )
{
	char *domain = NULL;
	char *token = NULL;

	LOGGER(LOG_CRITICAL,"Entering function:REMOVE_PREFIX\n");


	token = strtok(input,"/");/*tokenizing the input domain name for preprocessing*/
	token = strtok(NULL,"/");
	if(NULL != token)
	{
		domain = malloc((strlen(token) + 2) * sizeof(char));
		strcpy((char *)domain,token);	
		strcat(domain,"\0");
	}/*end if*/
	else
	{
		domain = malloc((strlen(input) + 2) * sizeof(char));/*allocating memory to store the preprocessed string*/
		strcpy((char *)domain,input);	
	}/*end if*/

	TRACE(DETAILED_TRACE,("Prefix removed %s\n",domain));
	LOGGER(LOG_CRITICAL,"Exiting function:REMOVE_PREFIX\n");
	return domain;
}	


/**********************************************************************************************************************************************
*
* 	Function name: Dns_Query
*
* 	Description:  Searches for domain name or IP in local cache,if present prints corresponding IP/domain name, else sends 
*		the query to nameserver and displays the response on the console

*	Return: nothing
*
**********************************************************************************************************************************************/

void Dns_Query(int sd,/*socket descriptor*/
	       struct sockaddr_in *nameserver,
	       char *query,
	       int option
	      )
{
	
	char *response = NULL;
	int send_ret = 0;
	int recv_ret = 0;
	int msg_length = 0;
	socklen_t len = 0;
	char buffer[MAX] = {'\0'};

	LOGGER(LOG_CRITICAL,"Entering function:DNS_QUERY\n");
	response = search_local_cache("hosts",query,option);/*calling function to search in local cache*/
	if(NULL != response)/*response will not be NULL when resource record is found in local cache*/
	{
		printf("Found in local cache\n");
		switch(option)
		{
			case IP:/*for query*/
				{	
					printf("DOMAIN NAME: %s\n",query);
					printf("IP Address: %s\n",response);
					free(response);
				}
				break;
			case DOMAIN:/*for inverse query*/
				{
					printf("IP Address: %s\n",query);
					printf("DOMAIN NAME: %s\n",response);
					free(response);
				}		
				break;
		}
		 LOGGER(LOG_CRITICAL,"Exiting function:DNS_QUERY\n");	
		return;
	}
	
	switch(option)
	{
		case IP:/*for query*/
			msg_length = create_message(buffer,query, T_A);
			break;
		case DOMAIN:/*for inverse query*/
		
				msg_length = create_message(buffer,query,T_PTR);
		
			break;
	}
	/*sending query to name server*/
	send_ret = sendto(sd, (char *)buffer, msg_length,0, (struct sockaddr *)nameserver, sizeof(struct sockaddr));
	if((msg_length != send_ret) || (0 > send_ret))
	{
		ERROR(ERROR_MAJOR,"failed to send message t server\n");
		LOGGER(LOG_CRITICAL,"Exiting function:DNS_QUERY\n");
		return;
	}
	memset(buffer, 0, MAX);
	
	len = sizeof(*nameserver);
	/*receiving response from name server*/
	recv_ret = recvfrom(sd,(char *)buffer, MAX, 0, (struct sockaddr *)nameserver, &len);
	if(0 > recv_ret)/*on failure to receive from server*/
	{
		
		ERROR(ERROR_MAJOR,"failed to receive from server\n");
		 
	}
	else/*on successfully receiving from server*/
	{
		printf("received from server\n");
	}
	 save_and_display(buffer,msg_length,"hosts");
	 free(response);
 	LOGGER(LOG_CRITICAL,"Exiting function:DNS_QUERY\n");
}


/**********************************************************************************************************************************************
*
* 	Function name: search_local_cache
*
* 	Description:  searches for the domain name/IP in the local cache
*
* 	Return: pointer to string containing corresponding IP/domain name if present else returns NULL
*
***********************************************************************************************************************************************/

char *search_local_cache(char *file,
			char *query,
			int option)
{
	FILE *fp = NULL;
	char file_buf[MAX] = {'\0'};	
	char file_host[HOST_SIZE]={'\0'};
	char file_ip[IP_SIZE]={'\0'};
	char *response = NULL;
	char *ret = NULL;

	 LOGGER(LOG_CRITICAL,"entering SEARCH_LOCAL_CACHE function\n");
	fp = fopen(file,"r");
	if(NULL == fp)
	{
		ERROR(ERROR_MINOR,"memory allocation failed\n");
		LOGGER(LOG_CRITICAL,"exiting SEARCH_LOCAL_CACHE function\n");
		return NULL;
	}


	while(1)
	{
		
		memset((char *)file_buf,0,MAX);
		ret = fgets((char *)file_buf,MAX-1,fp);

		if (NULL == ret)
		{
			fclose(fp);
			LOGGER(LOG_CRITICAL,"exiting SEARCH_LOCAL_CACHE function\n");
			return NULL;
		}

		file_buf[strlen((char *)file_buf)-1] = '\0';
		TRACE(DETAILED_TRACE,("Record retrieved: %s\n",file_buf));
			
		if ('#'	!= file_buf[0])/*if a record is deleted starting of the record is marked with #*/
		{
			
			memset(file_ip,0,IP_SIZE);
			memset(file_host,0,HOST_SIZE);

			sscanf((char *)file_buf,"%s\t%s",(char *)file_ip,(char *)file_host);
			
			switch(option)
			{
			
				case IP:if (0 == strncmp((char *)file_host,(char *)query,strlen((char *)query)+1))
					{	
						response = malloc( (strlen((char *)file_ip)+1) * sizeof(char));
						strncpy((char *)response,(char *)file_ip,strlen((char *)file_ip)+1);
						fclose(fp);
						LOGGER(LOG_CRITICAL,"exiting SEARCH_LOCAL_CACHE function\n");
						return response;
					}
					break;
				case DOMAIN:if (0 == strncmp((char *)file_ip,(char *)query,strlen((char *)query)+1))
					{	
						response = malloc( (strlen((char *)file_host)+1) * sizeof(char));
						strncpy((char *)response,(char *)file_host,strlen((char *)file_host)+1);
						fclose(fp);
						LOGGER(LOG_CRITICAL,"exiting SEARCH_LOCAL_CACHE function\n");
						return response;
					}
					break;
			}
		}
	}
}


/**********************************************************************************************************************************************
*
* Function name: HostToDns
*
* Description:  converts host format of domain name to dns format
*		ex:www.google.com -> 3www6google3com0
* Return: pointer to formatted string
*
***********************************************************************************************************************************************/
	
char *HostToDns(char *domain_name)
{	
	char *token;
	char *dns = NULL;
	int len = 0;
	char ch[3];

	LOGGER(LOG_CRITICAL,"entering function: HOST TO DNS\n");
	len = HOST_SIZE ; //starting and ending two length characters(ex: www.google.com -> 3www6google3com0)
	
	dns = malloc((len) * sizeof(char));
	strcpy((char *)dns,"");

	if(NULL == dns)/*error checking for malloc*/
	{
		
		ERROR(ERROR_CRITICAL,"memory allocation failed\n");
		LOGGER(LOG_CRITICAL,"exiting HOST TO DNS function\n");
	}
	
	strcat((char *)dns,"");
	token = strtok((char *)domain_name,".");

	while(NULL != token)
	{
		sprintf(ch,"%d",(int)strlen(token));
		strcat((char *)dns,ch);
		strncat((char *)dns,token,strlen(token));
		token = strtok(NULL,".");
	}
	strcat((char *)dns,"0");
	LOGGER(LOG_CRITICAL,"exiting HOST TO DNS function\n");
	return dns;
}

/**********************************************************************************************************************************************
*
* Function name: IPToDomain
*
* Description:  converts input IP to .IN-ADDR.ARPA. format
*
* Return: pointer to formatted string
*
***********************************************************************************************************************************************

char *IPToDomain(char *IP_address)
{
	char *token[4];
	char *temp;
	char *domain_name = NULL;
	int i=0;
	
	LOGGER(LOG_CRITICAL,"Entering function:IP TO DOMAIN\n");

	//dynamically allocating memory to store the formatted string
	temp = malloc((strlen((char *)IP_address) + 1) * sizeof(char));
	if( NULL == temp)//if memory allocation fails
	{
		ERROR(ERROR_MAJOR,"memory allocation failed\n");
		LOGGER(LOG_CRITICAL,"Exiting function:IP TO DOMAIN\n");
		return domain_name;
	}

	strncpy(temp,(char *)IP_address,strlen((char *)IP_address)+1);
	domain_name = malloc((strlen((char *)IP_address) + 15) * sizeof(char)); //IP + .IN-ADDR.ARPA.
	strcpy((char *)domain_name,"");

	token[0] = strtok(temp, ".");//tokenizing the input IP address
	while(NULL != token[i])
	{
		i += 1;
		token[i] = strtok(NULL, ".");
	}//end while

	for(i = 3; i>=0; i--)
	{
		strncat((char *)domain_name,token[i],strlen(token[i]) + 1);
		strcat((char *)domain_name,".");
	}//end for
	strcat((char *)domain_name,"IN-ADDR.ARPA.");
	LOGGER(LOG_CRITICAL,"Exiting function:IP TO DOMAIN\n");
	return domain_name;
}	
*/

/**********************************************************************************************************************************************
	
	FUNCTION DESCRIPTION
	NAME:		create_message
	WORK:		to create the DNS query message for DNS query and DNS inverse query.
	ARGUMENTS:	buffer, query, qtype
	RETURN VALUE:	length of the message(to be sent)

*********************************************************************************************************************************************/

int create_message(
		      char *buffer,  		/*to contain message*/
		       char *query,		/*containing IP address/domain_name*/
		      int   qtype			/*to identify query type(IP address/domain_name)*/
        	    )
{
	/*variable declarations*/
	struct DNS_HEADER *header=NULL;
	struct QUESTION *qinfo=NULL;
	char *temp=NULL;
	char *free_temp = NULL;
	char *qname=NULL;
	int msg_len = 0;
	
	/*FILE LOGGING*/
	LOGGER(LOG_CRITICAL, "Start of function: create_message\n");

	/*set header pointer to the starting of the message*/
	header = (struct DNS_HEADER *)buffer;	

	/*set various header fields*/
	header->id = (unsigned short) htons(getpid());		/*Identifier*/
	header->qr = 0; 					/*query*/

	/*if query is for IP address*/
	if(T_A == qtype)
	{
		header->opcode = 0;			/*standard query*/
	}

	/*if query is for domain name (inverse query)*/
	else if(T_PTR == qtype)
	{
		header->opcode =  1;		/*inverse query*/

	}
	
	/*set various message header fields*/
	header->aa = 0; 				/*Not authoritative*/
	header->tc = 0;					/*no truncation*/
	header->rd = 1;					/*recursion desired*/
	header->ra = 0;					/*recursion not available*/
	header->z = 0;					/*for future use*/
	header->ad = 0;
	header->cd = 0;
	header->rcode = 0;				/*to be used by server*/
	header->q_count = htons(1);			/*number of questions asked by client*/
	header->ans_count = 0;				/*number of answers (set by server)*/
	header->auth_count = 0;
	header->add_count = 0;
	
	/*set pointer after message header*/
	qname = (char *)&buffer[sizeof(struct DNS_HEADER)];

	/*query is for IP address*/
	if(T_A == qtype)
	{
		/*set domain name in the DNS format*/
		temp = HostToDns(query);	
		free_temp = temp;
	}

	/*query is for domain_name*/
	else if(T_PTR == qtype)
	{
		/*set IP address according to DNS format*/
		//temp = IPToDomain(query);
		temp = query;
		
	}

	/*set formatted query in the proper place in the DNS message*/
	strcpy(qname,temp);
	if(NULL != free_temp)
	{
		free(free_temp);
	}

	/*move at appropriate position in message to set another fields*/
	qinfo = (struct QUESTION *)&buffer[sizeof(struct DNS_HEADER) + (strlen((const char *)qname + 1))];
	
	qinfo->qtype = htons(qtype);			/*set query type in message*/
	qinfo->qclass = htons(1);			/*set query class in message*/

	/*finding total message length*/
	msg_len = sizeof(struct DNS_HEADER) + strlen((char *)qname) + 1 + sizeof(struct QUESTION);


	/*FILE LOGGING*/
	LOGGER(LOG_CRITICAL, "End of function: create_message\n");

	return msg_len;
}							/*end of function*/

/*********************************************************************************************************************************************
	FUNCTION DESCRIPTION
	NAME:		validate_ip
	WORK:		to check that IP address entered by user is valid or not.
	ARGUMENTS:	ip
	RETURN VALUE:	SUCCESS or FAILURE
*********************************************************************************************************************************************/

int validate_ip(
		char *ip			/*IP address*/
		)
{
	/*variable declarations*/
	char *token = NULL;
	char *temp = NULL ;
	int num = 0;
	int count = 0;

	/*FILE LOGGING*/
	LOGGER(LOG_CRITICAL, "Start of function: validate_ip\n");

	/*in dotted decimal notation maximum 15 char can be entered, so check for that*/
	if(strlen(ip) > 15 || isalpha(*ip))
	{

		/*LOGGING*/
		LOGGER(LOG_CRITICAL, "invalid IP address\nexiting validate_ip function\n");
		ERROR(ERROR_MINOR,"invalid ip\n");
		return FAILURE;
	}

	/*check for special IP addersses*/

	/*loopback address*/
	if(0 == strcmp(ip,"127.0.0.1"))						
	{
		printf("Local host\n");

		/*LOGGING*/
		LOGGER(LOG_CRITICAL, "invalid IP address\nexiting validate_ip function\n");

		return FAILURE;
	}

	/*special unicast and broadcast*/
	if((0 == strcmp(ip,"0.0.0.0")) || (0 == strcmp(ip,"255.255.255.255")))	
	{
		printf("Special address\n");

		/*LOGGING*/
		LOGGER(LOG_CRITICAL, "invalid IP address\nexiting validate_ip function\n");
		return FAILURE;
	}

	temp = malloc((strlen(ip) + 1) * sizeof(char));

	strncpy(temp, ip, strlen(ip) + 1);

	/*check to ensure that each field in IP address is in range 0 - 255*/
	token = strtok(temp, ".");
	count++;
	while(NULL != token)
	{
		/*remove each token of IP address*/
		num = atoi(token);			

		/*check the range*/
		if(num < 0 || num > 254)		
		{
			free(temp);

			/*LOGGING*/
			LOGGER(LOG_CRITICAL, "invalid IP address\nexiting validate_ip function\n");
			return FAILURE;
		}
		
		/*remove another token from IP address*/
		token = strtok(NULL,".");
		count++;
		
	}
	free(temp);
	if(4 > count)
	{
		/*LOGGING*/
		LOGGER(LOG_CRITICAL, "invalid IP address\nexiting validate_ip function\n");
		return FAILURE;
	}
	/*FILE LOGGING*/
	LOGGER(LOG_CRITICAL, "End of function: validate_ip\n");

	return SUCCESS;
}							/*end of function*/

/**********************************************************************************************************************************************
	FUNCTION DESCRIPTION
	NAME:		Login_server
	WORK:		to check the user authorization for DNS update.
	ARGUMENTS:	sd, nameserver
	RETURN VALUE:	SUCCESS or FAILURE
***********************************************************************************************************************************************/

int Login_server(
			int sd,					/*socket descriptor*/
			struct sockaddr_in *nameserver		/*adderss information of the server*/
		)
{
	/*variable declarations*/
	login info;
	socklen_t len=0;
	int is_send_success=0;
	int is_recv_success=0;

	/*FILE LOGGING*/
	LOGGER(LOG_CRITICAL, "Start of function: Login_server\n");

	printf("enter username (less than 10 characters)\n");
	scanf("%s",info.user);
	printf("enter password (less than 10 characters)\n");
	scanf("%s",info.password);

	/*send username and password to the server for authorization*/
	is_send_success = sendto(sd,"login",6,0,(struct sockaddr *)nameserver,sizeof(*nameserver));

	/*error checking*/
	if(0 > is_send_success){
		ERROR(ERROR_CRITICAL, "Error in sending login information to server...\n");
		LOGGER(LOG_CRITICAL, "returning from Login_server function\n");
		return FAILURE;
	}

	is_send_success = sendto(sd,&info,sizeof(info),0,(struct sockaddr *)nameserver,sizeof(*nameserver));

	/*error checking*/
	if(0 > is_send_success){
		ERROR(ERROR_CRITICAL, "Error in sending username and password  to server...\n");
		LOGGER(LOG_CRITICAL, "returning from Login_server function\n");
		return FAILURE;
	}

	/*receive message from server having authorization response data*/
	recvfrom(sd,&info,sizeof(info),0,(struct sockaddr *)nameserver,&len);

	/*error checking*/
	if(0 > is_recv_success){
		ERROR(ERROR_CRITICAL, "Error in sending username and password  to server...\n");
		LOGGER(LOG_CRITICAL, "returning from Login_server function\n");
		return FAILURE;
	}

	/*FILE LOGGING*/
	LOGGER(LOG_CRITICAL, "End of function: Login_server\n");

	if(1 == info.response)
		return SUCCESS;				/*if person is authorized*/
	else
		return FAILURE;				/*for unauthorization person*/

}							/*end of function*/

/**********************************************************************************************************************************************
	FUNCTION DESCRIPTION
	NAME:		save_and_display
	WORK:		to save the DNS response received from the server and show the output to the user.
	ARGUMENTS:	buffer, length, file
	RETURN VALUE:	void
**********************************************************************************************************************************************/

void save_and_display(
			char *buffer,			/*buffer holding the response from server*/
			int length,				/*length of the message received*/
			char *file				/*local dnsfile at client*/
		     )
{
	/*variable declarations*/
	FILE *fp=NULL;
	struct DNS_HEADER *header=NULL;
	struct RES_RECORD *res_rec=NULL;
	int count = 0;
        int next_record=0;

	/*FILE LOGGING*/
	LOGGER(LOG_CRITICAL, "Start of function: save_and_display\n");

	/*to set header to the message header*/
	header = (struct DNS_HEADER *)buffer;		
	printf("<<--HEADER-->>\n");
	
	/*if server has successfully processed the query*/
	if (header->qr == 1 && header->rcode == 0)
	{

		/*set pointer to the actual resource record contained in the message*/
		res_rec = (struct RES_RECORD *)&buffer[length + (count * sizeof(struct RES_RECORD))];

		/*open file in append mode*/
		fp = fopen(file,"a");			

		/*error checking*/
		if(NULL==fp)
		{
			ERROR(ERROR_CRITICAL, "Error in opening local DNS file at client...\n");
			LOGGER(LOG_CRITICAL, "returning from save_and_display function\n");
			return ;
		}

		printf("Received Data from Server\n");
		switch(header->opcode)			
		{

			/*if query was for IP address*/
			case 0:							
			
				/*print the various fields of message*/
				printf("opcode:QUERY,status:NOERROR,id: %hu\n",header->id);
				printf("flags:qr,rd,ra; QUERY: %hu\n",(unsigned short)htons(header->q_count));
				printf("ANSWER: %hu\n",header->ans_count);
				printf("AUTHORITY: %hu\n",header->auth_count);
				printf("ADDITIONAL:%hu\n",header->add_count);
				printf("\n<<--QUESTION SECTION-->>\n");
				printf("%s\t\tIN\t",res_rec->name);
				printf("A\t");				

				printf("\n<<--ANSWER SECTION-->>\n");
				/*write all resource records in the local file*/
				while(1)
				{
					fprintf(fp,"%s\t%s\t\t",res_rec->rdata,res_rec->name);
					fprintf(fp,"A\t");
					fprintf(fp,"IN\n");
					printf("%s\t\tIN\t",res_rec->name);
					printf("A\t");
					printf("%s\n",res_rec->rdata);
					count = count + 1;
					if(count >= header->ans_count){		

						/*if all records have been traversed*/
						break;				
					}
					next_record = count * sizeof(struct RES_RECORD);
					res_rec = (struct RES_RECORD *)&buffer[length + next_record];
				}							
				/*end of while loop*/

				break;

			/*if query was for domain name*/
			case 1: 						

				/*print the various fields of message on screen and file*/
				printf("opcode:IQUERY,status:NOERROR,id: %hu\n",header->id);
				printf("flags:qr,rd,ra; QUERY: %hu\n",(unsigned short)htons(header->q_count));
				printf("ANSWER: %hu\n",header->ans_count);
				printf("AUTHORITY: %hu\n",header->auth_count);
				printf("ADDITIONAL:%hu\n",header->add_count);
				printf("\n<<--QUESTION SECTION-->>\n");
				printf("%s\t\tIN\t",res_rec->rdata);
				printf("PTR\t");
				fprintf(fp,"%s\t%s\t\t",res_rec->rdata,res_rec->name);
				fprintf(fp,"PTR\t");
				fprintf(fp,"IN\n");
				printf("\n<<--ANSWER SECTION-->>\n");
				printf("%s\t\tIN\t",res_rec->name);
				printf("PTR\t");
				printf("%s\n",res_rec->rdata);
				break;
		}

		/*close local file*/
		fclose(fp);
	}	
	
	else
	{
		printf("RECORD NOT FOUND\n");
	}
	/*FILE LOGGING*/
	LOGGER(LOG_CRITICAL, "End of function: save_and_display\n");
}										/*end of function*/
