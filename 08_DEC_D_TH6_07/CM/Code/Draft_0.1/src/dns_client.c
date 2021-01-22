/**********************************************************************************************************************************************
*
* 	File name 	: dnsclient.c
* 
*  	Description	: file contains main function that connect to name server,it also receives requests from users for query,
*               		inverse query,update functionalities
*
***********************************************************************************************************************************************/

/************************************************************************************************************************************************      
				 INCLUDING HEADER FILES 
***********************************************************************************************************************************************/
 
#include	"logger.h"
#include	"header.h"
#include	"message.h"
#include 	"authenticate.h"
#include	"resource.h"

int g_trace_level = DETAILED_TRACE;


/***********************************************************************************************************************************************
*
* 	Function name	: main
*
* 	Description	: connect to name server,it also receives requests from users for query,inverse query,update functionalities
*
* 	Return		: SUCCESS or FAILURE
*
***********************************************************************************************************************************************/


int main(int C, char * V[])
{
	int sd = 0; 						// socket_descriptor
	char *domain_name = NULL;				
	struct sockaddr_in nameserver;
	int is_valid = 0;					
	int status = 0;

	LOGGER(LOG_CRITICAL,"Entering function:main\n");

	/*Error checking for command line arguments*/
	
	if(C < 3)
	{
		Display_Help()					// Help Instructions
		return FAILURE;	
	}


	
	/* SOCKET CREATION */
	
	sd = socket(AF_INET,SOCK_DGRAM,0);

	/*Error checking for socket creation*/
	if(0 > sd)
	{
		ERROR(ERROR_MAJOR,"socket creation failed\n");
		LOGGER(LOG_CRITICAL,"Exiting function:main\n");
		return FAILURE;
	}

	

	/* CONFIGURTION OF ADDRESS FIELDS */
	
	nameserver.sin_family = AF_INET;
	nameserver.sin_port = htons(PORT);
	nameserver.sin_addr.s_addr = inet_addr(SERVER);

	TRACE(BRIEF_TRACE,("socket created\n"));

	/*READ OPTIONS FROM COMMAND LINE */

	if(0 == strcmp(V[1],"-DI"))
	{
		if(isdigit(V[2][0]))
		{
			printf("Invalid domain name\n");
		}
		else
		{
			domain_name = remove_prefix(V[2]);
			TRACE(DETAILED_TRACE, ("prefix removed %s\n",domain_name));
			Dns_Query(sd,&nameserver,domain_name,IP);
			free(domain_name);
		}
	}
	
	else if(0 == strcmp(V[1], "-ID"))
	{
		is_valid = validate_ip(V[2]);					/*calling function to validate input IP*/
		if(SUCCESS == is_valid)
		{
			Dns_Query(sd,&nameserver,V[2],DOMAIN);
		}
		else
		{
			printf("invalid ip address entered\n");
			ERROR(ERROR_MINOR,"invalid ip\n");
			LOGGER(LOG_CRITICAL,"Exiting function:main\n");
			return FAILURE;
		}
	}
	else if(0 == strcmp(V[1], "-U"))
	{
		if(NULL == V[3]) 
		{
			Display_Help()						/*Help Instructions*/
			LOGGER(LOG_CRITICAL,"Exiting function:main\n");
			return FAILURE;
		}/*end if*/

		status = Login_server(sd,&nameserver);
		if(status == SUCCESS)
		{
			
			is_valid = validate_ip(V[3]);				/*calling function to validate input IP*/
			if(SUCCESS == is_valid)
			{
				Update(sd,&nameserver,ADD,V[2],V[3]);
			}
			else
			{
				ERROR(ERROR_MINOR,"invalid ip\n");
				return FAILURE;
			}
		}
		else
		{
			printf("login unsuccessfull\n");
			LOGGER(LOG_CRITICAL,"Exiting function:main\n");
			return FAILURE;
		}/*end if*/
	}
	else if(0 == strcmp(V[1],"-D"))
	{	
		if(NULL == V[3])
		{
			Display_Help()/*calling macro to display help*/
			LOGGER(LOG_CRITICAL,"Exiting function:main\n");
			return FAILURE;
		}
		status = Login_server(sd,&nameserver);
		if(status == SUCCESS)
		{
			
			is_valid = validate_ip(V[3]);/*calling function to validate input IP*/
			if(SUCCESS == is_valid)
			{
				Update(sd,&nameserver,DELETE,V[2],V[3]);
			}
			else/*if IP is Invalid*/
			{
				ERROR(ERROR_MINOR,"invalid IP\n");
				return FAILURE;
			}
		}
		else/*on unsuccessful login*/
		{
			printf("login unsuccessfull\n");
			LOGGER(LOG_CRITICAL,"Exiting function:main\n");
			return FAILURE;
		}
	}
	else
	{
		Display_Help()							/*Help Instructions*/
	}
	LOGGER(LOG_CRITICAL,"Exiting function:main\n");
	return SUCCESS;
}

